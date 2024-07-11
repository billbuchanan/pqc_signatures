#include "field.h"

#define FIELD_POLY 0x1b

uint8_t field_add(uint8_t x, uint8_t y) {
    return x^y;
}

uint8_t field_mul(uint8_t x, uint8_t y) {
    uint8_t z = 0;
    for (; x; x>>=1) {
        if (x & 1)
            z ^= y;
        if (y & 0x80)
            y = (y<<1) ^ FIELD_POLY;
        else
            y <<= 1;
    }
    return z;
}

uint8_t field_inv(uint8_t x) {
    uint8_t y = field_mul(x,x); //2
    uint8_t z = field_mul(x,y); //3
    y = field_mul(z,z); //6
    y = field_mul(y,y); //12
    z = field_mul(y,z); //15
    y = field_mul(y,y); //24
    y = field_mul(y,y); //48
    y = field_mul(y,z); //63
    y = field_mul(y,y); //126
    y = field_mul(y,x); //127
    return field_mul(y,y); //254
}
