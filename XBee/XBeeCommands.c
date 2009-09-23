#include "XBee/XBee.h"
#include "Core/ShwattGlobals.h"
#include <stdfix.h>
#include <inttypes.h>

#define BroadcastPreludeLength     14

#define XBeeContains_StateLength                10
#define XBeeContains_StateErrorsLength          14
#define XBeeContains_MeasuresLength             10
#define XBeeContains_MeasureNoiseLength         XBeeContains_MeasuresLength
#define XBeeContains_MeasureBiasesLength        XBeeContains_MeasuresLength
#define XBeeContains_ShwattStatusLength         3
#define XBeeContains_PerformanceLength          10
#define XBeeContains_InterComLength             18

uint8_t BroadcastDataOptions;

void SetBroadcastData(uint8_t options)
{
   BroadcastDataOptions = options;
}

extern _Fract hMatrixDynamics[4];          
extern volatile uint8_t cyclesSinceLastBroadcast;

void WriteBroadcastPrelude(uint8_t* checksum)
{
   XBeeWriteByte(0x10, checksum);   // API command ID, transmit request (byte 1)

   XBeeWriteByte(0x00, checksum);   // Frame ID (0, for no response)    (byte 2)

   XBeeWriteByte(0x00, checksum);   // Broadcast TX 0x00000000FFFF      (byte 3)
   XBeeWriteByte(0x00, checksum);   // 64 bit address
   XBeeWriteByte(0x00, checksum);
   XBeeWriteByte(0x00, checksum);
   XBeeWriteByte(0x00, checksum);
   XBeeWriteByte(0x00, checksum);
   XBeeWriteByte(0x00, checksum);      
   XBeeWriteByte(0x00, checksum);      // Coordinator TX 0x0000 

   XBeeWriteByte(0xFF, checksum);      // Anytime 16-bit address unknown
   XBeeWriteByte(0xFE, checksum);       

   XBeeWriteByte(0x01, checksum); // Broadcast radius
   XBeeWriteByte(0x08, checksum); // Enable multicast 14?
}

void BroadcastData(void)
{
   if (!BroadcastDataOptions)
      return;

   uint8_t  checksum = 0;
   uint16_t length   = BroadcastPreludeLength;

   length += 1; //broadcast data options
   length += ( BroadcastDataOptions & XBeeContains_State )         ? XBeeContains_StateLength         : 0;
   length += ( BroadcastDataOptions & XBeeContains_StateErrors )   ? XBeeContains_StateErrorsLength   : 0;
   length += ( BroadcastDataOptions & XBeeContains_Measures )      ? XBeeContains_MeasuresLength      : 0;
   length += ( BroadcastDataOptions & XBeeContains_MeasureNoise )  ? XBeeContains_MeasureNoiseLength  : 0;
   length += ( BroadcastDataOptions & XBeeContains_MeasureBiases ) ? XBeeContains_MeasureBiasesLength : 0;
   length += ( BroadcastDataOptions & XBeeContains_Performance )   ? XBeeContains_PerformanceLength   : 0;
   length += ( BroadcastDataOptions & XBeeContains_ShwattStatus )  ? XBeeContains_ShwattStatusLength  : 0;
   length += ( BroadcastDataOptions & XBeeContains_InterCom    )   ? XBeeContains_InterComLength      : 0;

   SerialWrite(START_FRAME);                    //start the frame
   XBeeWriteBytes(length, 0);                   //set the length (dont' need checksum yet)
   WriteBroadcastPrelude(&checksum);

   // Write the extra data header
   XBeeWriteByte(BroadcastDataOptions, &checksum);

   if( BroadcastDataOptions & XBeeContains_InterCom ) 
   {
      //intercom flag          
      XBeeWriteByte(0xDA, &checksum);                           //byte 1
      //the intercom has a sub checksum (to ensure another API#10 frame doesnt' collide)
      uint8_t subChecksum = 0;          
      XBeeWriteFract(state[phiIndex],                       &subChecksum); //byte 2,3
      XBeeWriteFract(state[phiDotIndex],                    &subChecksum); //byte 4,5
      XBeeWriteAccum(stateErrors[phiIndex][phiIndex],       &subChecksum); //byte 6789
      XBeeWriteAccum(stateErrors[phiDotIndex][phiDotIndex], &subChecksum); //byte 10,11,12,13
      XBeeWriteFract(measures[xAxisIndex],                  &subChecksum); //byte 14,15
      XBeeWriteFract(measures[zAxisIndex],                  &subChecksum); //byte 16,17
      // when resumed (including checksum), result will be 0xDA
      checksum   += subChecksum;
      subChecksum = 0xDA - subChecksum;         
      XBeeWriteByte(subChecksum, &checksum);                    //byte 18
   }

   if( BroadcastDataOptions & XBeeContains_State )
   {
      XBeeWriteFract(state[thetaIndex], &checksum);
      XBeeWriteFract(state[phiIndex], &checksum);
      XBeeWriteFract(state[phiDotIndex], &checksum);
      XBeeWriteAccum(crankLength, &checksum);
   }

   if( BroadcastDataOptions & XBeeContains_StateErrors )
   {
      XBeeWriteAccum(stateErrors[thetaIndex][thetaIndex], &checksum);
      XBeeWriteAccum(stateErrors[phiIndex][phiIndex], &checksum);
      XBeeWriteAccum(stateErrors[phiDotIndex][phiDotIndex], &checksum);
      XBeeWriteFract(crankError, &checksum);
   }

   if (BroadcastDataOptions & XBeeContains_ShwattStatus )
   {
      XBeeWriteByte(KalmanState, &checksum);
      XBeeWriteByte(TriggerState, &checksum);
      XBeeWriteByte(CalibrationState, &checksum);
   }

   if (BroadcastDataOptions & XBeeContains_Performance )
   {
      XBeeWriteBytes(cyclesSinceLastBroadcast, &checksum);
      XBeeWriteAccum(gyroGain, &checksum);
      //XBeeWriteAccum(timeSinceLastMeasure, &checksum);
      XBeeWriteFract(gravity, &checksum);
      XBeeWriteFract(timeSinceLastMeasure, &checksum);
   }

   if( BroadcastDataOptions & XBeeContains_Measures )
   {
      XBeeWriteFract(measures[xAxisIndex], &checksum);
      XBeeWriteFract(measures[zAxisIndex], &checksum);
      XBeeWriteFract(measures[yRateIndex], &checksum);
      XBeeWriteFract(measures[triggerPhiIndex], &checksum);
      XBeeWriteFract(measures[triggerPhiDotIndex], &checksum);
   }

   if( BroadcastDataOptions & XBeeContains_MeasureNoise)
   {
      XBeeWriteFract(measureNoise[xAxisIndex], &checksum);
      XBeeWriteFract(measureNoise[zAxisIndex], &checksum);
      XBeeWriteFract(measureNoise[yRateIndex], &checksum);
   }

   if( BroadcastDataOptions & XBeeContains_MeasureBiases)
   {
      XBeeWriteFract(measureBias[xAxisIndex], &checksum);
      XBeeWriteFract(measureBias[zAxisIndex], &checksum);
      XBeeWriteFract(measureBias[yRateIndex], &checksum);
   }

   checksum = 0xFF - checksum;
   XBeeWriteByte(checksum, 0);
}

