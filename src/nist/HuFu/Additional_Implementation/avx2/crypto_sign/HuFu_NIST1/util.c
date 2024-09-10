#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <immintrin.h>
#include <string.h>
#include <time.h>

#include "util.h"
#include "params.h"
#include "random/random.h"
#include "sha3/shake.h"
// generate matrix R with dist. CBD(1)
void cbd(int16_t *R, uint8_t *seed_R)
{
	prng rng;
	get_seed(32, &rng, seed_R);
	int n = (PARAM_M + PARAM_N) * PARAM_M;
	int32_t tb[4] = {-1, 0, 0, 1};
	uint8_t buf[n / 4];
	// randombytes(buf, n / 4);
	prng_get_bytes(&rng, buf, n / 4);
	for (int i = 0; i < n / 4; ++i)
	{
		R[4 * i + 0] = tb[buf[i] & 0x3];
		R[4 * i + 1] = tb[(buf[i] >> 2) & 0x3];
		R[4 * i + 2] = tb[(buf[i] >> 4) & 0x3];
		R[4 * i + 3] = tb[buf[i] >> 6];
	}
}

// cholesky decomposition
int cholesky(double *L, double *A, int dim)
{
	for (int k = 0; k < dim; k++)
	{
		double sum = 0;
		for (int i = 0; i < k; i++)
			sum += tri_mat_element(L, k, i) * tri_mat_element(L, k, i);
		sum = tri_mat_element(A, k, k) - sum;
		if (sum < 0)
			return 0;
		tri_mat_element(L, k, k) = sqrt(sum);
		for (int i = k + 1; i < dim; i++)
		{
			sum = 0;
			for (int j = 0; j < k; j++)
				sum += tri_mat_element(L, i, j) * tri_mat_element(L, k, j);
			tri_mat_element(L, i, k) = (tri_mat_element(A, i, k) - sum) / tri_mat_element(L, k, k);
		}
	}
	return 1;
}

int16_t modq(int32_t a)
{
	a = a % PARAM_Q;
	if (a < -PARAM_Q / 2)
		a += PARAM_Q;
	if (a >= PARAM_Q / 2)
		a -= PARAM_Q;
	return a;
}

//--------------------------optimized with SIMD----------------------------------------------------
// lower triangle mat A is a square mat
void tri_mat_mul(double *C, const double *A, const double *B, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
		{
			mat_element(C, col, i, j) = 0;
			for (int k = 0; k <= i; ++k)
				mat_element(C, col, i, j) += tri_mat_element(A, i, k) * mat_element(B, col, k, j);
		}
}

void tri_mat_mul_avx2_con1(double *C, const double *A, const double *B, int row)
{ 
       __m256d sum, a, b;      
       for(int i = 0; i < row; i++){
             C[i] = 0;
             sum = _mm256_setzero_pd();
             for(int j = 0; j < i/4; j++){ 
                  a = _mm256_loadu_pd(A + j*4);
                  b = _mm256_loadu_pd(B + j*4);      
                  a = _mm256_mul_pd(a, b);
                  sum = _mm256_add_pd(sum, a);
             }
             double st[4];
             _mm256_storeu_pd(st, sum);
             C[i] = st[0] + st[1] + st[2] + st[3];
             int res = i % 4;
             for(int k = i - res; k < i+1; k++){
                  C[i] = C[i] + A[k] * B[k];
             }
             A = A + (i + 1);
       }       
}

void mat_mul(int16_t *C, const int16_t *A, const int16_t *B, int l1, int l2, int l3)
{
	for (int i = 0; i < l1; ++i)
		for (int j = 0; j < l3; ++j)
		{
			mat_element(C, l3, i, j) = 0;
			for (int k = 0; k < l2; ++k)
				mat_element(C, l3, i, j) += mat_element(A, l2, i, k) * mat_element(B, l3, k, j);
		}
}
void real_mat_mul_avx2_con1_int16(double *C, const int16_t *A, const double *B, int l1, int l2)
{
	__m256d a1, a2, a3, a4, b, sum1, sum2, sum3, sum4;
	for (int i = 0; i < l1 / 4; i++)
	{
		sum1 = _mm256_setzero_pd();
		sum2 = _mm256_setzero_pd();
		sum3 = _mm256_setzero_pd();
		sum4 = _mm256_setzero_pd();
		for (int j = 0; j < l2 / 4; j++)
		{
			b = _mm256_loadu_pd(B + j * 4);
			a1 = _mm256_set_pd(*(A + (i * 4 + 0) * l2 + j * 4+3), *(A + (i * 4 + 0) * l2 + j * 4 + 2), *(A + (i * 4 + 0) * l2 + j * 4 + 1), *(A + (i * 4 + 0) * l2 + j * 4 + 0));
			a2 = _mm256_set_pd(*(A + (i * 4 + 1) * l2 + j * 4+3), *(A + (i * 4 + 1) * l2 + j * 4 + 2), *(A + (i * 4 + 1) * l2 + j * 4 + 1), *(A + (i * 4 + 1) * l2 + j * 4 + 0));
			a3 = _mm256_set_pd(*(A + (i * 4 + 2) * l2 + j * 4+3), *(A + (i * 4 + 2) * l2 + j * 4 + 2), *(A + (i * 4 + 2) * l2 + j * 4 + 1), *(A + (i * 4 + 2) * l2 + j * 4 + 0));
			a4 = _mm256_set_pd(*(A + (i * 4 + 3) * l2 + j * 4+3), *(A + (i * 4 + 3) * l2 + j * 4 + 2), *(A + (i * 4 + 3) * l2 + j * 4 + 1), *(A + (i * 4 + 3) * l2 + j * 4 + 0));
			a1 = _mm256_mul_pd(a1, b);
			a2 = _mm256_mul_pd(a2, b);
			a3 = _mm256_mul_pd(a3, b);
			a4 = _mm256_mul_pd(a4, b);
			sum1 = _mm256_add_pd(sum1, a1);
			sum2 = _mm256_add_pd(sum2, a2);
			sum3 = _mm256_add_pd(sum3, a3);
			sum4 = _mm256_add_pd(sum4, a4);
		}
		double st1[4], st2[4], st3[4], st4[4];
		_mm256_storeu_pd(st1, sum1);
		_mm256_storeu_pd(st2, sum2);
		_mm256_storeu_pd(st3, sum3);
		_mm256_storeu_pd(st4, sum4);
		C[i * 4 + 0] = st1[0] + st1[1] + st1[2] + st1[3];
		C[i * 4 + 1] = st2[0] + st2[1] + st2[2] + st2[3];
		C[i * 4 + 2] = st3[0] + st3[1] + st3[2] + st3[3];
		C[i * 4 + 3] = st4[0] + st4[1] + st4[2] + st4[3];
	}
}
void mat_mul_avx2_con1(int16_t *C, const int16_t *A, const int16_t *B, int l1, int l2)
{

	__m256i a1, a2, a3, a4, b, sum1, sum2, sum3, sum4;
	for (int i = 0; i < l1 / 4; i++)
	{
		sum1 = _mm256_setzero_si256();
		sum2 = _mm256_setzero_si256();
		sum3 = _mm256_setzero_si256();
		sum4 = _mm256_setzero_si256();
		for (int j = 0; j < l2 / 16; j++)
		{
			b = _mm256_loadu_si256((__m256i *)(B + j * 16));
			a1 = _mm256_loadu_si256((__m256i *)(A + (i * 4 + 0) * l2 + j * 16));
			a2 = _mm256_loadu_si256((__m256i *)(A + (i * 4 + 1) * l2 + j * 16));
			a3 = _mm256_loadu_si256((__m256i *)(A + (i * 4 + 2) * l2 + j * 16));
			a4 = _mm256_loadu_si256((__m256i *)(A + (i * 4 + 3) * l2 + j * 16));
			a1 = _mm256_mullo_epi16(a1, b);
			a2 = _mm256_mullo_epi16(a2, b);
			a3 = _mm256_mullo_epi16(a3, b);
			a4 = _mm256_mullo_epi16(a4, b);
			sum1 = _mm256_add_epi16(sum1, a1);
			sum2 = _mm256_add_epi16(sum2, a2);
			sum3 = _mm256_add_epi16(sum3, a3);
			sum4 = _mm256_add_epi16(sum4, a4);
		}
		uint16_t st1[16], st2[16], st3[16], st4[16];
		_mm256_storeu_si256((__m256i *)st1, sum1);
		_mm256_storeu_si256((__m256i *)st2, sum2);
		_mm256_storeu_si256((__m256i *)st3, sum3);
		_mm256_storeu_si256((__m256i *)st4, sum4);
		C[i * 4 + 0] = st1[0] + st1[1] + st1[2] + st1[3] + st1[4] + st1[5] + st1[6] + st1[7] + st1[8] + st1[9] + st1[10] + st1[11] + st1[12] + st1[13] + st1[14] + st1[15];
		C[i * 4 + 1] = st2[0] + st2[1] + st2[2] + st2[3] + st2[4] + st2[5] + st2[6] + st2[7] + st2[8] + st2[9] + st2[10] + st2[11] + st2[12] + st2[13] + st2[14] + st2[15];
		C[i * 4 + 2] = st3[0] + st3[1] + st3[2] + st3[3] + st3[4] + st3[5] + st3[6] + st3[7] + st3[8] + st3[9] + st3[10] + st3[11] + st3[12] + st3[13] + st3[14] + st3[15];
		C[i * 4 + 3] = st4[0] + st4[1] + st4[2] + st4[3] + st4[4] + st4[5] + st4[6] + st4[7] + st4[8] + st4[9] + st4[10] + st4[11] + st4[12] + st4[13] + st4[14] + st4[15];
	}
}

void mat_mul_avx2(int16_t *C, const int16_t *A, const int16_t *B, int l1, int l2, int l3)
{

	int16_t *temp = malloc(l2 * sizeof(int16_t));
	int16_t *ct = malloc(l1 * sizeof(int16_t));

	for (int i = 0; i < l3; i++)
	{
		for (int j = 0; j < l2; j++)
		{
			temp[j] = B[j * l3 + i];
		}
		mat_mul_avx2_con1(ct, A, temp, l1, l2);
		for (int j = 0; j < l1; j++)
		{
			C[j * l3 + i] = ct[j];
		}
	}
	free(temp);
	free(ct);
}

void mat_add(int16_t *C, const int16_t *A, const int16_t *B, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) + mat_element(B, col, i, j);
}
void mat_add_avx2(int16_t *C,const int16_t *A,const int16_t *B,int row, int col){
	__m256i vecA, vecB, vecC;
	for (int i = 0; i < row * col / 16; i++)
	{
		vecA = _mm256_loadu_si256((__m256i*)(A+16*i));
		vecB = _mm256_loadu_si256((__m256i*)(B+16*i));
		vecC = _mm256_add_epi16(vecA, vecB);
		_mm256_storeu_si256((__m256i*)(C+16*i),vecC);
		
	}
}

void mat_sub_avx2(int16_t *C, const int16_t *A, const int16_t *B, int row, int col){
	__m256i vecA, vecB, vecC;
	for (int i = 0; i < row * col / 16; i++)
	{
		vecA = _mm256_loadu_si256((__m256i*)(A+16*i));
		vecB = _mm256_loadu_si256((__m256i*)(B+16*i));
		vecC = _mm256_sub_epi16(vecA, vecB);
		_mm256_storeu_si256((__m256i*)(C+16*i),vecC);
		
	}
}
void real_mat_mul(double *C, const double *A, const double *B, int l1, int l2, int l3)
{
	for (int i = 0; i < l1; ++i)
		for (int j = 0; j < l3; ++j)
		{
			mat_element(C, l3, i, j) = 0;
			for (int k = 0; k < l2; ++k)
				mat_element(C, l3, i, j) += mat_element(A, l2, i, k) * mat_element(B, l3, k, j);
		}
}

void real_mat_mul_avx2_con1(double *C, const double *A, const double *B, int l1, int l2)
{
	__m256d a1, a2, a3, a4, b, sum1, sum2, sum3, sum4;
	for (int i = 0; i < l1 / 4; i++)
	{
		sum1 = _mm256_setzero_pd();
		sum2 = _mm256_setzero_pd();
		sum3 = _mm256_setzero_pd();
		sum4 = _mm256_setzero_pd();
		for (int j = 0; j < l2 / 4; j++)
		{
			b = _mm256_loadu_pd(B + j * 4);
			a1 = _mm256_loadu_pd(A + (i * 4 + 0) * l2 + j * 4);
			a2 = _mm256_loadu_pd(A + (i * 4 + 1) * l2 + j * 4);
			a3 = _mm256_loadu_pd(A + (i * 4 + 2) * l2 + j * 4);
			a4 = _mm256_loadu_pd(A + (i * 4 + 3) * l2 + j * 4);
			a1 = _mm256_mul_pd(a1, b);
			a2 = _mm256_mul_pd(a2, b);
			a3 = _mm256_mul_pd(a3, b);
			a4 = _mm256_mul_pd(a4, b);
			sum1 = _mm256_add_pd(sum1, a1);
			sum2 = _mm256_add_pd(sum2, a2);
			sum3 = _mm256_add_pd(sum3, a3);
			sum4 = _mm256_add_pd(sum4, a4);
		}
		double st1[4], st2[4], st3[4], st4[4];
		_mm256_storeu_pd(st1, sum1);
		_mm256_storeu_pd(st2, sum2);
		_mm256_storeu_pd(st3, sum3);
		_mm256_storeu_pd(st4, sum4);
		C[i * 4 + 0] = st1[0] + st1[1] + st1[2] + st1[3];
		C[i * 4 + 1] = st2[0] + st2[1] + st2[2] + st2[3];
		C[i * 4 + 2] = st3[0] + st3[1] + st3[2] + st3[3];
		C[i * 4 + 3] = st4[0] + st4[1] + st4[2] + st4[3];
	}
}

void real_mat_add(double *C, const double *A, const double *B, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) + mat_element(B, col, i, j);
}

void mat_sub(int16_t *C, const int16_t *A, const int16_t *B, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) - mat_element(B, col, i, j);
}

void mat_neg(int16_t *A, int row, int col)
{
	for (int i = 0; i < row; ++i)
		for (int j = 0; j < col; ++j)
			mat_element(A, col, i, j) = -mat_element(A, col, i, j);
}


//A*Bt, A_col = B_col, C is A_row*B_row;  
void real_mat_mul_avx2_AmulBt(double *C, double *A, double *B, int A_row, int A_col, int B_row){
        
        for(int i = 0; i < A_row; i++){
               real_mat_mul_avx2_con1(C, B, A, B_row, A_col);
               A = A + A_col;
               C = C + B_row;
        }

}

void mat_mul_avx2_int16_AmulBt(int16_t *C, int16_t *A, int16_t *B, int A_row, int A_col, int B_row){
        
        for(int i = 0; i < A_row; i++){
               mat_mul_avx2_con1(C, B, A, B_row, A_col);
               A = A + A_col;
               C = C + B_row;
        }

}
//A*tri(Bt), B is A_col * A_col, C is A_row * A_col
void tri_mat_mul_avx2_AmultriBt(double *C, double *A, double *B, int A_row, int A_col){
        
        for(int i = 0; i < A_row; i++){
               tri_mat_mul_avx2_con1(C, B, A, A_col);
               A = A + A_col;
               C = C + A_col;
        }

}

void vector_mul_int16(int16_t *C, int16_t *A, int16_t *B, int len){
	__m256i a, b, sum;
	sum = _mm256_setzero_si256();

	for(int i=0; i<len/16; i++){
			a = _mm256_loadu_si256((__m256i *)(A + i * 16));    
			b = _mm256_loadu_si256((__m256i *)(B + i * 16)); 
			a = _mm256_mullo_epi16(a, b);
			sum = _mm256_add_epi16(sum, a);
	}
	uint16_t st[16];
	_mm256_storeu_si256((__m256i *)st, sum);
	C[0] = st[0] + st[1] + st[2] + st[3] + st[4] + st[5] + st[6] + st[7] + st[8] + st[9] + st[10] + st[11] + st[12] + st[13] + st[14] + st[15];
}


//A * At, results the low tri_mat
void tri_mat_mul_int16_AAt(int16_t *C, int16_t *A, int row, int col){
       for(int i = 0; i < row; i++){
              for(int j = 0; j <= i; j++){
                     vector_mul_int16(C, A + i * col, A + j * col, col);
                     C = C + 1;
              }
       }
}

void vector_mul_double(double *C, double *A, double *B, int len){
       __m256d a, b, sum;
       sum = _mm256_setzero_pd();

       for(int i=0; i<len/4; i++){
              a = _mm256_loadu_pd(A + i * 4);    
              b = _mm256_loadu_pd(B + i * 4); 
              a = _mm256_mul_pd(a, b);
              sum = _mm256_add_pd(sum, a);
       }
       double st[4];
       _mm256_storeu_pd(st, sum);
       C[0] = st[0] + st[1] + st[2] + st[3];
}

void tri_mat_mul_double_AAt(double *C, double *A, int row, int col){
       for(int i = 0; i < row; i++){
              for(int j = 0; j <= i; j++){
                     vector_mul_double(C, A + i * col, A + j * col, col);
                     C = C + 1;
              }
       }
}

// generate L_22, L32, L_33 by using R
int generate_auxiliary_mat(double *L_22, double *L_32, double *L_33, int16_t *R)
{
	double sigma_prime_square = (PARAM_SIGMA1 * PARAM_SIGMA1 - PARAM_R * PARAM_R) / (PARAM_q * PARAM_R * PARAM_q * PARAM_R);
	int16_t *E = R, *S = R + PARAM_M * PARAM_M;
	double sigma_pre = sigma_prime_square / (1-sigma_prime_square);
	// compute L_22
	double *SSt = malloc(PARAM_N * (1 + PARAM_N) / 2 * sizeof(double));
	int16_t *SSt_int16 = malloc(PARAM_N * (1 + PARAM_N) / 2 * sizeof(int16_t));
	tri_mat_mul_int16_AAt(SSt_int16, S, PARAM_N, PARAM_M);  
	for (int i = 0; i < PARAM_N; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			tri_mat_element(SSt, i, j) = (double)tri_mat_element(SSt_int16, i, j) * sigma_pre;
		}
		tri_mat_element(SSt, i, i) += sigma_prime_square;
	} 
	free(SSt_int16);
	
	int flag = cholesky(L_22, SSt, PARAM_N);

	if (flag == 0)
	{
		free(SSt);
		return 0;
	}
	double *inv_L_22 = malloc(PARAM_N * (1 + PARAM_N) / 2 * sizeof(double));
	tri_mat_inv(inv_L_22, L_22);
	// compute L_32
	double *ESt = malloc(PARAM_M * PARAM_N * sizeof(double));        
	int16_t *ESt_int16 = malloc(PARAM_M * PARAM_N * sizeof(int16_t));
	mat_mul_avx2_int16_AmulBt(ESt_int16, E, S, PARAM_M, PARAM_M, PARAM_N);
	for (int i = 0; i < PARAM_M; i++)
	{
		for (int j = 0; j < PARAM_N; ++j)
		{
			ESt[i*PARAM_N + j] = ((double)ESt_int16[i*PARAM_N + j]) * sigma_pre;
		}
	}
	free(ESt_int16);
	tri_mat_mul_avx2_AmultriBt(L_32, ESt, inv_L_22, PARAM_M, PARAM_N); 
	
	// compute L_33
	double *EEt = malloc(PARAM_M * (1 + PARAM_M) / 2 * sizeof(double));
	int16_t *EEt_int16 = malloc(PARAM_M * (1 + PARAM_M) / 2 * sizeof(int16_t));
	tri_mat_mul_int16_AAt(EEt_int16, E, PARAM_M, PARAM_M);  
	for (int i = 0; i < PARAM_M; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			tri_mat_element(EEt, i, j) = (double)tri_mat_element(EEt_int16, i, j) * sigma_pre;
		}
	} 
	free(EEt_int16);

	double *L_32_tri = malloc(PARAM_M * (1 + PARAM_M) / 2 * sizeof(double));
	tri_mat_mul_double_AAt(L_32_tri, L_32, PARAM_M, PARAM_N);

	for (int i = 0; i < PARAM_M * (1 + PARAM_M) / 2; i++)
	{		
		EEt[i] = EEt[i] - L_32_tri[i];		
	} 
	free(L_32_tri);
	for (int i = 0; i < PARAM_M; i++)
	{
		tri_mat_element(EEt, i, i) += sigma_prime_square;
	}

	flag = cholesky(L_33, EEt, PARAM_M);
	if (flag == 0)
	{
		free(SSt);
		free(inv_L_22);
		free(ESt);
		free(EEt);
		return 0;
	}

	// scale the matrices by q * r
	for (int i = 0; i < PARAM_M; i++)
	{
		for (int j = 0; j <= i; ++j)
			tri_mat_element(L_33, i, j) *= PARAM_q * PARAM_R;
		for (int j = 0; j < PARAM_N; ++j)
			mat_element(L_32, PARAM_N, i, j) *= PARAM_q * PARAM_R;
	}
	for (int i = 0; i < PARAM_N; i++)
		for (int j = 0; j <= i; ++j)
			tri_mat_element(L_22, i, j) *= PARAM_q * PARAM_R;

	free(SSt);
	free(inv_L_22);
	free(ESt);
	free(EEt);


	return 1;
}

// compute matrix inversion for low-triangular matrix
void tri_mat_inv(double *OUT, const double *IN)
{
	for (int k = 0; k < PARAM_N; k++)
	{
		tri_mat_element(OUT, k, k) = 1 / tri_mat_element(IN, k, k);
	}
	for (int i = 1; i < PARAM_N; i++)
	{
		for (int j = 0; j < i; j++)
		{
			double sum = 0.0;
			for (int k = j; k < i; k++)
			{
				sum += tri_mat_element(IN, i, k) * tri_mat_element(OUT, k, j);
			}
			tri_mat_element(OUT, i, j) = -sum * tri_mat_element(OUT, i, i);
		}
	}
}

double norm_square(const int16_t *v, int n)
{
	double norm_square = 0;
	for (int i = 0; i < n; i++)
		norm_square += v[i] * v[i];
	return norm_square;
}

// pack and unpack mat R, in which each element has two bits
void pack_mat_r(uint8_t *buf, const int16_t *data)
{
	for (size_t i = 0; i < PARAM_R_BYTES; i++)
	{
		buf[i] = ((data[0] & 0x03) << 6) | ((data[1] & 0x03) << 4) | ((data[2] & 0x03) << 2) | (data[3] & 0x03);
		data += 4;
	}
}

void unpack_mat_r(int16_t *data, const uint8_t *buf)
{
	for (size_t i = 0; i < PARAM_R_BYTES; i++)
	{
		data[0] = (buf[i] & 0xc0) >> 6;
		data[1] = (buf[i] & 0x30) >> 4;
		data[2] = (buf[i] & 0x0c) >> 2;
		data[3] = (buf[i] & 0x03);
		data += 4;
	}
}

void unpack_mat_r_new(int16_t *data, const uint8_t *buf, int buf_len)
{
	for (int i = 0; i < buf_len; i++)
	{
		data[0] = (buf[i] & 0xc0) >> 6;
		data[1] = (buf[i] & 0x30) >> 4;
		data[2] = (buf[i] & 0x0c) >> 2;
		data[3] = (buf[i] & 0x03);
		data += 4;
	}
}

void unpack_mat_r_avx2(int16_t *data, const uint8_t *buf, int buf_len)
{
	for (int i = 0; i < buf_len / 32; i++)
	{
		__m256i a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3, b4, b5, b6, b7;
		a0 = _mm256_loadu_si256((__m256i *)(buf));
		a1 = _mm256_slli_epi16(a0, 2);
		a2 = _mm256_slli_epi16(a0, 4);
		a3 = _mm256_slli_epi16(a0, 6);
		a4 = _mm256_slli_epi16(a0, 8);
		a5 = _mm256_slli_epi16(a0, 10);
		a6 = _mm256_slli_epi16(a0, 12);
		a7 = _mm256_slli_epi16(a0, 14);

		a0 = _mm256_srai_epi16(a0, 14);
		a1 = _mm256_srai_epi16(a1, 14);
		a2 = _mm256_srai_epi16(a2, 14);
		a3 = _mm256_srai_epi16(a3, 14);
		a4 = _mm256_srai_epi16(a4, 14);
		a5 = _mm256_srai_epi16(a5, 14);
		a6 = _mm256_srai_epi16(a6, 14);
		a7 = _mm256_srai_epi16(a7, 14);

		b0 = _mm256_unpacklo_epi16(a0, a1);
		b1 = _mm256_unpackhi_epi16(a0, a1);
		b2 = _mm256_unpacklo_epi16(a2, a3);
		b3 = _mm256_unpackhi_epi16(a2, a3);
		b4 = _mm256_unpacklo_epi16(a4, a5);
		b5 = _mm256_unpackhi_epi16(a4, a5);
		b6 = _mm256_unpacklo_epi16(a6, a7);
		b7 = _mm256_unpackhi_epi16(a6, a7);

		a0 = _mm256_unpacklo_epi32(b0, b2);
		a1 = _mm256_unpackhi_epi32(b0, b2);
		a2 = _mm256_unpacklo_epi32(b1, b3);
		a3 = _mm256_unpackhi_epi32(b1, b3);
		a4 = _mm256_unpacklo_epi32(b4, b6);
		a5 = _mm256_unpackhi_epi32(b4, b6);
		a6 = _mm256_unpacklo_epi32(b5, b7);
		a7 = _mm256_unpackhi_epi32(b5, b7);

		b0 = _mm256_unpacklo_epi64(a4, a0);
		b1 = _mm256_unpackhi_epi64(a4, a0);
		b2 = _mm256_unpacklo_epi64(a5, a1);
		b3 = _mm256_unpackhi_epi64(a5, a1);
		b4 = _mm256_unpacklo_epi64(a6, a2);
		b5 = _mm256_unpackhi_epi64(a6, a2);
		b6 = _mm256_unpacklo_epi64(a7, a3);
		b7 = _mm256_unpackhi_epi64(a7, a3);

		a0 = _mm256_permute2f128_si256(b0, b1, 0x20);
		a1 = _mm256_permute2f128_si256(b0, b1, 0x31);
		a2 = _mm256_permute2f128_si256(b2, b3, 0x20);
		a3 = _mm256_permute2f128_si256(b2, b3, 0x31);
		a4 = _mm256_permute2f128_si256(b4, b5, 0x20);
		a5 = _mm256_permute2f128_si256(b4, b5, 0x31);
		a6 = _mm256_permute2f128_si256(b6, b7, 0x20);
		a7 = _mm256_permute2f128_si256(b6, b7, 0x31);

		_mm256_storeu_si256((__m256i *)data, a0);
		_mm256_storeu_si256((__m256i *)(data + 16), a2);
		_mm256_storeu_si256((__m256i *)(data + 32), a4);
		_mm256_storeu_si256((__m256i *)(data + 48), a6);
		_mm256_storeu_si256((__m256i *)(data + 64), a1);
		_mm256_storeu_si256((__m256i *)(data + 80), a3);
		_mm256_storeu_si256((__m256i *)(data + 96), a5);
		_mm256_storeu_si256((__m256i *)(data + 112), a7);
		buf = buf + 32;
		data = data + 128;
	}
}
