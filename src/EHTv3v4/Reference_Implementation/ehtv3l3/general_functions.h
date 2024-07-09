#ifndef general_functions_h
#define general_functions_h

#include <stdbool.h>

int NIST_rng(int x);
unsigned char* allocate_unsigned_char_vector_memory(int n);
unsigned char** allocate_unsigned_char_matrix_memory(int m, int n);
unsigned short** allocate_unsigned_short_matrix_memory(int m, int n);
void free_matrix(int m, unsigned char** A);
void free_short_matrix(int m, unsigned short** A);
void zero_vector(int n, unsigned char* A);
void zero_matrix(int m, int n, unsigned char** A);
int signum(int x);
int mod_q(int x);
int a_mod_q(int x);
int s_mod_q(int x);
int p_mod_q(int x);
void decimal_to_binary(int decimal, bool *bin_vec, int num_bits);
void matrix_multiply(int m, int l, int n, unsigned char** A, unsigned char **B, unsigned char **C);
void hash_of_message(const unsigned char* m, unsigned long long mlen, unsigned char** h);

#endif
