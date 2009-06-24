#ifndef Shwatt_DAQFunction_h
#define Shwatt_DAQFunction_h

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include "HardwareData.h"
#include "Clock.h"

#define AcquiringData   (1 << 3)
#define WaitingForYrate (1 << 2)
#define WaitingForXaxis (1 << 1)
#define WaitingForZaxis (1 << 0)

unsigned char ShwattDaqState;

/* ShwattDaqState stores current conversion progress
 *
 * AcquiringData:  1, when all measurements have not been taken
 * WaitingFor____ indicates whether the corresponding measurements have been taken
 *
 */

extern void ShwattDaqComplete(void);

//void SetupADC(volatile uint16_t* myXaxis, volatile uint16_t* myZaxis, 
//              volatile uint16_t* myYrate, volatile uint8_t* timeMSB, volatile uint8_t* timeLSB);
void SetupADC(volatile uint16_t* myXaxis, volatile uint16_t* myZaxis, 
              volatile uint16_t* myYrate);
uint16_t DataReady(void);
unsigned char AcquireData(void);

uint16_t GetXaxis(void);
uint16_t GetZaxis(void);
uint16_t GetYrate(void);

#endif
