#include "FractSupport.h"

int bitsr(_Fract __input)
{
   int __out;
   memcpy(&__out, &__input, sizeof __out);
   return __out;
}

_Fract rbits(int __input)
{
   _Fract __out;
   memcpy(&__out, &__input, sizeof __out);
   return __out;
}

