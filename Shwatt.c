#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "Calibration/Calibrate.h"
#include "DAQ/ShmittTrigger.h"
#include "Algorithms/Kalman.h"
#include "XBee/XBee.h"
#include "Math/FractSupport.h"
#include "DAQ/Clock.h"
#include "Core/ShwattGlobals.h"               // all shared state information

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
   EnableRemoteShwattInput();

   timeSinceLastMeasure = 0;
   KalmanState      = 0;
   TriggerState     = 0;
   CalibrationState = 0;

   state[thetaIndex]  = InitialTheta;
   state[phiIndex]    = InitialPhi;
   state[phiDotIndex] = InitialPhiDot;
   crankLength        = InitialCrank;

   initializeStateErrors();

   /*
   SetBroadcastData(    XBeeContains_ShwattStatus 
                      | XBeeContains_Measures 
                      | XBeeContains_MeasureNoise
                      | XBeeContains_MeasureBiases );
                      */

#ifdef FakeConditions
   DoFakeCalibSequence(&measureBias[yRateIndex], &measureNoise[yRateIndex],
                       &measureBias[xAxisIndex], &measureNoise[xAxisIndex], 
                       &measureBias[zAxisIndex], &measureNoise[zAxisIndex], 
                       &gravity, &gyroGain);
#else
   DoCalibSequence(&measureBias[yRateIndex], &measureNoise[yRateIndex],
                   &measureBias[xAxisIndex], &measureNoise[xAxisIndex], 
                   &measureBias[zAxisIndex], &measureNoise[zAxisIndex], 
                   &gravity, &gyroGain);
#endif

   measureNoise[triggerPhiIndex]    = TrigPhiNoise;
   measureNoise[triggerPhiDotIndex] = TrigPhiDotNoise;

   BroadcastData();
   
   /*
   SetBroadcastData(  XBeeContains_InterCom //interfoot communication
                    | XBeeContains_State 
                    | XBeeContains_Performance 
                    | XBeeContains_ShwattStatus
                    | XBeeContains_Measures 
                    | XBeeContains_StateErrors 
                   );
    */
   SetBroadcastData(  XBeeContains_InterCom  );//interfoot communication


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

