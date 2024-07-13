#ifndef _FFT_H__
#define _FFT_H__

#include <stddef.h>
#include <stdint.h>

void fft(uint64_t *result, const uint64_t *v, const size_t v_len, const size_t field_words, const size_t domain_size, const uint64_t *domain_shift);
void ifft(uint64_t *result, const uint64_t *v, const size_t field_words, const size_t domain_size, const uint64_t *domain_shift);

#endif
