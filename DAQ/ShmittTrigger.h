#ifndef ShmittTriggered_h
#define ShmittTriggered_h

#include <stdfix.h>
#include <inttypes.h>
#include <avr/io.h>
#include "Core/ShwattGlobals.h"
#include "Math/FractSupport.h"
#include "DAQ/Clock.h"

void ShmittTrigger(void);
char isMoving(void);

#endif


