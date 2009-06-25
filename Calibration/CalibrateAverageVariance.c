#include "Calibrate.h"

#ifndef FakeConditions
uint16_t varianceSquared(uint32_t* SumOfSquares, uint32_t* Sum, uint8_t nSamplesByTwo, uint8_t multiplier)
{
   // s2 = ( n*Sum(y^2) - Sum(y)^2 )/n^2
   uint64_t averageSquared     = (*Sum)*(*Sum);
   uint64_t scaledSumOfSquares = (*SumOfSquares) << nSamplesByTwo;
   uint64_t difference = (uint64_t)(scaledSumOfSquares - averageSquared);
   difference += (1 << (nSamplesByTwo + nSamplesByTwo - 1)); //round correctly
   //difference >>= (nSamplesByTwo + nSamplesByTwo);
   difference >>= (nSamplesByTwo + nSamplesByTwo - multiplier-multiplier);
   return (uint16_t)(difference & 0xFFFF);
}

void GetAveragesAndVariances(int16_t* yRateAverage, int16_t* yRateNoise, 
                             int16_t* xAxisAverage, int16_t* xAxisNoise, 
                             int16_t* zAxisAverage, int16_t* zAxisNoise)
{
   uint32_t YrateSum        = 0;
   uint32_t YrateSumSquared = 0;
   uint32_t ZaxisSum        = 0;
   uint32_t ZaxisSumSquared = 0;
   uint32_t XaxisSum        = 0;
   uint32_t XaxisSumSquared = 0;

   for(uint16_t sample = 0; sample < NumSamples; ++sample)
   {
      AcquireData();
      _delay_ms(DelayBetweenSamples);
      while(!DataReady()) // wait for ADC
         ;
      uint16_t tempYrate = GetYrate();
      uint16_t tempXaxis = GetXaxis();
      uint16_t tempZaxis = GetZaxis();

      YrateSum += tempYrate;
      XaxisSum += tempXaxis;
      ZaxisSum += tempZaxis;

      YrateSumSquared += tempYrate*tempYrate;
      XaxisSumSquared += tempXaxis*tempXaxis;
      ZaxisSumSquared += tempZaxis*tempZaxis;
   }

   uint16_t YrateVarianceSquared = varianceSquared(&YrateSumSquared, &YrateSum, NumSamplesByTwo, ScaleMeasures);
   uint16_t XaxisVarianceSquared = varianceSquared(&XaxisSumSquared, &XaxisSum, NumSamplesByTwo, ScaleMeasures);
   uint16_t ZaxisVarianceSquared = varianceSquared(&ZaxisSumSquared, &ZaxisSum, NumSamplesByTwo, ScaleMeasures);

   YrateSum += (1 << (NumSamplesByTwo-ScaleMeasures-1)); //round correctly 
   XaxisSum += (1 << (NumSamplesByTwo-ScaleMeasures-1)); //round correctly 
   ZaxisSum += (1 << (NumSamplesByTwo-ScaleMeasures-1)); //round correctly 

   // take average of all.
   YrateSum >>= (NumSamplesByTwo - ScaleMeasures);
   XaxisSum >>= (NumSamplesByTwo - ScaleMeasures);
   ZaxisSum >>= (NumSamplesByTwo - ScaleMeasures);

   *yRateAverage        = YrateSum;
   *xAxisAverage        = XaxisSum;
   *zAxisAverage        = ZaxisSum;

   *yRateNoise       = YrateVarianceSquared;
   *xAxisNoise       = XaxisVarianceSquared;
   *zAxisNoise       = ZaxisVarianceSquared;
}
#endif
