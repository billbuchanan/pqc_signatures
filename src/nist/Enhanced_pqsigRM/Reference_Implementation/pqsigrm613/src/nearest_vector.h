#ifndef __NEARESR_VECTOR
#define __NEARESR_VECTOR

#include <stdlib.h>
#include <math.h>
#include "common.h"

void init_decoding(float* pool );
void recursive_decoding_mod(float* y, int r1, int m1, int f, int l, uint16_t *perm1, uint16_t *perm2, matrix* Hrep); 

void y_permute(float *y, const int f, const int r, uint16_t *Q);
void y_depermute(float *y, const int f, const int r, uint16_t *Q);

#endif
