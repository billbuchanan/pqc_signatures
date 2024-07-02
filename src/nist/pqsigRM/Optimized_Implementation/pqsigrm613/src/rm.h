//
// Code for RM code Generation, Decoding.
//

#ifndef __RM_H
#define __RM_H

#include "common.h"

extern const uint16_t rm_dim[7][13];

void rm_gen(matrix* gen, uint32_t r, uint32_t m, 
	uint32_t row_f, uint32_t row_r, uint32_t col_f, uint32_t col_r);

#endif
