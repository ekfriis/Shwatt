#ifndef Shwatt_CalibrationRoutines_h
#define Shwatt_CalibrationRoutines_h

#include "Core/ShwattGlobals.h"

#include "DAQ/ShwattDAQ.h"
#include "Math/FractSupport.h"
#include "Algorithms/Kalman.h"
#include <util/delay.h>
#include <stdlib.h>
#include "DAQ/Clock.h"

void DoCalibSequence(_Fract* yRateBias, _Fract* yRateNoise, 
                     _Fract* xAxisBias, _Fract* xAxisNoise, 
                     _Fract* zAxisBias, _Fract* zAxisNoise,
                     _Fract* gravity,   _Accum* gyroGain);

void DoFakeCalibSequence(_Fract* yRateBias, _Fract* yRateNoise, 
                         _Fract* xAxisBias, _Fract* xAxisNoise, 
                         _Fract* zAxisBias, _Fract* zAxisNoise,
                         _Fract* gravity,   _Accum* gyroGain);

void GetAveragesAndVariances(int16_t* yRateAverage, int16_t* yRateNoise, 
                             int16_t* xAxisAverage, int16_t* xAxisNoise, 
                             int16_t* zAxisAverage, int16_t* zAxisNoise);

uint16_t varianceSquared(uint32_t* SumOfSquares, uint32_t* Sum, uint8_t nSamplesByTwo, uint8_t multiplier);

uint8_t checkSideUp(int16_t* yBias, int16_t* yNoise, _Accum* yIntegral);
void waitUntilFacing(uint8_t sideFlag, int16_t* yBias, int16_t* yNoise, _Accum* yIntegral);

#endif
