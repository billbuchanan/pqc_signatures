#include "rm.h"

const uint16_t rm_dim[7][13]  
={{1,1,1,1,1,1,1,1,1,1,1,1,1}, 
  {0,2,3,4,5,6,7,8,9,10,11,12,13}, 
  {0,0,4,7,11,16,22,29,37,46,56,67,79}, 
  {0,0,0,8,15,26,42,64,93,130,176,232,299}, 
  {0,0,0,0,16,31,57,99,163,256,386,562,794}, 
  {0,0,0,0,0,32,63,120,219,382,638,1024,1586}, 
  {0,0,0,0,0,0,64,127,247,466,848,1486,2510}};

matrix* rm_gen(matrix* gen, int r, int m, uint16_t row_f, uint16_t row_r, 
	uint16_t col_f, uint16_t col_r) {
	if (r == 0) {
		for (int i = 0; i < (1<<m); i++) {
			set_element(gen, row_f, col_f + i, 1);
		}
	} else if (m == r) {
		for (int i = 0; i < (1<<m); i++) {
			 set_element(gen, row_f + i, col_f + i, 1);
		}
	} else {
		uint16_t col_m = (col_f+col_r)/2;
		rm_gen(gen, r,   m-1, 	row_f, row_f + rm_dim[r][m-1], col_f, 	col_m);
		rm_gen(gen, r,   m-1, 	row_f, row_f + rm_dim[r][m-1], col_m, 	col_r);
		rm_gen(gen, r-1, m-1,  	row_f + rm_dim[r][m-1], row_r, col_m, 	col_r);
	}
	return gen;
}
