#ifndef ShmittTriggered_h
#define ShmittTriggered_h

#include <stdfix.h>
#include <inttypes.h>
#include <avr/io.h>
#include "ShwattGlobals.h"
#include "FractSupport.h"
#include "Clock.h"

void ShmittTrigger(void);
char isMoving(void);

#endif


