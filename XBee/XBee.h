#ifndef Shwatt_XBEE
#define Shwatt_XBEE

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdfix.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/* 
 * XBee data codes
 *
 * Always send one byte with contents, then data
 *
 */ 

#define XBeeContains_ShwattStatus       (1 << 0)
#define XBeeContains_State              (1 << 1)
#define XBeeContains_StateErrors        (1 << 2)
#define XBeeContains_Measures           (1 << 3)
#define XBeeContains_MeasureNoise       (1 << 4)
#define XBeeContains_MeasureBiases      (1 << 5)
#define XBeeContains_Performance        (1 << 6)
#define XBeeContains_InterCom          (1 << 7) //inter-shwatt communication

// XBee stuff
#define START_FRAME             0x7E
#define XOFF                    0x13
#define XON                     0x11
#define ESCAPE_CHAR             0x7D
#define ESCAPE_XOR              0x20


void SetBroadcastData(uint8_t options);
void BroadcastData(void);

void BeginSerial(long baud);
void SerialWrite(unsigned char byte);
void EnableRemoteShwattInput(void);;

void XBeeWriteByte(unsigned char byte  , unsigned char* checkSum);
void XBeeWriteBytes(unsigned int input , unsigned char* checkSum);
void XBeeWriteFract(_Fract input       , unsigned char* checkSum);
void XBeeWriteAccum(_Accum input       , unsigned char* checkSum);

#endif
