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
#include "Parameters.h"
#

// bitfield status flags
#define XBeeRecieving ((1 << 0) & 0xFF)
#define XBeeNeedMSB   ((1 << 1) & 0xFF)
#define XBeeNeedLSB   ((1 << 2) & 0xFF)
#define XBeeEscapeOn  ((1 << 3) & 0xFF)

// buffer the incoming UART serial stream
unsigned char xb_rx_buffer[RX_BUFFER_SIZE];
volatile unsigned char xb_rx_buffer_head;
volatile unsigned char xb_rx_buffer_tail;

//buffer the outgoing UART serial stream
unsigned char xb_tx_buffer[TX_BUFFER_SIZE];
volatile unsigned char xb_tx_buffer_head;
volatile unsigned char xb_tx_buffer_tail;

struct   XBeeFrameInBuffer frame_buffer[FRAME_BUFFER_SIZE];
unsigned char              frame_buffer_head;
unsigned char              frame_buffer_next;
unsigned char              frame_buffer_tail;

volatile unsigned int    bytesToGet;

volatile unsigned char XBeeStatusFlags;

// variables to keep track of reading the buffers

uint16_t currentFrameDataToRead;

unsigned char XBeeFramesAvailable()
{
   return (FRAME_BUFFER_SIZE + frame_buffer_head - frame_buffer_tail) % FRAME_BUFFER_SIZE;
}

void SetupFrameInfo(void)
{
   currentFrameDataToRead = frame_buffer[frame_buffer_tail].length;
}


/// mark this frame as read
void TrashCurrentFrame(void)
{
   frame_buffer_tail = (frame_buffer_tail + 1) % FRAME_BUFFER_SIZE;
   // fake "read" this frame's data out of the serial buffer
   // now the data index pointer of the frame_tail points to the tail of the rx data
   // and we have deleted the oldest frame
   if (frame_buffer_tail != frame_buffer_head)
   {
      xb_rx_buffer_tail = frame_buffer[frame_buffer_tail].rx_tail;
      SetupFrameInfo();
   }
   else
   {
      xb_rx_buffer_tail = xb_rx_buffer_head; //no other frames exist, so no serial data..
   }

}

uint8_t FrameDataAvailable(void)
{
   //TODO
   return 0;
}



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

void BeginSerial(long baud)
{
#if defined(__AVR_ATmega168__)
	UBRR0H = ((F_CPU / 16 + baud / 2) / baud - 1) >> 8;
	UBRR0L = ((F_CPU / 16 + baud / 2) / baud - 1);
	
	// enable rx and tx
	sbi(UCSR0B, RXEN0);
	sbi(UCSR0B, TXEN0);
	
	// enable interrupt on complete reception of a byte
	sbi(UCSR0B, RXCIE0);
#else
	UBRRH = ((F_CPU / 16 + baud / 2) / baud - 1) >> 8;
	UBRRL = ((F_CPU / 16 + baud / 2) / baud - 1);
	
	// enable rx and tx
	sbi(UCSRB, RXEN);
	sbi(UCSRB, TXEN);
	
	// enable interrupt on complete reception of a byte
	sbi(UCSRB, RXCIE);
#endif
	
	// defaults to 8-bit, no parity, 1 stop bit
}

void SerialWrite(unsigned char byte)
{
   //wait until the buffer is not full
   unsigned char next_in_buffer = (xb_tx_buffer_head + 1) % TX_BUFFER_SIZE;
   while(next_in_buffer == xb_tx_buffer_tail) //do nothing until the tail is sent out
   {
#if defined(__AVR_ATmega168__)
      UCSR0B |= (1 << UDRIE0);
#else
      UCSRB  |= (1 << UDRIE0);
#endif
   }
   //write into the buffer
   xb_tx_buffer[xb_tx_buffer_head] = byte;
   xb_tx_buffer_head = next_in_buffer;
   //enable the TX_DATA_REGISTER_EMPTY interrupt (if it isn't already)
   //so this gets clocked out when the UART is ready
#if defined(__AVR_ATmega168__)
   UCSR0B |= (1 << UDRIE0);
#else
   UCSRB  |= (1 << UDRIE0);
#endif
}

//ISR routine for an empty transmit buffer.  Enabled in SerialWrite(...)
//takes next object in tx buffer.  If no more items in buffer, disable this interrupt
ISR(USART_UDRE_vect)
{
   //if we have sent everything and there is nothing to send, disable interrupt and exit
   if(xb_tx_buffer_head == xb_tx_buffer_tail)
   {
#if defined(__AVR_ATmega168__)
      UCSR0B &= ~(1 << UDRIE0); //disable interrupt
#else
      UCSRB  &= ~(1 << UDRIE0);
#endif
      return;
   }
   //otherwise, write the next value from tx buffer to the uart
#if defined(__AVR_ATmega168__)
   UDR0 = xb_tx_buffer[xb_tx_buffer_tail];
#else
   UDR  = xb_tx_buffer[xb_tx_buffer_tail];
#endif
   //increment buffer tail
   xb_tx_buffer_tail = (xb_tx_buffer_tail + 1) % TX_BUFFER_SIZE;
}


#define DISABLE_SERIAL_RX
#ifndef DISABLE_SERIAL_RX
#if defined(__AVR_ATmega168__)
SIGNAL(SIG_USART_RECV)
#else
SIGNAL(SIG_UART_RECV)
#endif
{
#if defined(__AVR_ATmega168__)
   unsigned char c = UDR0;
#else
   unsigned char c = UDR;
#endif
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

   //is this the beginning of a new xbee frame?
   if( !(XBeeStatusFlags & XBeeRecieving) )
   {
      if (c == START_FRAME) { //yes!
         XBeeStatusFlags |= ( XBeeRecieving | XBeeNeedMSB | XBeeNeedLSB );
         // we don't store this in the serial buffer
      }
      //ignore anything else!
      return;
   }
   //we are in an XBee frame.  check if we have gotten the length
   if( XBeeStatusFlags & XBeeNeedMSB )
   {
      bytesToGet = (c << 8) & 0xFF00;
      XBeeStatusFlags &= ~(XBeeNeedMSB);
      return;
   }
   if( XBeeStatusFlags & XBeeNeedLSB )
   {
      bytesToGet |= c & 0x00FF;
      XBeeStatusFlags &= ~(XBeeNeedLSB);
      //now we are in the frame, and have the length
      //begin setting up the Xbee frame buffer
      //frame_buffer_next indexes the frame we are currently working on populating
      frame_buffer_next = (frame_buffer_head + 1) % FRAME_BUFFER_SIZE;
      if(frame_buffer_next == frame_buffer_tail)
      {
         //buffer overflow!  tough titty, we are overwriting the oldest frame
         TrashCurrentFrame();

         //now setup the new frame
         frame_buffer[frame_buffer_next].rx_tail = xb_rx_buffer_head; //TODO: check this isn't off by 1...
         frame_buffer[frame_buffer_next].length  = bytesToGet;
      }
      return;
   }

   //if we reach this point, we are reading the data in the buffer

   //if bytestToGet == 0; then we are at the check sum, store it in the frame object
   //and reset everything.  Advance the frame buffer head to indicate new data to read
   if(bytesToGet-- == 0) //also decrements
   {
      frame_buffer[frame_buffer_next].checksum = c;
      frame_buffer_head = frame_buffer_next;
      XBeeStatusFlags &= ~(XBeeRecieving);
      XbeeFrameHandler(frame_buffer_next);  
      return;
   }

   //otherwise we are in the API cmd/data area, save this in the serial buffer

   int i = (xb_rx_buffer_head + 1) % RX_BUFFER_SIZE;

   // if we should be storing the received character into the location
   // just before the tail (meaning that the head would advance to the
   // current location of the tail), we're about to overflow the buffer
   // and so we don't write the character or advance the head.
   if (i != xb_rx_buffer_tail) {
      xb_rx_buffer[xb_rx_buffer_head] = c;
      xb_rx_buffer_head = i;
   }
}
#endif
