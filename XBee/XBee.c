/*
  wiring_serial.c - serial functions.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $
*/

#include "XBee/XBee.h"
#include "Core/Parameters.h"
#include "Core/ShwattGlobals.h"
#include "Math/FractSupport.h"
#include <inttypes.h>

// bitfield status flags
#define XBeeRecieving ((1 << 0) & 0xFF)
#define XBeeNeedMSB   ((1 << 1) & 0xFF)
#define XBeeNeedLSB   ((1 << 2) & 0xFF)
#define XBeeEscapeOn  ((1 << 3) & 0xFF)

volatile unsigned int    bytesToGet;
volatile uint16_t        lastTwoBytes; //stores last word recieved
volatile uint8_t         bytesGot;
volatile uint8_t         checksum;

volatile unsigned char XBeeStatusFlags;

uint8_t sumBytes(uint16_t input)
{
   uint8_t output = input & 0x00FF;
   output += (input > 8) & 0x00FF;
   return output;
}

void EnableRemoteShwattInput(void)
{
   // enable interrupt on complete reception of a byte
   sbi(UCSR0B, RXCIE0);
}

SIGNAL(SIG_USART_RECV)
{
   // read the UART
   unsigned char c = UDR0;

   // are we currently not recieving a frame?
   if( !(XBeeStatusFlags & XBeeRecieving) )
   {
      //is this the beginning of a new xbee frame?
      if (c == START_FRAME) { //yes!
         XBeeStatusFlags |= ( XBeeRecieving | XBeeNeedMSB | XBeeNeedLSB );
         bytesGot = 1;
      }
      //ignore anything else!
      return;
   }

   //un-escape, if necessary
   if(c == ESCAPE_CHAR)
   {
      XBeeStatusFlags |= XBeeEscapeOn;
      return;
   }
   if(XBeeStatusFlags & XBeeEscapeOn)
   {
      c ^= ESCAPE_XOR;
      XBeeStatusFlags &= ~(XBeeEscapeOn);
   }

   //increment here (escape chars not included in length)
   bytesGot++;

   if( bytesGot > 31 ) //done w/the the frame
   {
      // stop parsing this frame and wait untl the next one
      XBeeStatusFlags &= ~(XBeeRecieving);
      return;
   }

   // shift last recieved byte over
   lastTwoBytes <<= 8;
   lastTwoBytes |= (0x00FF & c);

   // skip all the even bytes (we save the value in the last two bytes)
   if( bytesGot % 2 == 0 )
      return;

   _Fract lastTwoBytesFract;
   lastTwoBytesFract = bitsr(lastTwoBytes);

   switch( bytesGot )
   {
      // byte 1 is START_FRAME
      // byte 2 is MSB of length
      case 3:                                   //length LSB
         bytesToGet = lastTwoBytes;
         break;
      // byte 4 is actually the API id
      case 5:                                   //API identifier          
         if( ((lastTwoBytes >> 8) & 0x00FF) != 0x90 ) // we only care about rx packets 
         {
            // wait until the next frame
            XBeeStatusFlags &= ~(XBeeRecieving);
         }
         checksum = 0;
         break;
      // bytes 5-15 are bunch of stupid address crap
      // bytes 16 & 17 give a flag (0xABCD) to indicate the external data frame
      case 17:
         if( lastTwoBytes != 0xABCD )
         {
            // if this isn't a XBee external data frame, we don't care 
            XBeeStatusFlags &= ~(XBeeRecieving);
         }
         checksum += sumBytes(0xABCD);
         break;
      // byte 18 is first data byte -> MSB of ext phi
      case 19:                                  //LSB of external phi
         measures[extPhiIndex] = bitsr(lastTwoBytes);
         checksum += sumBytes(lastTwoBytes);
         break;
      // byte 20 is MSB of ext phiDot
      case 21:                                  //LSB of external phiDot
         measures[extPhiDotIndex] = bitsr(lastTwoBytes);
         checksum += sumBytes(lastTwoBytes);
         break;
      // byte 22 is MSB of ext phiError
      case 23:                                  //LSB of ext Phi error
         measureNoise[extPhiIndex] = bitsr(lastTwoBytes);
         checksum += sumBytes(lastTwoBytes);
         break;
      // byte 22 is MSB of ext phiDotError
      case 25:
         measureNoise[extPhiDotIndex] = bitsr(lastTwoBytes);
         checksum += sumBytes(lastTwoBytes);
         break;
      case 27:
         checksum += sumBytes(lastTwoBytes);
         // ext xAxis, not implemented
         break;
      case 29:
         checksum += sumBytes(lastTwoBytes);
         // ext zAxis, not implemented
         break;
      // we keep the local checksum in byte 30
      case 31:
         //TODO: send TriggerState as well in MSB?
         checksum += ((lastTwoBytes >> 8) & 0x00FF);
         if( checksum == 0x69 )  //woop woop checksum is OK
         {
            KalmanState |= ExternalDataBit;
         } 
         // TODO: better error handling?
         // dont' care about rest of frame
         XBeeStatusFlags &= ~(XBeeRecieving);
         break;
   }
   return;
}
