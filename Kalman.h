#ifndef EK_Kalman_h
#define EK_Kalman_h

#include "ShwattGlobals.h"

#include "HardwareData.h"
#include "TrigLookup.h"
#include <inttypes.h>
#include <stdlib.h>

void KalmanCheckForNewData(void);
void KalmanCacheMeasures(void);
void KalmanPredictStateAndCovariance(void);
void KalmanUpdateXaxis(void);
void KalmanUpdateZaxis(void);
void KalmanUpdateExternal(void);
void KalmanUpdateTriggered(void);
void KalmanUpdateDynamics(const _Fract residual, const uint8_t measurementIndex);

void initializeStateErrors(void);
void KalmanCache(void);

#endif
