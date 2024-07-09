#ifndef common_functions_h
#define common_functions_h

#include <stdbool.h>

unsigned short* allocate_vector_memory(int n);
unsigned short*** allocate_gr_matrix_memory(int m, int n);
void free_gr_matrix(int m, int n, unsigned short*** A);
void zero_vector(int n, unsigned short* A);
void zero_gr_matrix(int m, int n, unsigned short*** A);
unsigned short NIST_rng(int x);
unsigned short mod_q(int x);
unsigned short a_mod_q(int x);
unsigned short s_mod_q(int x);
unsigned short p_mod_q(int x);
int multiplicative_inverse(int a, int q);
int signum(int x);
unsigned short composition_index(int i1, int i2);
bool solve_agumented_matrix(int n, unsigned short AM[n][n+1]);
bool inverse_in_Gq(unsigned short* vec, unsigned short* inverse_vec);
void product_in_Gq(unsigned short* vec1, unsigned short* vec2, unsigned short* result, bool overwrite);
bool is_gr_empty(unsigned short* A);
void grmatrix_multiply(int m, int l, int n, unsigned short*** A, unsigned short*** B, unsigned short*** C);
void hash_of_message(const unsigned char* m, unsigned long long mlen, unsigned short*** h);

#endif
