#include "Calibrate.h"

void DoFakeCalibSequence(_Fract* yRateBias, volatile _Fract* yRateNoise, 
                         _Fract* xAxisBias, volatile _Fract* xAxisNoise, 
                         _Fract* zAxisBias, volatile _Fract* zAxisNoise,
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
   waitUntilFacing(CircuitSideUp, 0, 0, 0);
   _Accum gyroIntegral = 0;
   waitUntilFacing(CircuitSideDown, &upYAverage, &upYNoise, &gyroIntegral);
   // gyro integral should correspond to a 180 degree flip.
   // 180 deg = 1 in binary angle
   *gyroGain = (1.0/gyroIntegral);
#endif
}

