#include "ShmittTrigger.h"

uint8_t  lastTriggerState;
int8_t   hasntMovedIn;
_Fract   lastTriggerPosition;
uint16_t lastTriggerTime;
// zx
// 01 is -0.5

_Fract accelSquared(uint8_t index)
{
   // remove bias
   _Fract Acc = measures[index] - measureBias[index];
   // square
   Acc = Acc*Acc;
   return Acc;
}

char isMoving()
{
   _Fract totalAcc = absr(accelSquared(xAxisIndex) + (accelSquared(zAxisIndex) - gSquared));
   //_Fract totalAcc = (accelSquared(xAxisIndex) + (accelSquared(zAxisIndex) - gSquared));
   //totalAcc = totalAcc*totalAcc;
   _Fract threshold = IsMovingThreshold*gSquared;

   if (totalAcc < threshold)
      hasntMovedIn++;
   else
      hasntMovedIn--;

   if (hasntMovedIn > 0)
   {
      if (hasntMovedIn > TimesMovingMax)
         hasntMovedIn = TimesMovingMax;
      TriggerState &= ~(IsMoving);
      return 0x0;
   }
   else //(hasntMovedIn < 0)
   {
      if (hasntMovedIn < TimesMovingMin)
         hasntMovedIn = TimesMovingMin;
      TriggerState |= IsMoving;
      return 0x1;
   }
}


/*
char greaterThanNoise(uint8_t index, _Fract offset)
{
   if ( measures[index] > (measureBias[index] + (measureNoise[index] << TriggerScaleNoise) - offset) )
      return 0x1;
   return 0x0;
}

char lessThanNoise(uint8_t index, _Fract offset)
{
   if ( measures[index] < (measureBias[index] - (measureNoise[index] << TriggerScaleNoise) - offset) )
      return 0x1;
   return 0x0;
}

void checkMeasureFlipped(uint8_t index, _Fract offset)
{
   uint8_t stateBitLow = (1 << index);
   if ( TriggerState & stateBitLow )
   {
      // check if is high enough to flip to the high state
      if (greaterThanNoise(index, offset))
      {
         TriggerState &= ~(stateBitLow);
      }
   }
   else  //we are in the high x state
   {
      // see if we are low enough to flip state
      if (lessThanNoise(index, offset))
      {
         TriggerState |= stateBitLow;
      }
   }
}
*/

#define greaterThanNoiseFlag (1 << 0)
#define lessThanNoiseFlag    (1 << 1)

/*
// trigv1 was working
char aboveOrBelowNoise(uint8_t index, _Fract offset)
{
   _Fract offsetBiasCorrected = measures[index] - measureBias[index] + offset;
   if( absr(offsetBiasCorrected) > (measureNoise[index] << TriggerScaleNoise) )
   {
      if( offsetBiasCorrected > 0 )
      {
         return greaterThanNoiseFlag;
      }
      else
         return lessThanNoiseFlag;
   }
   return 0x0;
}

void checkMeasureFlipped(uint8_t index, _Fract offset)
{
   uint8_t stateBitLow = (1 << index);
   uint8_t aboveOrBelowNoiseFlag = aboveOrBelowNoise(index, offset);
   if ( TriggerState & stateBitLow )
   {
      // check if is high enough to flip to the high state
      if ( aboveOrBelowNoiseFlag & greaterThanNoiseFlag )
      {
         TriggerState &= ~(stateBitLow);
      }
   }
   else  //we are in the high x state
   {
      // see if we are low enough to flip state
      if ( aboveOrBelowNoiseFlag & lessThanNoiseFlag )
      {
         TriggerState |= stateBitLow;
      }
   }
}
*/

uint8_t outsideNoise(_Fract value, _Fract expectedBias)
{
   _Fract offsetBiasCorrected = value - expectedBias;
   //DEBUG
   if( absr(offsetBiasCorrected) > bitsr(TriggerNoise) )
   //if( absr(offsetBiasCorrected) > 0 )
   {
      if( offsetBiasCorrected > 0 )
      {
         return greaterThanNoiseFlag;
      }
      else 
      {
         return lessThanNoiseFlag;
      }
   }
   return 0x0;
}

void checkFlipped(uint8_t index, uint8_t outsideNoiseFlag) 
{
   uint8_t stateBitLow = (1 << index); //get the bit flag for the current axis
   if ( TriggerState & stateBitLow )
   {
      // check if is high enough to flip to the high state
      if ( outsideNoiseFlag & greaterThanNoiseFlag )
      {
         TriggerState &= ~(stateBitLow);
      }
   }
   else  //we are in the high x state
   {
      // see if we are low enough to flip state
      if ( outsideNoiseFlag & lessThanNoiseFlag )
      {
         TriggerState |= stateBitLow;
      }
   }
}

void ShmittTrigger(void)
{
   uint8_t oldState = TriggerState;
   /*
   checkMeasureFlipped(xAxisIndex, 0);
   checkMeasureFlipped(zAxisIndex, gravity);
   */
   // check for zero crossings
   _Fract unbiasedXaxis = measures[xAxisIndex] - measureBias[xAxisIndex];
   _Fract unbiasedZaxis = measures[zAxisIndex] - measureBias[zAxisIndex];
   _Fract labXaxis = gCosTheta*unbiasedXaxis - gSinTheta*unbiasedZaxis; //?
   _Fract labZaxis = gCosTheta*unbiasedZaxis + gSinTheta*unbiasedXaxis;
   //_Fract labXaxis = gravity*unbiasedXaxis; //?
   //_Fract labZaxis = gravity*unbiasedZaxis;
   checkFlipped(xAxisIndex, outsideNoise(labXaxis, 0));
   checkFlipped(zAxisIndex, outsideNoise(labZaxis, -gSquared)); //note gSquared since we use gSinTheta etc for convenience

   // check if nothing changed
   uint8_t changedBits = (oldState ^ TriggerState) & ( xLow | zLow );

   if (changedBits)
   {

       // this sort of works
       /*
      if (changedBits & xLow) // x changed from high to low
      {
         // we are at either the top or bottom of stroke
         if (TriggerState & zLow)
         {
            // top of stroke
            measures[triggerPhiIndex] =  0.5;
         } else
         {
            // bottom of stroke
            measures[triggerPhiIndex] = -0.5;
         }
      } else if (changedBits & zLow)
      {
         // we are at either the front or bake of the stroke
         if (TriggerState & xLow)
         {
            //back
            measures[triggerPhiIndex] = 0.0;
         } else
         {
            measures[triggerPhiIndex] = -1.0;
         }
      }
      */
      if (changedBits & xLow) // x changed from high to low
      {
         // we are at either the top or bottom of stroke
         if (TriggerState & zLow)
         {
            // top of stroke
            measures[triggerPhiIndex] = 0.0;
         } else
         {
            // bottom of stroke
            measures[triggerPhiIndex] =  -1.0;
         }
      } else if (changedBits & zLow)
      {
         // we are at either the front or bake of the stroke
         if (TriggerState & xLow)
         {
            //back
            measures[triggerPhiIndex] =  -0.5; // -0.5 matches screen, neg crank
         } else
         {
            measures[triggerPhiIndex] =  0.5; // 0.5 matches screen,neg crank
         }
      }
      KalmanState  |= ShmittTriggered;
      TriggerState |= TriggeredPhi; //new measure
      TriggerState &= ~(TriggeredPhiDot); //trigger on phiDot ahs yet to be determined

      uint16_t currentTime = rawTime();
      // Now check if we have moved to a difference quadrant
      _Fract changeInPhi = measures[triggerPhiIndex] - lastTriggerPosition;

      if ( changeInPhi != 0 )
      {
         uint16_t timeChange = currentTime - lastTriggerTime;
         measures[triggerPhiDotIndex] = changeInPhi / time(timeChange);
         //measures[triggerPhiDotIndex] =  time(timeChange);
         // DEBUG DISABLE FOR NOW
         TriggerState |= TriggeredPhiDot; //new measure
      }
      lastTriggerPosition = measures[triggerPhiIndex];
      lastTriggerTime     = currentTime;
      lastTriggerState    = TriggerState;
   }

}







   



