#include "XBee/XBee.h"
#include "Math/FractSupport.h"

/*
 * Write various fixed-point data types to an XBEE radio
 *
 */

void XBeeWriteFract(_Fract input, unsigned char * checkSum)
{
   XBeeWriteBytes( bitsr(input), checkSum);
}

void XBeeWriteAccum(_Accum input, unsigned char * checkSum)
{
   long myBits = bitsk(input);
   unsigned int MSBs = (myBits & 0xFFFF0000) >> 16;
   unsigned int LSBs = (myBits & 0x0000FFFF);
   XBeeWriteBytes(MSBs, checkSum);
   XBeeWriteBytes(LSBs, checkSum);
}


