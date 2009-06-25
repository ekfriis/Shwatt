#include "XBee/XBee.h"

/*
 * A set of commands to write commands using the XBEE API (w/ escaped characters)
 * communication system.
 *
 */

void XBeeWriteByte(unsigned char byte, unsigned char * checkSum)
{
   if (byte == START_FRAME || byte == XOFF || byte == XON || byte == ESCAPE_CHAR)
   {
      SerialWrite(ESCAPE_CHAR);
      byte ^= ESCAPE_XOR;
   } else if (checkSum)
      *checkSum += byte;
   SerialWrite(byte);
}

void XBeeWriteBytes(unsigned int input, unsigned char * checkSum)
{
   unsigned char input_MSB = (input >> 8) & 0xFF;
   unsigned char input_LSB =  input       & 0xFF;
   XBeeWriteByte( input_MSB, checkSum);
   XBeeWriteByte( input_LSB, checkSum);
}
