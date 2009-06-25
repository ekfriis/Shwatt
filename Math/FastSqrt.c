#include "TrigLookup.h"

uint16_t fastSqrt(uint16_t num) {
   //from wikipedia
   uint16_t op = num;
   uint16_t res = 0;
   uint16_t one = 1 << 14; // The second-to-top bit is set: 1L<<30 for long

   // "one" starts at the highest power of four <= the argument.
   while (one > op)
      one >>= 2;

   while (one != 0) {
      if (op >= res + one) {
         op -= res + one;
         res += one << 1;
      }
      res >>= 1;
      one >>= 2;
   }
   return res;
}
