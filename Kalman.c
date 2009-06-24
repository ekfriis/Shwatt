#include "Kalman.h"

_Fract cachedMeasures[NumMeasures];             // measures are volatile
//_Fract cachedTimeSinceLastMeasure;
time_type cachedTimeSinceLastMeasure;

//_Fract hMatrixDynamics[4];          // one dimensional, as we only consider one measurement at a time 
_Accum hMatrixDynamics[4];          // one dimensional, as we only consider one measurement at a time 
_Accum hMatrixCrank;                // only constant term is crank radius (1x1)
//_Fract hMatrixBiases[4];          // biases are linear in the measurement terms, so this is the unit matrix
//

void initializeStateErrors(void)
{
   stateErrors[thetaIndex][thetaIndex]   = InitialThetaError;
   stateErrors[thetaIndex][phiIndex]     = 0;
   stateErrors[phiIndex][thetaIndex]     = 0;
   stateErrors[thetaIndex][phiDotIndex]  = 0;
   stateErrors[phiDotIndex][thetaIndex]  = 0;

   stateErrors[phiIndex][phiIndex]       = InitialPhiError;
   stateErrors[phiIndex][phiDotIndex]    = 0;
   stateErrors[phiDotIndex][phiIndex]    = 0;

   stateErrors[phiDotIndex][phiDotIndex] = InitialPhiDotError;
   crankError                            = InitialCrankError;
}


void KalmanUpdateDynamics(_Fract residual, const uint8_t measurementIndex)
{
   KalmanState |= Predicting;

   if (StatusPins & StatusPin1)
      StatusPins &= ~(StatusPin1);
   else
      StatusPins |= StatusPin1;

   //_Fract kMatrix[NumStateVars];
   //stateError_type kMatrix[NumStateVars];
   _Accum kMatrix[NumStateVars];
   // compute kalman gain matrix
   // since we are processing only 1 measurement at a time, this is 1 x n
   // matrix, where n is the number of state elements

   // remember that we only store the lower elements of the covariance matrix
   // therefore, the first index must always be <= the second. P is symmetric, so swapping
   // them doesn't matter
   //
   // Note that the hMatrix term always contains the correct row - it is precomputed
   // for each difference measurement
   
   /*
   kMatrix[0]         = stateErrors[0][0]*hMatrixDynamics[0] +
                        stateErrors[0][1]*hMatrixDynamics[1] +
                        stateErrors[0][2]*hMatrixDynamics[2];

   kMatrix[1]         = stateErrors[1][0]*hMatrixDynamics[0] +  
                        stateErrors[1][1]*hMatrixDynamics[1] +
                        stateErrors[1][2]*hMatrixDynamics[2];

   kMatrix[2]         = stateErrors[2][0]*hMatrixDynamics[0] +
                        stateErrors[2][1]*hMatrixDynamics[1] +
                        stateErrors[2][2]*hMatrixDynamics[2];
                        */
   kMatrix[0]  = stateErrors[0][0]*hMatrixDynamics[0];
   kMatrix[0] += stateErrors[0][1]*hMatrixDynamics[1];
   kMatrix[0] += stateErrors[0][2]*hMatrixDynamics[2];

   kMatrix[1]  = stateErrors[1][0]*hMatrixDynamics[0];
   kMatrix[1] += stateErrors[1][1]*hMatrixDynamics[1];
   kMatrix[1] += stateErrors[1][2]*hMatrixDynamics[2];

   kMatrix[2]  = stateErrors[2][0]*hMatrixDynamics[0];
   kMatrix[2] += stateErrors[2][1]*hMatrixDynamics[1];
   kMatrix[2] += stateErrors[2][2]*hMatrixDynamics[2];


   // compute denominator terms
   // H1 P11 H1t
   //_Fract dynamicsDenominator =       hMatrixDynamics[0]*kMatrix[0] +
   /*
   _Accum dynamicsDenominator =       hMatrixDynamics[0]*kMatrix[0] +
                                      hMatrixDynamics[1]*kMatrix[1] +
                                      hMatrixDynamics[2]*kMatrix[2];
                                      */
   _Accum dynamicsDenominator  = hMatrixDynamics[0]*kMatrix[0];
   dynamicsDenominator        += hMatrixDynamics[1]*kMatrix[1];
   dynamicsDenominator        += hMatrixDynamics[2]*kMatrix[2];

   dynamicsDenominator += measureBiasError[measurementIndex];     //this term is sandwiched by the identity H matrixes
   dynamicsDenominator += measureNoise[measurementIndex];         //i.e. the gaussian noise on the accelerometer for this measurement

   if( KalmanState & CrankUpdateBit && DEBUG_DISABLE_CRANKUPDATE)
   {
      //_Fract kMatrixCrank;
      _Accum kMatrixCrank;
      dynamicsDenominator += crankError*hMatrixCrank*hMatrixCrank;

      kMatrixCrank            = hMatrixCrank*crankError
                              + crankDynamicsErr[0]*hMatrixDynamics[0] //cross term in covariance
                              + crankDynamicsErr[1]*hMatrixDynamics[1]
                              + crankDynamicsErr[2]*hMatrixDynamics[2];

      // H2 P21 H1t
      dynamicsDenominator += hMatrixCrank*kMatrixCrank;

      //_Fract h2_P22_plus_h1_P12_temp = kMatrixCrank;
      _Accum h2_P22_plus_h1_P12_temp = kMatrixCrank;

      kMatrixCrank = kMatrixCrank / dynamicsDenominator;

      //update constants term and covariance
      crankLength += kMatrixCrank*residual;

      //P22 = P22 - K2 H1 P21 - K2 H2 P22 
      crankError = crankError - kMatrixCrank*h2_P22_plus_h1_P12_temp;

      // P12 = P12 - K2 H1 P11 - K2 H2 12  (TODO: add K2 H1 P11)
      //_Fract tempProduct = kMatrixCrank*hMatrixCrank;
      _Accum tempProduct = kMatrixCrank*hMatrixCrank;
      crankDynamicsErr[0] = crankDynamicsErr[0] - tempProduct*crankDynamicsErr[0];
      crankDynamicsErr[1] = crankDynamicsErr[1] - tempProduct*crankDynamicsErr[1];
      crankDynamicsErr[2] = crankDynamicsErr[2] - tempProduct*crankDynamicsErr[2];
   }

   if( KalmanState & BiasesUpdateBit && DEBUG_DISABLE_BIASUPDATE)
   {
      _Fract kMatrixBias;  //we only consider the diagonal terms, and only one at a time;
                           //the biases corresponding to the current measure
                                
      kMatrixBias = measureBiasError[measurementIndex] / dynamicsDenominator;

      //update state
      measureBias[measurementIndex] += kMatrixBias*residual;

      //update covariance
      measureBiasError[measurementIndex] = measureBiasError[measurementIndex] - kMatrixBias*measureBiasError[measurementIndex];
   }

   kMatrix[0]  = kMatrix[0]  / dynamicsDenominator;
   kMatrix[1]  = kMatrix[1]  / dynamicsDenominator;
   kMatrix[2]  = kMatrix[2]  / dynamicsDenominator;

   //update state
   //TEST
   state[0] += (state_type)(kMatrix[0]*residual);
   state[1] += (state_type)(kMatrix[1]*residual);
   state[2] += (state_type)(kMatrix[2]*residual);

   //DEBUG
   /*
   if( state[thetaIndex] > 0.15 )
      state[thetaIndex] = 0.15;
   else if( state[thetaIndex] < -0.15 )
      state[thetaIndex] = -0.15;
      */


   //update covariance
   //P = (I - KH)P
   //P11 = P11 - H1 K1 P11 - K1 H2 P12 (TODO: Add K1 H2 P12, maybe not as it shoudl be small)
   //_Fract tempSum = hMatrixDynamics[0]*stateErrors[0][0]
   /*
   _Accum tempSum = hMatrixDynamics[0]*stateErrors[0][0]
                 + hMatrixDynamics[1]*stateErrors[0][1]
                 + hMatrixDynamics[2]*stateErrors[0][2];
                 */
   _Accum tempSum;
   tempSum  = hMatrixDynamics[0]*stateErrors[0][0];
   tempSum += hMatrixDynamics[1]*stateErrors[0][1];
   tempSum += hMatrixDynamics[2]*stateErrors[0][2];

             // r  c                            
   stateErrors[0][0]  =  stateErrors[0][0] - kMatrix[0]*tempSum;
   stateErrors[0][1]  =  stateErrors[0][1] - kMatrix[1]*tempSum;
   stateErrors[0][2]  =  stateErrors[0][2] - kMatrix[2]*tempSum;
    
   /*
   tempSum       = hMatrixDynamics[0]*stateErrors[1][0] 
                 + hMatrixDynamics[1]*stateErrors[1][1]
                 + hMatrixDynamics[2]*stateErrors[1][2];
                 */
   tempSum  = hMatrixDynamics[0]*stateErrors[1][0];
   tempSum += hMatrixDynamics[1]*stateErrors[1][1];
   tempSum += hMatrixDynamics[2]*stateErrors[1][2];

   stateErrors[1][0]  =  stateErrors[1][0] - kMatrix[0]*tempSum;
   stateErrors[1][1]  =  stateErrors[1][1] - kMatrix[1]*tempSum;
   stateErrors[1][2]  =  stateErrors[1][2] - kMatrix[2]*tempSum;

   /*
   tempSum       = hMatrixDynamics[0]*stateErrors[2][0] 
                 + hMatrixDynamics[1]*stateErrors[2][1]
                 + hMatrixDynamics[2]*stateErrors[2][2];
                 */
   tempSum  = hMatrixDynamics[0]*stateErrors[2][0];
   tempSum += hMatrixDynamics[1]*stateErrors[2][1];
   tempSum += hMatrixDynamics[2]*stateErrors[2][2];

   stateErrors[2][0]  =  stateErrors[2][0] - kMatrix[0]*tempSum;
   stateErrors[2][1]  =  stateErrors[2][1] - kMatrix[1]*tempSum;
   stateErrors[2][2]  =  stateErrors[2][2] - kMatrix[2]*tempSum;

   KalmanState &= (uint8_t)(~Predicting); //clear predicting bit
}
   
void KalmanCheckForNewData()
{
   if ( KalmanState & (ExternalDataBit | ShmittTriggered | NewDataBit) )
   {
      KalmanCacheMeasures();
      // always do this promptly, as this data goes bad!
      if ( (KalmanState & ExternalDataBit) )
      {
         KalmanState &= ~(ExternalDataBit); //only use the external data once
         KalmanUpdateExternal();
      }

      if ( (KalmanState & ShmittTriggered) )
      {
         KalmanState &= ~(ShmittTriggered);  //only use this data once
         KalmanUpdateTriggered();
         //DEBUG
         /*
         state[thetaIndex] = 0;
         KalmanState |= CrankUpdateBit;
         KalmanUpdateXaxis();
         KalmanUpdateZaxis();
         KalmanState &= ~CrankUpdateBit;
         */
      }

      // if we have new data that means a new time step.  
      //  Propagate the state and covariance forward in time.
      //  The measurement updates will be called continuously, outside the loop
      if ( KalmanState & NewDataBit )
      {
         cachedTimeSinceLastMeasure = timeSinceLastMeasure;
         KalmanState &= ~(NewDataBit);
         // DEBUG
         KalmanPredictStateAndCovariance();
      }
   }
}

void KalmanCacheMeasures()
{
   // Cache the measurement (to prevent collisions during the update stage)
   // turn off the ready for data and new data bit
   KalmanState &= (unsigned char)( ~(ReadyForData) );
   // WATCHOUT FOR COLISIONS HERE
   cachedMeasures[0]          = measures[0];
   cachedMeasures[1]          = measures[1];
   cachedMeasures[2]          = measures[2];
   cachedMeasures[3]          = measures[3];
   cachedMeasures[4]          = measures[4];
   cachedMeasures[5]          = measures[5];
   cachedMeasures[6]          = measures[6];

   // reset the ready for data bit
   KalmanState |= (unsigned char)(ReadyForData);
}

void KalmanPredictStateAndCovariance() 
{

   // Update system dynamics
   //control objects (from gyros)
   
   // only integrate stuff if we are outside the margin of error
   _Fract myShortTime = (_Fract)cachedTimeSinceLastMeasure;

   state[phiIndex]   += state[phiDotIndex]*((_Fract)myShortTime);

   _Fract yRate = (cachedMeasures[yRateIndex] - measureBias[yRateIndex]);
   state[thetaIndex] += (yRate)*gyroGain*myShortTime*(-1);

   // Update system dynamics errors
   // P(k|k-1) = F P Ft + Q
   //
   stateErrors[thetaIndex][phiIndex]  +=  stateErrors[thetaIndex][phiDotIndex]*myShortTime;
   stateErrors[phiIndex][thetaIndex]   =  stateErrors[thetaIndex][phiIndex]; //TODO: check mirroring for stability

   stateErrors[phiIndex][phiIndex]    += ( (stateErrors[phiIndex][phiDotIndex]*myShortTime) << 1) + 
                                         ( (stateErrors[phiDotIndex][phiDotIndex]*myShortTime)*myShortTime );

   stateErrors[phiIndex][phiDotIndex] +=  stateErrors[phiDotIndex][phiDotIndex]*myShortTime;
   stateErrors[phiDotIndex][phiIndex]  =  stateErrors[phiIndex][phiDotIndex];

   //Add process error
   stateErrors[thetaIndex][thetaIndex]   += Q_theta;
   stateErrors[phiIndex][phiIndex]       += Q_phi;
   stateErrors[phiDotIndex][phiDotIndex] += Q_phiDot;

   // only consider the crank length error if we are currently estimating it.
   if(KalmanState & CrankUpdateBit)
   {
      // errPhi.errCrank is increased by errPhiDot.errCrank*dt
      crankDynamicsErr[phiIndex] += crankDynamicsErr[phiDotIndex]*myShortTime;
      crankError += Q_crank;
   }

   if(KalmanState & BiasesUpdateBit && DEBUG_DISABLE_BIASUPDATE)
   {
      measureBiasError[xAxisIndex]     += Q_AccelBias;
      measureBiasError[zAxisIndex]     += Q_AccelBias;
      measureBiasError[yRateIndex]     += Q_GyroBias;           //this value doesn't really matter, its never used
      measureBiasError[extPhiIndex]    += Q_ExtPhiBias;
      measureBiasError[extPhiDotIndex] += Q_ExtPhiDotBias;
   }

   /*
   if (stateErrors[thetaIndex][thetaIndex] < 0)
      stateErrors[thetaIndex][thetaIndex] = 0.999;
   if (stateErrors[phiIndex][phiIndex] < 0)
      stateErrors[phiIndex][phiIndex] = 0.999;
   if (stateErrors[phiDotIndex][phiDotIndex] < 0)
      stateErrors[phiDotIndex][phiDotIndex] = 0.999;
      */

}

void KalmanCache(void)
{
   phiDot            = state[phiDotIndex]; //convert to radians
   phiDotSquared     = phiDot*phiDot;
   crankRadiusPhiDotSquared = crankLength*phiDotSquared;

   CosTheta_minus_Phi = LookupCosine( (state[thetaIndex] -  state[phiIndex]) );
   SinTheta_minus_Phi = LookupSine(   (state[thetaIndex] -  state[phiIndex]) );
   gSinTheta           = gravity*LookupSine(   state[thetaIndex]  );
   gCosTheta           = gravity*LookupCosine( state[thetaIndex]  );
}

void KalmanUpdateXaxis(void)
{
   KalmanCache();
   // *** Compute terms for left, forward facing accelerometer ***
   //_Fract estimateMeasureFromState       = measureBias[xAxisIndex] + crankRadiusPhiDotSquared * CosTheta_minus_Phi - gSinTheta;
   //_Fract estimateMeasureFromState       = measureBias[xAxisIndex] - crankRadiusPhiDotSquared * CosTheta_minus_Phi - gSinTheta;
   
   // version from test
   _Fract estimateMeasureFromState       = measureBias[xAxisIndex] - crankRadiusPhiDotSquared * SinTheta_minus_Phi - gSinTheta;

   _Fract residual                       = cachedMeasures[xAxisIndex] - estimateMeasureFromState;

   // compute linearized h matrix 
   // note that g is defined as one, by defintion of units

   // theta term 
   // -g Cos(LeftTilt) - crank phiDot^2 Sin(LeftTilt-phi)
   //hMatrixDynamics[thetaIndex] = (gCosTheta + crankRadiusPhiDotSquared*SinTheta_minus_Phi)*(-1);
   //hMatrixDynamics[thetaIndex] = crankRadiusPhiDotSquared*SinTheta_minus_Phi - gCosTheta ;
   hMatrixDynamics[thetaIndex] = -crankRadiusPhiDotSquared*CosTheta_minus_Phi - gCosTheta ;


   // phi term
   // crank phiDot^2 Sin(LeftTilt-phi)
   //hMatrixDynamics[phiIndex] = (crankRadiusPhiDotSquared*SinTheta_minus_Phi);
   //hMatrixDynamics[phiIndex] = (crankRadiusPhiDotSquared*SinTheta_minus_Phi)*(-1);
   hMatrixDynamics[phiIndex] = crankRadiusPhiDotSquared*CosTheta_minus_Phi;

   // phi dot term
   // 2 crank phiDot Cos(LeftTilt - phi)
   //hMatrixDynamics[phiDotIndex] = 2*(crankLength*phiDot*CosTheta_minus_Phi);
   //hMatrixDynamics[phiDotIndex] = (-2)*(crankLength*phiDot*CosTheta_minus_Phi);
   hMatrixDynamics[phiDotIndex] = (-2)*(crankLength*phiDot*SinTheta_minus_Phi);

   // crank radius term
   if (KalmanState & CrankUpdateBit)
   {
      //hMatrixCrank = (-1)*phiDotSquared*CosTheta_minus_Phi;
      //hMatrixCrank = phiDotSquared*CosTheta_minus_Phi;
      hMatrixCrank = -phiDotSquared*SinTheta_minus_Phi;
   }

   KalmanUpdateDynamics(residual, xAxisIndex);
}

void KalmanUpdateZaxis(void)
{
   KalmanCache();
   // *** Compute terms for left, upward facing accelerometer ***
   //_Fract estimateMeasureFromState = measureBias[zAxisIndex] - crankRadiusPhiDotSquared * SinTheta_minus_Phi - gCosTheta;
   //_Fract estimateMeasureFromState = measureBias[zAxisIndex] + crankRadiusPhiDotSquared * SinTheta_minus_Phi - gCosTheta;
     
   // from test
   _Fract estimateMeasureFromState = measureBias[zAxisIndex] - crankRadiusPhiDotSquared * CosTheta_minus_Phi - gCosTheta;
   _Fract residual                 = cachedMeasures[zAxisIndex] - estimateMeasureFromState;

   // theta term 
   // g Sin(LeftTilt) - crank * phiDot^2 * Cos(LT - Phi)
   //hMatrixDynamics[thetaIndex] = gSinTheta - crankRadiusPhiDotSquared * CosTheta_minus_Phi;
   //hMatrixDynamics[thetaIndex] = gSinTheta + crankRadiusPhiDotSquared * CosTheta_minus_Phi;
   hMatrixDynamics[thetaIndex] = gSinTheta + crankRadiusPhiDotSquared*SinTheta_minus_Phi;

   // phi term
   // crank phiDot^2 Sin(Phi - RightTilt)
   //hMatrixDynamics[phiIndex] = (crankRadiusPhiDotSquared*CosTheta_minus_Phi);
   //hMatrixDynamics[phiIndex] = (-crankRadiusPhiDotSquared*CosTheta_minus_Phi);
   hMatrixDynamics[phiIndex] = (-crankRadiusPhiDotSquared*SinTheta_minus_Phi);

   // phi dot term
   // -2 crank phiDot Sin(LeftTilt - phi)
   //hMatrixDynamics[phiDotIndex] = (-2)*(crankLength*phiDot*SinTheta_minus_Phi);
   //hMatrixDynamics[phiDotIndex] = (2)*(crankLength*phiDot*SinTheta_minus_Phi);
   hMatrixDynamics[phiDotIndex] = (-2)*(crankLength*phiDot*CosTheta_minus_Phi);

   // crank radius term
   if (KalmanState & CrankUpdateBit)
   {
      //hMatrixCrank = phiDotSquared*SinTheta_minus_Phi*(-1);
      //hMatrixCrank = phiDotSquared*SinTheta_minus_Phi;
      hMatrixCrank = -phiDotSquared*CosTheta_minus_Phi;
   }

   KalmanUpdateDynamics(residual, zAxisIndex);
}

void KalmanUpdateExternal(void)
{
   // *** Compute terms for left, upward facing accelerometer ***
   // Do external phi measurement
   _Fract estimateMeasureFromState = measureBias[extPhiIndex] + (state[phiIndex] - 1.0); //external phi should be pi radians in the other direction
   _Fract residual                 = cachedMeasures[extPhiIndex] - estimateMeasureFromState;

   hMatrixDynamics[thetaIndex]  = 0;
   hMatrixDynamics[phiIndex]    = 1;
   hMatrixDynamics[phiDotIndex] = 0;
   hMatrixCrank                 = 0;

   KalmanUpdateDynamics(residual, extPhiIndex);

   // Do external phi dot measurement.
   //  we always do these together because the measurement spoils quickly

   estimateMeasureFromState = measureBias[extPhiDotIndex] + state[phiDotIndex];      //should always be the same
   residual                 = cachedMeasures[extPhiDotIndex] - estimateMeasureFromState;

   hMatrixDynamics[phiIndex]    = 0;
   hMatrixDynamics[phiDotIndex] = 1;

   KalmanUpdateDynamics(residual, extPhiDotIndex);
}

void KalmanUpdateTriggered(void)
{
   _Fract estimateMeasureFromState = measureBias[triggerPhiIndex] + state[phiIndex]; //should match exactly state
   _Fract residual                 = cachedMeasures[triggerPhiIndex] - estimateMeasureFromState;

   hMatrixDynamics[thetaIndex]  = 0;
   hMatrixDynamics[phiIndex]    = 1;
   hMatrixDynamics[phiDotIndex] = 0;
   hMatrixCrank                 = 0;

   KalmanUpdateDynamics(residual, triggerPhiIndex);

   // Do external phi dot measurement, if flagged
   if ( TriggerState & TriggeredPhiDot )
   {
      estimateMeasureFromState = measureBias[triggerPhiDotIndex] + state[phiDotIndex];      //should always be the same
      residual                 = cachedMeasures[triggerPhiDotIndex] - estimateMeasureFromState;

      hMatrixDynamics[phiIndex]    = 0;
      hMatrixDynamics[phiDotIndex] = 1;

      KalmanUpdateDynamics(residual, triggerPhiDotIndex);
   }
}





