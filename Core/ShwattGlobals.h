#ifndef ShwattGlobalsDefinitons_H
#define ShwattGlobalsDefinitons_H

#include <inttypes.h>
#include <stdfix.h>
#include "HardwareData.h"
#include "Parameters.h"

// bit set to define Kalman filter state flags
extern volatile uint8_t KalmanState;
#define NewDataBit              ((uint8_t)0x80)
#define ReadyForData            ((uint8_t)0x40)
#define BiasesUpdateBit         ((uint8_t)0x20)
#define CrankUpdateBit          ((uint8_t)0x10)
#define Predicting              ((uint8_t)0x08)
#define ShmittTriggered         ((uint8_t)0x04)
#define ExternalDataBit         ((uint8_t)0x02)
#define Calibrating             ((uint8_t)0x01)

//bit set to determine the calibration state of the shwatt
extern uint8_t CalibrationState;
#define CalibSequence   (1 << 0)
#define CircuitSideUp   (1 << 1)
#define CircuitSideDown (1 << 2)
#define Still           (1 << 3)
#define Flip            (1 << 4)
#define CalibCrank      (1 << 5)

#define NumMeasures             7
#define xAxisIndex              0
#define zAxisIndex              1
#define yRateIndex              2
#define extPhiIndex             3
#define extPhiDotIndex          4
#define triggerPhiIndex         5
#define triggerPhiDotIndex      6

extern uint8_t TriggerState;
#define Triggered               (1 << 7)
#define TriggeredPhi            (1 << 6)
#define TriggeredPhiDot         (1 << 5)
#define IsMoving                (1 << 4)  // 0 if stationary
#define xLow                    (1 << xAxisIndex)
#define zLow                    (1 << zAxisIndex)

typedef _Fract measure_type;
typedef _Fract state_type;
//typedef _Fract stateError_type;
typedef _Accum stateError_type;
typedef _Accum time_type;

// The global dynamics state of the Shwatt
#define NumStateVars            3
#define thetaIndex              0
#define phiIndex                1
#define phiDotIndex             2

extern state_type state[NumStateVars];              
extern stateError_type stateErrors[NumStateVars][NumStateVars];

extern volatile measure_type measures[NumMeasures];   // from DAQ and messages from other foot
extern measure_type measureNoise[NumMeasures];        // R vector, determined from calibration
extern measure_type measureBias[NumMeasures];         // this is actually part of the state vector
extern measure_type measureBiasError[NumMeasures];    // actually diagonal elements of state covariance

//extern volatile _Fract timeSinceLastMeasure;    // time elapsed since last measurement - as recorded in DAQ
extern time_type timeSinceLastMeasure;    // time elapsed since last measurement - as recorded in DAQ
                                                // necessary to integrate yRate, etc.

//extern _Fract crankLength;                      // a state 'constant'
extern _Accum crankLength;                      // a state 'constant'
extern _Accum crankError;                       // diagonal elemenent of covariance for crank
extern _Accum crankDynamicsErr[NumStateVars];   // off-diagonal covariance between crank error and state dynamics

extern measure_type gravity;                          // determined from calibration
extern measure_type gSquared;

extern _Accum gyroGain;

// cached quantities
extern _Fract phiDot; 
extern _Fract phiDotSquared;
extern _Fract crankRadiusPhiDotSquared;

extern _Fract CosTheta_minus_Phi;
extern _Fract SinTheta_minus_Phi;
extern _Fract gSinTheta;
extern _Fract gCosTheta;

#endif
