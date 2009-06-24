#ifndef EK_HardwareData_H
#define EK_HardwareData_H

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define HIGH 0x1
#define LOW  0x0

//#define Fract_Positive_One = ( 32765.9/32768.0 ) //as close as you can get
#define Fract_Positive_One = ( 0.99996 ) //as close as you can get

#define INPUT 0x0
#define OUTPUT 0x1

// hardware constants, etc
#define XbeeAddressMSB          0x0013a200
#define XbeeAddressMSBBytSum    0xB5

#define HeadXBeeAddress         0x4030cfba
#define HeadXBeeAddressBytSum   0xF9

#define RightXBeeAddress        0x4030cf92
#define RightXBeeAddressBytSum  0xD1

#define MonXBeeAddress          0x4030cfbd
#define MonXBeeAddressBytSum    0xFC

#define LeftXBeeAddress         0x402d2350
#define LeftXBeeAddressBytSum   0xE0

#define TIMER_2_PRESCALE        256
#define TIMER_2_PRESCALE_MASK   ((1 << CS22) | (1 << CS21) | (0 << CS20)) //256 precale
#define TIMER_2_INIT_VALUE      120    //this matches the rate above, but a bit faster

#define DAQ_RATE                ((F_CPU)/(2.0*TIMER_2_PRESCALE*(256-TIMER_2_INIT_VALUE)))
#define FUNDAMENTAL_RATE        (114.88) //Hz

#define ARDUINO_SUPPLY_VOLTAGE  (5.0)
#define ADC_REF_VOLTAGE         (3.3)
#define IMU_SUPPLY_VOLTAGE      (3.3)

#define ADC_VOLTS_PER_BIT       (ADC_REF_VOLTAGE/1024.0)

//ADXL 330 data
#define ADXL330_BIAS_EXPECT     ((IMU_SUPPLY_VOLTAGE/2)/ADC_VOLTS_PER_BIT)
#define ADXL330_SCALE_EXPECT    (1.0/(0.3/ADC_VOLTS_PER_BIT))
//Q value for bias terms
#define ADXL330_BIAS_NOISE      ((int16_t)5)//((int16_t)((10/1000)(10/1000)*((int32_t)(1) << 13)))
                                // ADXL bias drift is +-1mg/degC, assume +- 2 mg
//Invensense IDG-300
#define IDG300_BIAS_EXPECT      ((uint16_t)((IMU_SUPPLY_VOLTAGE/2)/ADC_VOLTS_PER_BIT))
// 2mV per Degree per Second            --> (180/pi)
// is 114.59 mV per radian per second   --> (100.21)
// is 11483.22 mV per radian per dt     --> (1/Pi)
#define IDG300_SCALE_EXPECT     (1.0/(11.48322/ADC_VOLTS_PER_BIT))
// is 3655.22 mV per binary angle per dt
//#define IDG300_SCALE_EXPECT     (1.0/(3.65522/ADC_VOLTS_PER_BIT))


#define ArduinoPin11 2 
#define ArduinoPin12 3 

#define XaxisPinMuxValue ( (1 << REFS0) | 0x00)
#define ZaxisPinMuxValue ( (1 << REFS0) | 0x01)
#define YratePinMuxValue ( (1 << REFS0) | 0x02)

#define MyPortBMask (1 << ArduinoPin11 | 1 << ArduinoPin12)
//#define MyPortBMask 0xFF

//#define WriteStatusPins(BYTE) (MyPortBMask && ( ((BYTE & 0x1) << ArduinoPin12) | (((BYTE & 0x2) >> 1) << ArduinoPin11)))
#define StatusPins PORTB
#define StatusPin0 (1 << 2)
#define StatusPin1 (1 << 3)


// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// ATMEL ATMEGA8 & 168 / ARDUINO
//
//                  +-\/-+
//            PC6  1|    |28  PC5 (AI 5)
//      (D 0) PD0  2|    |27  PC4 (AI 4)
//      (D 1) PD1  3|    |26  PC3 (AI 3)
//      (D 2) PD2  4|    |25  PC2 (AI 2)
// PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
//      (D 4) PD4  6|    |23  PC0 (AI 0)
//            VCC  7|    |22  GND
//            GND  8|    |21  AREF
//            PB6  9|    |20  AVCC
//            PB7 10|    |19  PB5 (D 13)
// PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
// PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//      (D 7) PD7 13|    |16  PB2 (D 10) PWM
//      (D 8) PB0 14|    |15  PB1 (D 9) PWM
//                  +----+
//
// (PWM+ indicates the additional PWM pins on the ATmega168.)

#endif
