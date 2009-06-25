#include "Calibrate.h"

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

   waitUntilFacing(CircuitSideUp, 0, 0, 0);

   GetAveragesAndVariances(&upYAverage, &upYNoise,
                           &upXAverage, &upXNoise,
                           &upZAverage, &upZNoise);

   // Establish the gain needed to calibrate the gyro.
   // Since the user is flipping it 180 degrees,
   // this integral should (once scaled) be 1. (in binary angle)
   _Accum gyroIntegral = 0;
   waitUntilFacing(CircuitSideDown, &upYAverage, &upYNoise, &gyroIntegral);
   // gyro integral should correspond to a 180 degree flip.
   // 180 deg = 1 in binary angle
   *gyroGain = (1.0/gyroIntegral);

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
}
