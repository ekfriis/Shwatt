#ifndef ShwattParameters
#define ShwattParameters

/*
 * Global parameters of the algorithm
 *
 */

#define FakeConditions
//#define ForceGyroCalib

// ==================================
// * Startup parameters
// ==================================
#define InitialTheta         0.01
#define InitialPhi          -0.1
#define InitialPhiDot        0.0
// #define InitialCrank       80 // comes from calc
#define InitialCrank         1   // from manual test (??)

#define InitialThetaError    0.1
#define InitialPhiError      0.5
#define InitialPhiDotError   0.01
#define InitialCrankError    5
#define InitialMeasureBias   0.0

#define TrigPhiNoise         0.00
#define TrigPhiDotNoise      0.05


// ==================================
// * Unit scaling
// ==================================
#define ScaleMeasures           4
//#define ScaleMeasures           2
//#define ScaleTime               1 
#define ScaleTime               5
//#define ScaleTime               2 
//#define ScaleTime               8 
//#define ScaleTime               1 // sort of werks
//#define ScaleTime               2 
//#define ScaleTime               3 // overflow in pi/4 
//#define ScaleTime               4

// ==================================
// * Fake conditions
// ==================================
#define FakeYrateBias           0x1A70
#define FakeYrateNoise          0x0097
#define FakeXaxisBias           0x1FBB
#define FakeZaxisBias           0x1FB0
#define FakeGravity             0x0661
#define FakeGSquared            ((FakeGravity)*(FakeGravity))
#define FakeGyroGain            0x00015B5E
#define FakeXaxisNoise          0x00A8
#define FakeZaxisNoise          0x0096

// ==================================
// * Shmitt Trigger
// ==================================

#define TriggerNoise            ( ( (FakeXaxisNoise) + (FakeZaxisNoise) ) >> 1 )
#define TriggerScaleNoise 2                             //scale known measure noise for trigger
#define IsMovingThreshold 0.2                           //percentage (of g) that must be satisfied of isMoving to return true
#define TimesMovingMax 60                               //number of times above requirement has to fail to be marked is/isnot moving
#define TimesMovingMin ( (-1) * TimesMovingMax )

// ==================================
// * Kalman filter parameters
// ==================================

#define Q_theta                 0.05
#define Q_phi                   0.05
#define Q_phiDot                0.05
#define Q_crank                 0.2

#define Q_AccelBias             ADXL330_BIAS_NOISE
#define Q_GyroBias              2
#define Q_ExtPhiBias            0
#define Q_ExtPhiDotBias         0

#define DEBUG_DISABLE_BIASUPDATE  0x0                    //if Zero disable bias updating
#define DEBUG_DISABLE_CRANKUPDATE 0x0                    //if Zero disable bias updating

// ==================================
// * Calibration parameters
// ==================================

#define NumSamplesByTwo 7                               // number of samples to take in each calib run
#define NumSamples (1 << NumSamplesByTwo)
#define DelayBetweenSamples 30                          // delay X ms between samples

// ==================================
// * Communication parameters
// ==================================
//
//#define BroadcastEveryNthDaqByTwo 2
//#define BaudRate                 19200
#define BaudRate                  57600
#define BroadcastEveryNthDaqByTwo 1
#define BroadcastEveryNthDaq      (1 << BroadcastEveryNthDaqByTwo)

#define RX_BUFFER_SIZE          32
#define TX_BUFFER_SIZE          128 
#define FRAME_BUFFER_SIZE       2

#endif
