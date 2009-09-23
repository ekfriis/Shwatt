#include "XBee/XBee.h"

/*
 * A set of commands to write commands using the XBEE API (w/ escaped characters)
 * communication system.
 *
 */

void XBeeWriteByte(unsigned char byte, unsigned char * checkSum)
{
   // add un-escaped byte to checksum, if desired
   if (checkSum)
      *checkSum += byte;

   // escape control chars
   if (byte == START_FRAME || byte == XOFF || byte == XON || byte == ESCAPE_CHAR)
   {
      SerialWrite(ESCAPE_CHAR);
      byte ^= ESCAPE_XOR;
   }    

   // write the byte
   SerialWrite(byte);
}

void XBeeWriteBytes(unsigned int input, unsigned char * checkSum)
{
   unsigned char input_MSB = (input >> 8) & 0xFF;
   unsigned char input_LSB =  input       & 0xFF;
   XBeeWriteByte( input_MSB, checkSum);
   XBeeWriteByte( input_LSB, checkSum);
}
