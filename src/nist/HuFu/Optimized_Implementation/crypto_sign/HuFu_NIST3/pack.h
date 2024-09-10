#include <stdint.h>
#include <string.h>
#include <math.h>

#include "params.h"

// Pack data into buf from the most significant bit to the least significant bit
// Each element of buf has n bits from the least significant bit
// Example:
// uint32_t a[]={18559,20130};
// uint8_t b[]={1,2,3,4,5,6,7,8};
// uint32_t c[2]={};
// pack(b,8,a,2,15);
// unpack(c,2,b,8,15);

void pack_new(uint8_t *buf, const size_t buf_len, const uint32_t *data, const size_t data_len);
void unpack_new(uint32_t *data, const size_t data_len, const uint8_t *buf, const uint8_t *siged, const size_t buf_len);
void mat_mul_unpack(int32_t *C, const uint8_t *A, const uint8_t *A1, const int32_t *B, int l1, int l2, int l3);
void pack_mat_r(uint8_t *buf, const int32_t *data);
void unpack_mat_r(int32_t *data, const uint8_t *buf, int buf_len);