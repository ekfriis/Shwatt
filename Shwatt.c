#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "Calibrate.h"
#include "ShmittTrigger.h"
#include "Kalman.h"
#include "XBee/XBee.h"
#include "FractSupport.h"
#include "Clock.h"
#include "ShwattGlobals.h"               // all shared state information

volatile uint16_t rawXaxis;              // store location of interrrupts
volatile uint16_t rawYrate;
volatile uint16_t rawZaxis;

uint16_t rawExtPhi;
uint16_t rawExtPhiDot;
uint16_t rawExtPhiError;
uint16_t rawExtPhiDotError;

volatile uint8_t cyclesSinceLastBroadcast; //count how many iterations we
                                           // perform between measurements

void clockOverflowHandler(void)
{
   if ( KalmanState &= ReadyForData )
      AcquireData();
   if (rawTimeMSB() % BroadcastEveryNthDaq == 0)
   {
      if (StatusPins & StatusPin1)
         StatusPins &= ~(StatusPin1);
      else
         StatusPins |= StatusPin1;
      BroadcastData();
      cyclesSinceLastBroadcast = 0;
   }
}

// called when a DAQ is complete (from ShwattDaq.c)
void ShwattDaqComplete(void)
{
   // prevent collision when accessing data
   //  this shouldn't happen often.  In in case it does,
   //  the DAQ will keep the current value (and time, imortantly)
   //  until the next 
   if (StatusPins & StatusPin0)
      StatusPins &= ~(StatusPin0);
   else
      StatusPins |= StatusPin0;
   if( !(KalmanState & ReadyForData) )
      return;
   // Check if we have new data
   uint16_t timeElapsed = DataReady(); //returns zero if no new data
   if (timeElapsed)
   {
      measures[xAxisIndex] = rbits(rawXaxis << ScaleMeasures);
      measures[yRateIndex] = rbits(rawYrate << ScaleMeasures);
      measures[zAxisIndex] = rbits(rawZaxis << ScaleMeasures);
      // flag that we have new data
      timeSinceLastMeasure = time(timeElapsed);
      KalmanState |= NewDataBit;
      ShmittTrigger();
      // check if not moving.  If so, manually set phiDot to zero
      if(!isMoving())
      {
         // don't use the shmitt trigger if we aren't moving.
         KalmanState &= ~(ShmittTriggered);
         // don't let phi dot go crazy
         state[phiDotIndex] = 0;
         initializeStateErrors();
      }
   }
   // right now, no external data
}

//void XbeeFrameHandler(uint8_t frame_buffer_head)
//{
   //TODO
//}

void setup(void)
{
   DDRB = MyPortBMask;
   StatusPins = StatusPin1;
   SetupADC(&rawXaxis, &rawZaxis, &rawYrate);
   setupClock();
   BeginSerial(BaudRate);

   timeSinceLastMeasure = 0;
   KalmanState      = 0;
   TriggerState     = 0;
   CalibrationState = 0;

   state[thetaIndex]  = InitialTheta;
   state[phiIndex]    = InitialPhi;
   state[phiDotIndex] = InitialPhiDot;
   crankLength        = InitialCrank;

   initializeStateErrors();

   SetBroadcastData(    XBeeContains_ShwattStatus 
                      | XBeeContains_Measures 
                      | XBeeContains_MeasureNoise
                      | XBeeContains_MeasureBiases );

   DoCalibSequence(&measureBias[yRateIndex], &measureNoise[yRateIndex],
                   &measureBias[xAxisIndex], &measureNoise[xAxisIndex], 
                   &measureBias[zAxisIndex], &measureNoise[zAxisIndex], 
                   &gravity, &gyroGain);
   measureNoise[triggerPhiIndex]    = TrigPhiNoise;
   measureNoise[triggerPhiDotIndex] = TrigPhiDotNoise;

   /*
   measureBias[extPhiIndex]        = 0;
   measureBias[extPhiDotIndex]     = 0;
   measureBias[triggerPhiIndex]    = 0;
   measureBias[triggerPhiDotIndex] = 0;
   // measure noise for external will be provided by the external shwatt
   //
   measureBiasError[yRateIndex] = measureNoise[yRateIndex]; //this should depend on nSamples...
   measureBiasError[xAxisIndex] = measureNoise[xAxisIndex]; //this should depend on nSamples...
   measureBiasError[zAxisIndex] = measureNoise[zAxisIndex]; //this should depend on nSamples...
   measureBiasError[extPhiIndex]    = 0.1;
   measureBiasError[extPhiDotIndex] = 0.1;
   */

   BroadcastData();
   
   SetBroadcastData(  XBeeContains_State 
                    | XBeeContains_Performance 
                    | XBeeContains_ShwattStatus
                    | XBeeContains_Measures 
                    | XBeeContains_StateErrors 
                   );

   KalmanState |= ReadyForData;
   //KalmanState |= CrankUpdateBit;
   //KalmanState |= BiasesUpdateBit;
}

void loop(void)
{
   cyclesSinceLastBroadcast++;

   KalmanCheckForNewData();
   KalmanUpdateXaxis();

   KalmanCheckForNewData();
   KalmanUpdateZaxis();
}

int main(void)
{
        sei();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

