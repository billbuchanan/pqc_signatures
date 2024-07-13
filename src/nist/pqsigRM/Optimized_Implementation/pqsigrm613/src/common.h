//
// Common functions for pqsigRM
// 
#ifndef __PQSIGRM_COMMON_H
#define __PQSIGRM_COMMON_H

#include "matrix.h"
#include "rng.h"
#include "parm.h"
#include "rm.h"

#include <openssl/sha.h>
#include <openssl/evp.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

unsigned char* hash_message(unsigned char *s, const unsigned char *m, 
	unsigned long long mlen, unsigned long long i);

int hamming_weight(matrix* e);

void swap(uint16_t *Q, const int i, const int j);

void permutation_gen(uint16_t *Q, uint32_t len);
void partial_permutation_gen(uint16_t* Q);

uint16_t random16(uint16_t n);

#endif 
