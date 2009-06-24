#include "Calibrate.h"

#ifdef FakeConditions
void DoCalibSequence(_Fract* yRateBias, _Fract* yRateNoise, 
                     _Fract* xAxisBias, _Fract* xAxisNoise, 
                     _Fract* zAxisBias, _Fract* zAxisNoise,
                     _Fract* gravity,   _Accum* gyroGain)
{
   int16_t yRateBiasInt  = FakeYrateBias;
   int16_t yRateNoiseInt = FakeYrateNoise;
   // Fake condtions
   *yRateBias  = rbits(yRateBiasInt);
   *xAxisBias  = rbits(FakeXaxisBias);
   *zAxisBias  = rbits(FakeZaxisBias);
   *gravity    = rbits(FakeGravity);
   gSquared    = (*gravity)*(*gravity);

   *gyroGain   = kbits(FakeGyroGain);
   *yRateNoise = rbits(yRateNoiseInt);
   *xAxisNoise = rbits(FakeXaxisNoise);
   *zAxisNoise = rbits(FakeZaxisNoise);

#ifdef ForceGyroCalib

   StatusPins = StatusPin0;
   CalibrationState |= CalibSequence;
   CalibrationState |= CircuitSideUp; //request circuit side up

   while(checkSideUp(0, 0, 0) != CircuitSideUp)
   {
      //block until circuit side up
      // don't do any integrating here, because 
      // we dont' yet know the gyro bias
   }
   //_delay_ms(1500);
   CalibrationState &= ~(CircuitSideUp);
   CalibrationState |= Still;

   StatusPins = StatusPin1;
   CalibrationState &= ~(Still);
   CalibrationState |= CircuitSideDown;

   // Establish the gain needed to calibrate the gyro.
   // Since the user is flipping it 180 degrees,
   // this integral should (once scaled) be 1. (in binary angle)
   _Accum gyroIntegral = 0;
   while(checkSideUp(&yRateBiasInt, &yRateNoiseInt, &gyroIntegral) != 0x01)
   {
      //block until flipped
   }

   // gyro integral should correspond to a 180 degree flip.
   // 180 deg = 1 in binary angle
   *gyroGain = (1.0/gyroIntegral);

   CalibrationState &= ~(CircuitSideDown);
   CalibrationState |= Still;

}

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

#else
}
#endif //ForceGyroCalib

#else

void DoCalibSequence(_Fract* yRateBias, _Fract* yRateNoise, 
                     _Fract* xAxisBias, _Fract* xAxisNoise, 
                     _Fract* zAxisBias, _Fract* zAxisNoise,
                     _Fract* gravity,   _Accum* gyroGain)
{
   // right side up
   int16_t upXAverage = 0;
   int16_t upZAverage = 0;
   int16_t upYAverage = 0;

   int16_t upXNoise = 0;
   int16_t upZNoise = 0;
   int16_t upYNoise = 0;

   int16_t downXAverage = 0;
   int16_t downZAverage = 0;
   int16_t downYAverage = 0;

   int16_t downXNoise = 0;
   int16_t downZNoise = 0;
   int16_t downYNoise = 0;

   StatusPins = StatusPin0;
   CalibrationState |= CalibSequence;
   CalibrationState |= CircuitSideUp; //request circuit side up

   while(checkSideUp(0, 0, 0) != CircuitSideUp)
   {
      //block until circuit side up
      // don't do any integrating here, because 
      // we dont' yet know the gyro bias
   }
   //_delay_ms(1500);
   CalibrationState &= ~(CircuitSideUp);
   CalibrationState |= Still;

   GetAveragesAndVariances(&upYAverage, &upYNoise,
                           &upXAverage, &upXNoise,
                           &upZAverage, &upZNoise);


   StatusPins = StatusPin1;
   CalibrationState &= ~(Still);
   CalibrationState |= CircuitSideDown;

   // Establish the gain needed to calibrate the gyro.
   // Since the user is flipping it 180 degrees,
   // this integral should (once scaled) be 1. (in binary angle)
   _Accum gyroIntegral = 0;
   while(checkSideUp(&upYAverage, &upYNoise, &gyroIntegral) != 0x01)
   {
      //block until flipped
   }

   // gyro integral should correspond to a 180 degree flip.
   // 180 deg = 1 in binary angle
   *gyroGain = (1.0/gyroIntegral);

   CalibrationState &= ~(CircuitSideDown);
   CalibrationState |= Still;

   GetAveragesAndVariances(&downYAverage, &downYNoise,
                           &downXAverage, &downXNoise,
                           &downZAverage, &downZNoise);

   CalibrationState &= ~(Still);
   CalibrationState &= ~(CircuitSideDown);  //OK, done getting biases & g - calculate!
                                            
   // X should be orthogonal to the ground, so this *should* be easy.
   // definitely needs sanity checks here...
   upXAverage += downXAverage;
   upXAverage >>= 1;

   upYAverage += downYAverage;
   upYAverage += 1; //rounding
   upYAverage >>= 1;

   // this is the only one that really needs up/down
   int16_t theZAverage = upZAverage + downZAverage;
   theZAverage += 1; //round
   theZAverage >>= 1;

   int16_t gravityCalib = (abs(upZAverage-downZAverage)+1)>>1;

   // why not average these?
   upXNoise += downXNoise;
   upXNoise >>= 1;

   upZNoise += downZNoise;
   upZNoise >>= 1;

   upYNoise += downYNoise;
   upYNoise >>= 1;

   *yRateBias  = rbits(upYAverage);
   *yRateNoise = rbits(upYNoise);
   *xAxisBias  = rbits(upXAverage);
   *xAxisNoise = rbits(upXNoise);
   *zAxisBias  = rbits(theZAverage);
   *zAxisNoise = rbits(upZNoise);
   *gravity    = rbits(gravityCalib);
   gSquared    = (*gravity)*(*gravity);

   CalibrationState |= (CircuitSideUp | Flip);
   CalibrationState &= ~(CalibSequence);
   StatusPins = 0;
}

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

#endif //end fake conditions
