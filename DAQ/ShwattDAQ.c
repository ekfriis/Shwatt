#include "ShwattDAQ.h"

// pointers to where to store the data
volatile uint16_t* Xaxis;
volatile uint16_t* Zaxis;
volatile uint16_t* Yrate;

void DoAConversion(void); //private prototype

/*
 * Keep track of the time that between measurements
 * for use in integration.  The time is taken after the Yrate is computed
 * as it is the one directly used in integration
 */

uint16_t  lastDaqTime_;    
uint16_t  currentDaqTime_; 

uint16_t GetXaxis(void) { return *Xaxis; };
uint16_t GetZaxis(void) { return *Zaxis; };
uint16_t GetYrate(void) { return *Yrate; };

//void SetupADC(volatile uint16_t* myXaxis, volatile uint16_t* myZaxis, 
//              volatile uint16_t* myYrate, volatile uint8_t* timeMSB, volatile uint8_t* timeLSB)
void SetupADC(volatile uint16_t* myXaxis, volatile uint16_t* myZaxis, 
              volatile uint16_t* myYrate)
{
   Xaxis = myXaxis;
   Zaxis = myZaxis;
   Yrate = myYrate;

   ShwattDaqState = 0;

   sbi(ADCSRA, ADPS2); // ADC clock prescale 128
   sbi(ADCSRA, ADPS1);
   sbi(ADCSRA, ADPS0);
   cbi(ADCSRA, ADATE); // disable ADC auto trigger
   sbi(ADCSRA, ADIE);  // enable ADC interrupt
   cbi(ADCSRA, ADEN);  // enable the ADC
   currentDaqTime_ = rawTime();
   lastDaqTime_    =  0;
}

uint16_t DataReady(void)
{
   // check if AcquiringData is set, but the flags for the individual 
   // measures are not, indicating they have all been acquired
   if (ShwattDaqState == AcquiringData)
   {
   //StatusPins = StatusPin1 | StatusPin0;
      // mark the current values as read
      ShwattDaqState = 0x0;
      return (currentDaqTime_ - lastDaqTime_);
   }
   return 0x0;
}

unsigned char AcquireData(void)
{
   // if we aren't in the middle of a DAQ, or if the last value hasn't 
   // been read yet
   if ( !(ShwattDaqState & AcquiringData) ) 
   {
      // Set the state to 'want all', have none'
      ShwattDaqState = (AcquiringData | WaitingForYrate | WaitingForXaxis | WaitingForZaxis); 
      // Enable the ADC
      sbi(ADCSRA, ADEN);
      // Start the chain of conversions
      DoAConversion();
      return 0x1; // return OK
   } 
   else if (ShwattDaqState == AcquiringData) 
   {
      //this point occurs if the data was not read
      //between the last successful DAQ and the current request.
      //recall the handler for completed data.  (It was probably blocked before)
      //ShwattDaqComplete();
      return 0x2;
   }

   // return failed
   return 0x0;
}

void DoAConversion(void)
{
   if (ShwattDaqState & WaitingForXaxis)
   {
      lastDaqTime_ = currentDaqTime_;
      ADMUX = XaxisPinMuxValue; //set pin to read
      sbi(ADCSRA, ADSC);        //start ADC
   } else if (ShwattDaqState & WaitingForZaxis)
   {
      ADMUX = ZaxisPinMuxValue;
      sbi(ADCSRA, ADSC);
   } else if (ShwattDaqState & WaitingForYrate)
   {
      ADMUX = YratePinMuxValue;
      sbi(ADCSRA, ADSC);
   } else
   {
      cbi(ADCSRA, ADEN);   //disable ADC
      ShwattDaqComplete(); //handler function for the end of the DAQ
   }

}

ISR(ADC_vect)
{
//DEBUG
   uint16_t DataValue = ADCL & 0x0FF;
   DataValue |= ( ADCH << 8 );
   if (ShwattDaqState & WaitingForXaxis)
   {
      *Xaxis = DataValue;
      ShwattDaqState &= ~(WaitingForXaxis);
   } else if (ShwattDaqState & WaitingForZaxis)
   {
      *Zaxis = DataValue;
      ShwattDaqState &= ~(WaitingForZaxis);
   } else if (ShwattDaqState & WaitingForYrate)
   {
      currentDaqTime_ = rawTime();
      *Yrate = DataValue;
      ShwattDaqState &= ~(WaitingForYrate);
   }
   DoAConversion();
}
