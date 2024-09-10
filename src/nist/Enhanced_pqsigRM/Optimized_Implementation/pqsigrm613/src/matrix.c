#include <immintrin.h>  // For AVX2 instructions

#include "matrix.h"

matrix* new_matrix (uint32_t nrows, uint32_t ncols)
{
    matrix* mat;
    mat = (matrix*) malloc (sizeof (matrix));
    mat->nrows = nrows; 
    mat->ncols = ncols;
    mat->colsize = (ncols + 63)/64;
    mat->elem = (uint64_t**)malloc(nrows * sizeof(uint64_t*));
    for (uint32_t i = 0; i < nrows; i++)
    {
        mat->elem[i] = (uint64_t*)calloc(mat->colsize, sizeof(uint64_t));
    }
    mat->made_with_import = 0;
    return mat;
}

matrix* new_matrix_with_pool (uint32_t nrows, uint32_t ncols, const uint8_t* pool)
{
    matrix* mat;
    mat = (matrix*) malloc (sizeof (matrix));
    mat->nrows = nrows; 
    mat->ncols = ncols;
    mat->colsize = (ncols + 63)/64;
    mat->elem = (uint64_t**)malloc(nrows * sizeof(uint64_t*));
    for (uint32_t i = 0; i < mat->nrows; i++){
        mat->elem[i] = (uint64_t*)(pool + i * mat->colsize * 8);
    }
    mat->made_with_import = 1;
    return mat;
}

void init_zero(matrix *self){
    for (uint32_t i = 0; i < self->nrows; i++)
    {
        memset(self->elem[i], 0, self->colsize * 8);
    }
}

void delete_matrix(matrix* self)
{   
    if(! self->made_with_import){
        for (uint32_t i = 0; i < self->nrows; i++) {
            free(self->elem[i]);
        }
    }
    free(self->elem);
    free(self);
}

void copy_matrix(matrix* self, matrix* src){
    for (uint32_t i = 0; i < src->nrows; i++)
    {
        memcpy(self->elem[i], src->elem[i], src->colsize * 8);
    }
}

void export_matrix(matrix* self, uint8_t* dest){
    for (uint32_t i = 0; i < self->nrows; i++){
        memcpy(dest + i * self->colsize * 8, self->elem[i], self->colsize * 8);
    }
}

void import_matrix(matrix* self, const uint8_t* src){
    for (uint32_t i = 0; i < self->nrows; i++){
        memcpy(self->elem[i], src + i * self->colsize * 8, self->colsize * 8);
    }
}

void row_addition_internal(matrix* self, const uint32_t r1, const uint32_t r2){
    for (uint32_t j = 0; j < self->colsize; j++) {
        self->elem[r1][j] ^= self->elem[r2][j];
    }
}

// make a matrix into rref form, inplace
void rref(matrix* self)
{
    // Assume column is longer than row
    uint32_t succ_row_idx = 0;
    uint32_t col_idx, row_idx = 0;
    for (col_idx = 0; col_idx < (self->ncols); ++col_idx) {
        
        // finding first row s.t. i th elem of the row is 1
        for(; row_idx < self->nrows; ++row_idx)
            if(get_element(self, row_idx, col_idx) == 1) 
                break;
        // When reaches the last row, increase column index and search again
        if (row_idx == self->nrows){ 
            row_idx = succ_row_idx;
            continue;
        }
        // if row_idx is not succ_row_idx, 
        // interchange between:
        // <succ_row_idx> th row <-> <row_idx> th row
        if(row_idx != succ_row_idx){
            row_interchange(self, succ_row_idx, row_idx);
        }
                
        // By adding <succ_row_idx> th row in the other nrows 
        // s.t. self(i, <succ_row_idx>) == 1,
        // making previous columns as element row.
        for(uint32_t i = 0; i < self->nrows; ++i){
            if(i == succ_row_idx) continue;

            if(get_element(self, i, col_idx) == 1){
                row_addition_internal(self, i, succ_row_idx);
            }
        }
        row_idx = ++succ_row_idx;
    }
}

// Input should be in rref form.
void get_pivot(matrix* self, uint16_t* lead, uint16_t* lead_diff){
    uint16_t row=0, col=0;
    uint16_t lead_idx=0, diff_idx=0;

    while((col < self->ncols) 
            && (row < self->nrows) 
            && (lead_idx < self->nrows) 
            && (diff_idx < (self->ncols - self->nrows))){

        if(get_element(self, row, col) == (uint8_t)1){
            lead[lead_idx++] = col++;
            row++;
        }
        else{
            lead_diff[diff_idx++] = col++;
        }
    }

    while(col < self->ncols){
        if(lead_idx < self->nrows) {
            lead[lead_idx++] = col++;
        }
        else{
            lead_diff[diff_idx++] = col++;
        }
    }
}

// assume vector is transposed
// self is also transposed
uint64_t xor_bits(uint64_t value) {
    value ^= value >> 32;
    value ^= value >> 16;
    value ^= value >> 8;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return value & 1ULL;
}

uint64_t xor256(__m256i *value) {
    // Split the 256-bit value into 64-bit parts
    uint64_t part0 = _mm256_extract_epi64(*value, 0);
    uint64_t part1 = _mm256_extract_epi64(*value, 1);
    uint64_t part2 = _mm256_extract_epi64(*value, 2);
    uint64_t part3 = _mm256_extract_epi64(*value, 3);
    // XOR all 64-bit parts together
    return part0 ^ part1 ^ part2 ^ part3;
}

void vec_mat_prod_avx256(matrix* self, matrix* mat, matrix* vec){
    for(uint32_t i = 0; i < mat->nrows; i++) {
        uint32_t j;
        uint64_t *m = mat->elem[i];
        uint64_t *v = vec->elem[0];
        __m256i *m256 = (__m256i*)m;
        __m256i *v256 = (__m256i*)v;
        
        __m256i block_sum_avx = _mm256_setzero_si256();
        for (j = 0; j < self->colsize; j+=4){
            __m256i s_val = _mm256_loadu_si256(&m256[j/4]);
            __m256i v_val = _mm256_loadu_si256(&v256[j/4]);
            __m256i elem_mul = _mm256_and_si256(s_val, v_val);
            block_sum_avx = _mm256_xor_si256(block_sum_avx, elem_mul);
        }

        uint64_t block_sum_64 = xor256(&block_sum_avx);

        for (; j < mat->colsize; j++) {
            block_sum_64 ^= mat->elem[i][j] & vec->elem[0][j];
        }

        // Now we need to horizontally XOR all the 64-bit integers
        uint64_t final_result = xor_bits(block_sum_64);
        set_element(self, 0, i, final_result);
    }
}

void vec_mat_prod_64(matrix* self, matrix* mat, matrix* vec){
    for(uint32_t i = 0; i < mat->nrows; i++) {
        uint64_t block_sum = 0ULL;
        for (uint32_t j = 0; j < mat->colsize; j++) {
            block_sum ^= mat->elem[i][j] & vec->elem[0][j];
        }
        block_sum = xor_bits(block_sum);
        set_element(self, 0, i, block_sum);
    }
}

void vec_mat_prod(matrix* self, matrix* mat, matrix* vec){
    if (mat->colsize < 4)
    {
        vec_mat_prod_64(self, mat, vec);
        return;
    }
    vec_mat_prod_avx256(self, mat, vec);
}

void vec_vec_add(matrix* self, matrix* vec){
    uint32_t j;
    uint64_t *s = self->elem[0];
    uint64_t *v = vec->elem[0];
    __m256i *s256 = (__m256i*)s;
    __m256i *v256 = (__m256i*)v;

    for (j = 0; j < self->colsize; j+=4){
        __m256i s_val = _mm256_loadu_si256(&s256[j/4]);
        __m256i v_val = _mm256_loadu_si256(&v256[j/4]);
        __m256i result = _mm256_xor_si256(s_val, v_val);
        _mm256_storeu_si256(&s256[j/4], result);
    }

    for (; j < self->colsize; ++j) {
        s[j] ^= v[j];
    }
}

uint8_t vec_vec_is_equal(matrix* self, matrix *vec){
     for (uint32_t j = 0; j < self->colsize - 1; j++) {
        if (self->elem[0][j] != vec->elem[0][j]){
            return 0;
        }
    }
    for (uint32_t j = 8*(self->colsize - 1); j < self->ncols; j++)
    {
        if (get_element(self, 0, j) != get_element(self, 0, j)){
            return 0;
        }
    }
    
    return 1;   
}

void dual(matrix* self, matrix* dual_sys){
    uint16_t lead[self->nrows];
    uint16_t lead_diff[self->ncols - self->nrows];    

    init_zero(dual_sys);

    rref(self);
    get_pivot(self, lead, lead_diff);

    // Fill not-identity part (P')
    for (uint32_t row = 0; row < dual_sys->nrows; row++) {
        for (uint32_t col = 0; col < self->nrows; col++) {
            set_element(dual_sys, row, lead[col], get_element(self, col, lead_diff[row]));
        }
    }
    
    for (uint32_t row = 0; row < dual_sys->nrows; row++) {
        set_element(dual_sys, row, lead_diff[row], 1);    
    }
}

void row_interchange(matrix* self, uint32_t row1, uint32_t row2) {
    uint64_t* temp = self->elem[row1];
    self->elem[row1] = self->elem[row2];
    self->elem[row2] = temp;
}

void partial_replace(matrix* self, const uint32_t r1, const uint32_t r2,
        const uint32_t c1, const uint32_t c2, 
        matrix* src, const int r3, const int c3){
    
    if((c1 % 64 == 0) && (c2 % 64 == 0) && (c3 % 64 == 0)){
        for(uint32_t i = 0; i < r2 - r1; i++) {
            for(uint32_t j = 0; j < c2 - c1; j+=64) {
                self->elem[r1+i][(c1+j)/64] = src->elem[r3+i][(c3+j)/64];
            }
        }
        return;
    }

    for(uint32_t i = 0; i < r2 - r1; i++) {
        for(uint32_t j = 0; j < c2 - c1; j++) {
            set_element(self, r1 + i, c1+j, get_element(src, r3 + i, c3 + j));
        }
    }
}

void codeword(matrix* self, uint8_t* seed, matrix* dest){
    for (uint32_t i = 0; i < self->nrows; i++) {
        uint32_t byte_index = i >> 3; // byte_index = bit_index / 8;
        uint32_t bit_offset = i & 7; // bit_offset = bit_index % 8;
        uint8_t rand_bit = (seed[byte_index] >> bit_offset) & 1;
        if (rand_bit == 1) {
            for (uint32_t j = 0; j < self->colsize; j++)
            {
                dest->elem[0][j] ^= self->elem[i][j];
            }
        }
    }
}

uint8_t is_zero(matrix* self){
    for (uint32_t i = 0; i < self->nrows; i++) {
        for (size_t j = 0; j < self->ncols; j++) {
            if(get_element(self, i, j) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

void col_permute(matrix* self, const int r1, const int r2
	, const int c1, const int c2, uint16_t* Q) {	
	matrix* copy = new_matrix(r2 - r1, c2 - c1);
	for (uint32_t r = 0; r < r2 - r1; r++) {
		for (uint32_t c = 0; c < c2 - c1; c++) {
			uint8_t bit = get_element(self, r1 + r, c1 + c);
			set_element(copy, r, c, bit);
		}
	}
	
	for(uint32_t c = 0; c < c2 - c1; c++) {
		for(uint32_t r = 0; r < r2 - r1; r++) {
            uint8_t bit =  get_element(copy, r, Q[c]);
			set_element(self, r1 + r, c1 + c, bit);
		}
	}

	delete_matrix(copy);
}
