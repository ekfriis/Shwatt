// Shared state information
#include "ShwattGlobals.h"

volatile uint8_t KalmanState;

//bit set to determine the calibration state of the shwatt
uint8_t CalibrationState;

// The global dynamics state of the Shwatt
state_type state[NumStateVars];              
stateError_type stateErrors[NumStateVars][NumStateVars] = {{0}};

volatile measure_type measures[NumMeasures];   // from DAQ and messages from other foot
volatile measure_type measureNoise[NumMeasures];        // R vector, determined from calibration
measure_type measureBias[NumMeasures];         // this is actually part of the state vector
measure_type measureBiasError[NumMeasures];    // actually diagonal elements of state covariance

//volatile _Fract timeSinceLastMeasure;    // time elapsed since last measurement - as recorded in DAQ
time_type timeSinceLastMeasure;          // time elapsed since last measurement - as recorded in DAQ
                                                // necessary to integrate yRate, etc.
// Shmitt Trigger state
uint8_t TriggerState;

_Accum crankLength;                      // a state 'constant'
stateError_type crankError;                       // diagonal elemenent of covariance for crank
stateError_type crankDynamicsErr[NumStateVars];   // off-diagonal covariance between crank error and state dynamics

measure_type gravity;                          // determined from calibration
measure_type gSquared;
_Accum gyroGain;

// cached quantities
_Fract phiDot; 
_Fract phiDotSquared;
_Fract crankRadiusPhiDotSquared;

_Fract CosTheta_minus_Phi;
_Fract SinTheta_minus_Phi;
_Fract gSinTheta;
_Fract gCosTheta;

