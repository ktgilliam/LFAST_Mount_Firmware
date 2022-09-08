#include <stdint.h>

typedef union
{
    double dblVal;
    uint8_t   charVals[sizeof(double)];
}  Double2Char_t;