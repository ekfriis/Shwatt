#include "Clock.h"

uint8_t clockMSB;
#define clockLSB (TCNT2)

void setupClock(void)
{
   TCCR2B |= TIMER_2_PRESCALE_MASK;
   TCCR2B &= ~( (1 << FOC2A)  | (1 << FOC2B)  );                //turn off forced outptu comare
   TCCR2A &= ~( (1 << WGM20)  | (1 << WGM21)  );                //turn off WGM20,21 (normal mode)
   TCCR2A &= ~( (1 << COM2A0) | (1 << COM2A1) );                //turn off COM2A0,21 (normal mode)
   TCNT2   = TIMER_2_INIT_VALUE;
   sbi(TIMSK2, TOIE2);                                          //enable overflow interrupt
}

uint16_t rawTime(void)
{
   return ( ( ( (uint16_t)clockMSB ) << 8) | clockLSB );
}

uint8_t rawTimeMSB(void)
{
   return clockMSB;
}

uint8_t rawTimeLSB(void)
{
   return clockLSB;
}


ISR(TIMER2_OVF_vect) 
{
   clockMSB++;
   // call external handler
   clockOverflowHandler();
}

time_type time(uint16_t rawInterval)
{
   return kbits( ((uint32_t)rawInterval) << ScaleTime);

   // DEBUG
   // this werks
   //uint32_t doot = 0xFFFF0000;
   //return kbits(doot);
}



