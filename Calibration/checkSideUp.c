#include "Calibration/Calibrate.h"

uint8_t checkSideUp(int16_t* yBias, int16_t* yNoise, _Accum* yIntegral)
{
   // Returns 0x1 for circuit side up
   //         0x2 for ground plane up
   //         0x0 for unstable
   //         Integrates over yRate.  Note that yBias is scaled
   //         by ScaleMeasures, so we must do that manually here
   const uint8_t nChecks           = 50;
   const uint8_t delayBetweenCheck = 50;
   const uint8_t safeDistance      = 0x15; //average g is ~0x60

   uint8_t circuitSideUp  = 0x1;
   uint8_t groundSideUp   = 0x1;

   for (uint8_t i = 0; i < nChecks; i++)
   {
      AcquireData();
      _delay_ms(delayBetweenCheck);
      uint16_t timeElapsed = 0;
      while(!timeElapsed)
      {
         // get the TCNT2 counts elapsed since this DAQ was performed
         //  and the last daq
         timeElapsed = DataReady();
      }

      if (yBias) //only do integral if specified.
      {
         int16_t tempYRate = (GetYrate() << ScaleMeasures);
         int16_t yRateUnbiased = (tempYRate - (*yBias));
         if( abs(yRateUnbiased) > (*yNoise)*2 )
            // add to Y integral
            *yIntegral += rbits(yRateUnbiased)*time(timeElapsed);

      }

      uint16_t tempXaxis = GetXaxis();
      uint16_t tempZaxis = GetZaxis();
      if (tempXaxis > (tempZaxis + safeDistance))
      {
         groundSideUp = 0x0;
      }
      if ((tempXaxis + safeDistance) < tempZaxis)
      {
         circuitSideUp = 0x0;
      }
      if (!circuitSideUp && !groundSideUp)
      {
         // if both have failed at some point, the shwatt is not stable
         return 0x0;
      }
      if (circuitSideUp && groundSideUp) 
      {
         // if both are still set at this point, the Zaxis is not perp to gravity
         return 0x0;
      }
   }
   if (circuitSideUp)
      return 0x1;
   if (groundSideUp)
      return 0x2;
   return 0x0;
}

void waitUntilFacing(uint8_t sideFlag, int16_t* yBias, int16_t* yNoise, _Accum* yIntegral)
{
   CalibrationState |= CalibSequence;
   CalibrationState |= sideFlag; //request circuit side up
   while(checkSideUp(0, 0, 0) != sideFlag)
   {
      //block until circuit side up
      // don't do any integrating here, because 
      // we dont' yet know the gyro bias
   }
   //_delay_ms(1500);
   CalibrationState &= ~(sideFlag);
   CalibrationState |= Still;
}

