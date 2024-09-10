#ifndef _POLY_H__
#define _POLY_H__

#include <stddef.h>
#include <stdint.h>

#include "domain.h"
#include "params.h"

int poly_add(uint64_t *sum, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words);
void poly_addEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words);
void poly_scalarMul(uint64_t *product, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t field_words);
void poly_scalarMulEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t field_words);
void poly_mul(uint64_t *product, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words);
void poly_mulEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words);
void poly_div(uint64_t *quotient, uint64_t *remainder, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words);
void poly_eval(uint64_t *result, const uint64_t *poly, const size_t poly_len, const uint64_t *x, const size_t field_words);
void poly_eval_over_domain(uint64_t *result, const uint64_t *poly, const size_t poly_len, const Domain *domain, const size_t field_words, const size_t field_bytesize);
size_t poly_deg(const uint64_t *poly, const size_t poly_len, const size_t field_words);

#endif
