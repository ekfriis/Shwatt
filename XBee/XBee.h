#ifndef Shwatt_XBEE
#define Shwatt_XBEE

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../FractSupport.h"

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
#define XBeeContains_HMatrix            (1 << 7)

#define XBeeContains_StateLength                10
//#define XBeeContains_StateErrorsLength          XBeeContains_StateLength
// DEBUG? maybe permananent w/ new accum stateerrors
#define XBeeContains_StateErrorsLength          14
#define XBeeContains_MeasuresLength             10
#define XBeeContains_MeasureNoiseLength         XBeeContains_MeasuresLength
#define XBeeContains_MeasureBiasesLength        XBeeContains_MeasuresLength
#define XBeeContains_HMatrixLength              XBeeContains_StateLength
#define XBeeContains_ShwattStatusLength         3
#define XBeeContains_PerformanceLength          10

// XBee stuff
#define START_FRAME             0x7E
#define XOFF                    0x13
#define XON                     0x11
#define ESCAPE_CHAR             0x7D
#define ESCAPE_XOR              0x20


void SetBroadcastData(uint8_t options);

void BroadcastData(void);

struct XBeeFrameInBuffer
{
   unsigned char rx_tail;
   unsigned int  length;
   unsigned char checksum;  //we keep this here so we can catch errors incase we overflow
                            //the frame buffer
};

/// returns number of frames in buffer
uint8_t XBeeFramesAvailable(void);
/// when we are done reading it
void TrashCurrentFrame(void);
uint8_t FrameDataAvailable(void);
/// returns current frame length
uint16_t XBeeFrameLength(void);
/// returns current frame checksum
uint8_t XBeeFrameChecksum(void);
/// read a byte from the XBee stream
uint8_t ReadXBeeDataByte(void);
/// read an int from the Xbee stream
uint16_t ReadXBeeDataInt(void);

extern void XbeeFrameHandler(uint8_t);

void SerialWrite(unsigned char byte);

void XBeeWriteByte(unsigned char byte  , unsigned char* checkSum);
void XBeeWriteBytes(unsigned int input , unsigned char* checkSum);
void XBeeWriteFract(_Fract input       , unsigned char* checkSum);
void XBeeWriteAccum(_Accum input       , unsigned char* checkSum);

void BeginSerial(long baud);

#endif
