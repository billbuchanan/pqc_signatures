//
// Code for RM code Generation, Decoding.
//

#ifndef __RM_H
#define __RM_H

#include "common.h"

extern const uint16_t rm_dim[7][13]; 

matrix* rm_gen(matrix* gen, int r, int m, uint16_t row_f, uint16_t row_r, uint16_t col_f, 
	uint16_t col_r);

#endif
