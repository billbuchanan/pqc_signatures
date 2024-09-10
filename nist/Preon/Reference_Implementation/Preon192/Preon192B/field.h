#ifndef _FIELD_H__
#define _FIELD_H__

#include <stddef.h>
#include <stdint.h>

// #define PROFILE_OP_COUNTS

#ifdef PROFILE_OP_COUNTS
size_t add_count;
size_t field_mul_count;
size_t gf264_mul_count;
size_t gf264_twice_count;
#endif

int is_zero(const uint64_t *a, const size_t field_words);
int field_equal(const uint64_t *a, const uint64_t *b, const size_t field_words);
void field_batch_inverse_and_mul(uint64_t *result, const uint64_t *elements, const size_t elements_len, const uint64_t *mul, const size_t field_words);
void field_add(uint64_t *sum, const uint64_t *a, const uint64_t *b, const size_t field_words);
void field_addEqual(uint64_t *a, const uint64_t *b, const size_t field_words);
void field_sub(uint64_t *difference, const uint64_t *a, const uint64_t *b, const size_t field_words);
void field_subEqual(uint64_t *a, const uint64_t *b, const size_t field_words);
void field_mul(uint64_t *product, const uint64_t *a, const uint64_t *b);
void field_mulEqual(uint64_t *a, const uint64_t *b, const size_t field_words);
void field_pow(uint64_t *result, const uint64_t *a, const size_t exponent, const size_t field_words);
void field_inv(uint64_t *inv, const uint64_t *base);
void field_swap_with_tmp(uint64_t *a, uint64_t *b, uint64_t *tmp, const size_t field_bytesize);
void field_swap(uint64_t *a, uint64_t *b, const size_t field_bytesize);

uint64_t gf264_mul(uint64_t a, uint64_t b);
uint64_t gf264_inv(uint64_t a);

// GF192 = GF64[y]/<y^3+y+1>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF192_inv(uint64_t *inv, const uint64_t *a);
void GF192_mul(uint64_t *product, const uint64_t *a, const uint64_t *b);
void GF192_mulEqual(uint64_t *a, const uint64_t *b);

// GF256 = GF64[y]/<y^4+y^3+y^2+GF64(2)y+GF64(2)>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF256_inv(uint64_t *inv, const uint64_t *a);
void GF256_mul(uint64_t *product, const uint64_t *a, const uint64_t *b);
void GF256_mulEqual(uint64_t *a, const uint64_t *b);

// GF320 = GF64[y]/<y^5+y^2+1>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF320_inv(uint64_t *inv, const uint64_t *a);
void GF320_mul(uint64_t *product, const uint64_t *a, const uint64_t *b);
void GF320_mulEqual(uint64_t *a, const uint64_t *b);

#endif
