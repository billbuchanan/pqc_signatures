#ifndef COMPRESS_H_
#define COMPRESS_H_

#include "api.h"
#include "keccak/keccak.h"

extern void hashArray(uint8_t *hash_out, const uint8_t *array_in, const uint64_t length);

extern void compressArray(uint8_t *carr_out, const uint64_t* arr_in, const int arr_size);

extern void decompressArray(uint64_t *darr_out, const uint8_t* arr_in, const int arr_size);

#endif

