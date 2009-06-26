#include "XBee/XBee.h"
#include "Core/ShwattGlobals.h"
#include <stdfix.h>
#include <inttypes.h>

#define BroadcastPreludeLength     14
#define BroadcastPreludeChecksum 0x14

uint8_t BroadcastDataOptions;

void SetBroadcastData(uint8_t options)
{
   BroadcastDataOptions = options;
}

extern _Fract hMatrixDynamics[4];          
extern volatile uint8_t cyclesSinceLastBroadcast;

void WriteBroadcastPrelude(void)
{
   SerialWrite(0x10);   // API command ID, transmit request (byte 1)
   SerialWrite(0x00);   // Frame ID (0, for no response)    (byte 2)
   SerialWrite(0x00);   // Broadcast TX 0x00000000FFFF      (byte 3)
   SerialWrite(0x00);   // 64 bit address
   SerialWrite(0x00);
   SerialWrite(0x00);
   SerialWrite(0x00);
   SerialWrite(0x00);
   SerialWrite(0xFF);
   SerialWrite(0xFF);
   SerialWrite(0xFF);      // Broadcast TX 0xFFFE
   SerialWrite(0xFE);      // 
   XBeeWriteByte(0x01, 0); // Broadcast radius
   XBeeWriteByte(0x08, 0); // Enable multicast 14?
}

void BroadcastData(void)
{
   if (!BroadcastDataOptions)
      return;
   uint8_t  checkSum = BroadcastPreludeChecksum;
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
   WriteBroadcastPrelude();

   // Write the extra data header
   XBeeWriteByte(BroadcastDataOptions, &checkSum);

   if( BroadcastDataOptions & XBeeContains_InterCom ) 
   {
      XBeeWriteByte(0xDA, &checkSum);   //intercom flag          
      uint8_t subChecksum = 0;          //the intercom has a sub checksum
      XBeeWriteFract(measures[phiIndex],        &subChecksum); 
      XBeeWriteFract(measures[phiDotIndex],     &subChecksum); 
      XBeeWriteFract(measureNoise[phiIndex],    &subChecksum); 
      XBeeWriteFract(measureNoise[phiDotIndex], &subChecksum); 
      XBeeWriteFract(measures[xAxisIndex],      &subChecksum); 
      XBeeWriteFract(measures[zAxisIndex],      &subChecksum); 
      checkSum   += subChecksum;
      subChecksum = 0xDA - subChecksum;         // when resumed (including checksum), result will be 0x69
      XBeeWriteByte(subChecksum, &checkSum);
   }

   if( BroadcastDataOptions & XBeeContains_State )
   {
      XBeeWriteFract(state[thetaIndex], &checkSum);
      XBeeWriteFract(state[phiIndex], &checkSum);
      XBeeWriteFract(state[phiDotIndex], &checkSum);
      XBeeWriteAccum(crankLength, &checkSum);
   }

   if( BroadcastDataOptions & XBeeContains_StateErrors )
   {
      XBeeWriteAccum(stateErrors[thetaIndex][thetaIndex], &checkSum);
      XBeeWriteAccum(stateErrors[phiIndex][phiIndex], &checkSum);
      XBeeWriteAccum(stateErrors[phiDotIndex][phiDotIndex], &checkSum);
      XBeeWriteFract(crankError, &checkSum);
   }

   if (BroadcastDataOptions & XBeeContains_ShwattStatus )
   {
      XBeeWriteByte(KalmanState, &checkSum);
      XBeeWriteByte(TriggerState, &checkSum);
      XBeeWriteByte(CalibrationState, &checkSum);
   }

   if (BroadcastDataOptions & XBeeContains_Performance )
   {
      XBeeWriteBytes(cyclesSinceLastBroadcast, &checkSum);
      XBeeWriteAccum(gyroGain, &checkSum);
      //XBeeWriteAccum(timeSinceLastMeasure, &checkSum);
      XBeeWriteFract(gravity, &checkSum);
      XBeeWriteFract(timeSinceLastMeasure, &checkSum);
   }

   if( BroadcastDataOptions & XBeeContains_Measures )
   {
      XBeeWriteFract(measures[xAxisIndex], &checkSum);
      XBeeWriteFract(measures[zAxisIndex], &checkSum);
      XBeeWriteFract(measures[yRateIndex], &checkSum);
      XBeeWriteFract(measures[triggerPhiIndex], &checkSum);
      XBeeWriteFract(measures[triggerPhiDotIndex], &checkSum);
   }

   if( BroadcastDataOptions & XBeeContains_MeasureNoise)
   {
      XBeeWriteFract(measureNoise[xAxisIndex], &checkSum);
      XBeeWriteFract(measureNoise[zAxisIndex], &checkSum);
      XBeeWriteFract(measureNoise[yRateIndex], &checkSum);
   }

   if( BroadcastDataOptions & XBeeContains_MeasureBiases)
   {
      XBeeWriteFract(measureBias[xAxisIndex], &checkSum);
      XBeeWriteFract(measureBias[zAxisIndex], &checkSum);
      XBeeWriteFract(measureBias[yRateIndex], &checkSum);
   }

   checkSum = 0xFF - checkSum;
   XBeeWriteByte(checkSum, 0);
}

