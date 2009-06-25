#ifndef ShwattClock
#define ShwattClock

/*
 * Central definition of time keeping
 *
 */

#include "Core/ShwattGlobals.h"
#include "Core/HardwareData.h"
#include <inttypes.h>
#include <avr/interrupt.h>
#include "Math/FractSupport.h"

extern void clockOverflowHandler(void);

void     setupClock(void);

uint16_t rawTime(void);
uint8_t  rawTimeMSB(void);
uint8_t  rawTimeLSB(void);
time_type   time(uint16_t rawInterval);

#endif
