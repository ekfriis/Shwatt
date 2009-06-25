#include "XBee/XBee.h"
#include "Core/Parameters.h"

//buffer the outgoing UART serial stream
unsigned char xb_tx_buffer[TX_BUFFER_SIZE];
volatile unsigned char xb_tx_buffer_head;
volatile unsigned char xb_tx_buffer_tail;

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

