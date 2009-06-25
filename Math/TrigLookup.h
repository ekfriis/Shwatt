#ifndef ARDUINO_SINELOOKUPTABLEINTERFACE_H
#define ARDUINO_SINELOOKUPTABLEINTERFACE_H

/*
 * Quarter-wave sine/cosine trig lookup table
 *
 * Author: Evan Friis ekfriis@gmail.com
 *
 * Input takes input as a FixedPoint Q0.15 binary angle
 * (see http://en.wikipedia.org/wiki/Binary_scaling#Binary_angles)
 *
 * i.e. 0x0000 =  0     degrees
 *      0x7FFF =  179.9 degrees
 *      0xFFFF = -180   degrees
 *
 * The output is also returned as Q0.15 number with
 *
 *      0x0000 =  0
 *      0x7FFF =  0.999969482421875
 *      0xFFFF = -1
 */

// for testing on a local machine
//#define TrigLookup_UnitTest

//#define FullByteLookupTable
//#define HalfByteLookupTable
#define QuarterByteLookupTable
//#define EighthByteLookupTable

#ifdef FullByteLookupTable
  #define MAX_TABLE_INDEX          0xFF
  #define LOOKUP_INDEX_BITS        6
  #define LOOKUP_LSB_MASK          0x003F  //gives the lowest 6 bits
#endif

#ifdef HalfByteLookupTable
  #define MAX_TABLE_INDEX          0x7F
  #define LOOKUP_INDEX_BITS        7
  #define LOOKUP_LSB_MASK          0x007F  //gives the lowest 7 bits
#endif

#ifdef QuarterByteLookupTable
  #define MAX_TABLE_INDEX          0x3F
  #define LOOKUP_INDEX_BITS        8
  #define LOOKUP_LSB_MASK          0x00FF  //gives the lowest 8 bits
#endif

#ifdef EighthByteLookupTable
  #define MAX_TABLE_INDEX          0x1F
  #define LOOKUP_INDEX_BITS        9
  #define LOOKUP_LSB_MASK          0x01FF  //gives the lowest 9 bits
#endif

//#define LOOKUP_PI_RADS           0x7FFF
//#define LOOKUP_HPI_RADS          0x3FFF
#define LOOKUP_PI_RADS           0x8000
#define LOOKUP_HPI_RADS          0x4000

#ifndef TrigLookup_UnitTest
#include <avr/pgmspace.h>
typedef prog_int16_t lookup_type;
#else
#include <iostream> //for stdint defintions...
typedef  int16_t lookup_type;
#endif

#include "Math/FractSupport.h"

// for sin(x)
//    If x > PI (i.e. X < 0), the negative flag is set, x is incremented by PI.
//    This puts x at its corresponding place in the upper two quadrants.
//    If x > PI/2, flag flipIndex is set and x is computed as the distance from PI;
//    since sine(x) is symmetric about PI/2 the output is invariant of this change.

int16_t findSineLookupValue(unsigned char input);

_Fract         LookupSine(_Fract theta);
_Fract         LookupCosine(_Fract theta);

int16_t        LookupSineInt(int16_t theta);
int16_t        LookupCosineInt(int16_t theta);

uint16_t       fastSqrt(uint16_t num);

#endif
