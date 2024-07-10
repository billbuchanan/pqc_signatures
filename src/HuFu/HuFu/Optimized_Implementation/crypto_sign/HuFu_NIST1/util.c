#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <immintrin.h>

#include "util.h"
#include "params.h"
#include "random/random.h"
#include "sha3/shake.h"
// generate matrix R with dist. CBD(1)
void cbd(int16_t * R, uint8_t * seed_R)
{
	prng rng;
	get_seed(32, &rng,seed_R);
	int n = (PARAM_M + PARAM_N) * PARAM_M;
	int32_t tb[4] = {-1, 0, 0, 1};
	uint8_t buf[n / 4];
	// randombytes(buf, n / 4);
	prng_get_bytes(&rng, buf, n / 4);
	for(int i = 0; i < n / 4; ++i){
		R[4 * i + 0] = tb[buf[i] & 0x3];
		R[4 * i + 1] = tb[(buf[i] >> 2) & 0x3];
		R[4 * i + 2] = tb[(buf[i] >> 4) & 0x3];
		R[4 * i + 3] = tb[buf[i] >> 6];
	}
}

// cholesky decomposition
int cholesky(double * L, double * A, int dim)
{  
    for(int k = 0; k < dim; k++)
	{  
        double sum = 0;  
        for(int i = 0; i < k; i++)  
            sum += tri_mat_element(L, k, i) * tri_mat_element(L, k, i);
        sum = tri_mat_element(A, k, k) - sum;  
        if(sum < 0) 
			return 0;
        tri_mat_element(L, k, k) = sqrt(sum);  
        for(int i = k + 1; i < dim; i++)
		{  
            sum = 0;  
            for(int j = 0; j < k; j++)  
                sum += tri_mat_element(L, i, j) * tri_mat_element(L, k, j);  
            tri_mat_element(L, i, k) = (tri_mat_element(A, i, k) - sum) / tri_mat_element(L, k, k);  
        }  
    }  
    return 1;
} 

int16_t modq(int32_t a){
	a = a % PARAM_Q;
	if(a < -PARAM_Q / 2)
		a += PARAM_Q;
	if(a >= PARAM_Q / 2)
		a -= PARAM_Q;
	return a;
}

// lower triangle mat A is a square mat
void tri_mat_mul(double * C, const double * A, const double * B, int row, int col){
	for(int i = 0 ; i < row ; ++i)
		for(int j = 0 ; j < col ; ++j){
			mat_element(C, col, i, j) = 0;
			for(int k = 0 ; k <= i; ++k)
				mat_element(C, col, i, j) += tri_mat_element(A, i, k) * mat_element(B, col, k, j);
		}
}

void mat_mul(int16_t * C, const int16_t * A, const int16_t * B, int l1, int l2, int l3){
	for(int i = 0 ; i < l1 ; ++i)
		for(int j = 0 ; j < l3 ; ++j){
			mat_element(C, l3, i, j) = 0;
			for(int k = 0 ; k < l2 ; ++k)
				mat_element(C, l3, i, j) += mat_element(A, l2, i, k) * mat_element(B, l3, k, j);
		}
}

void mat_add(int16_t * C, const int16_t * A, const int16_t * B, int row, int col){
	for(int i = 0 ; i < row ; ++i)
		for(int j = 0 ; j < col ; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) + mat_element(B, col, i, j);
}

void real_mat_mul_int16(double * C, const int16_t * A, const double * B, int l1, int l2, int l3){
	for(int i = 0 ; i < l1 ; ++i)
		for(int j = 0 ; j < l3 ; ++j){
			mat_element(C, l3, i, j) = 0;
			for(int k = 0 ; k < l2 ; ++k)
				mat_element(C, l3, i, j) += mat_element(A, l2, i, k) * mat_element(B, l3, k, j);
		}
}
void real_mat_mul(double * C, const double * A, const double * B, int l1, int l2, int l3){
	for(int i = 0 ; i < l1 ; ++i)
		for(int j = 0 ; j < l3 ; ++j){
			mat_element(C, l3, i, j) = 0;
			for(int k = 0 ; k < l2 ; ++k)
				mat_element(C, l3, i, j) += mat_element(A, l2, i, k) * mat_element(B, l3, k, j);
		}
}

void real_mat_add(double * C, const double * A, const double * B, int row, int col){
	for(int i = 0 ; i < row ; ++i)
		for(int j = 0 ; j < col ; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) + mat_element(B, col, i, j);
}

void mat_sub(int16_t * C, const int16_t * A, const int16_t * B, int row, int col){
	for(int i = 0 ; i < row ; ++i)
		for(int j = 0 ; j < col ; ++j)
			mat_element(C, col, i, j) = mat_element(A, col, i, j) - mat_element(B, col, i, j);
}

void mat_neg(int16_t * A, int row, int col){
	for(int i = 0 ; i < row ; ++i)
		for(int j = 0 ; j < col ; ++j)
			mat_element(A, col, i, j) = -mat_element(A, col, i, j);
}
void vector_mul_int16(int16_t *C, int16_t *A, int16_t *B, int len){
	uint16_t  st[16]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,};
	uint16_t 	mul[16];
	for (int i = 0; i < len / 16; i++)
	{
		for (int j = 0; j < 16;j++){
			mul[j] = A[16*i+j] * B[16*i+j];
			st[j] = st[j] + mul[j];
		}
	}
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
void mat_mul_avx2_int16_AmulBt(int16_t *C, int16_t *A, int16_t *B, int A_row, int A_col, int B_row){
        
        for(int i = 0; i < A_row; i++){
               mat_mul(C, B, A, B_row, A_col,1);
               A = A + A_col;
               C = C + B_row;
        }

}
void tri_mat_mul_avx2_con1(double *C, const double *A, const double *B, int row)
{ 
	   
	   for(int i = 0; i < row; i++){
			C[i] = 0;
			double st[4] = {0.0, 0.0, 0.0, 0.0};
			double mul[4];
			for (int j = 0; j < i / 4; j++)
			{
					 
				for (int m = 0; m < 4;m++){
					mul[m] = A[j*4+m] * B[j*4+m];
					st[m]= st[m] + mul[m];
				}
				
			}
			C[i] = st[0] + st[1] + st[2] + st[3];
			int res = i % 4;
			for(int k = i - res; k < i+1; k++){
				C[i] = C[i] + A[k] * B[k];
			}
			A = A + (i + 1);
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
void vector_mul_double(double *C, double *A, double *B, int len){
	   double st[4]={0.0,0.0,0.0,0.0};
	   double mul[4];
	   for (int i = 0; i < len / 4; i++)
	   {
			for (int j = 0; j < 4;j++){
				mul[j] = A[4*i+j] * B[4*i+j];
				st[j] = st[j] + mul[j];
			}
	   }
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
int generate_auxiliary_mat(double * L_22, double * L_32, double * L_33, int16_t * R)
{
	double sigma_prime_square = (PARAM_SIGMA1 * PARAM_SIGMA1 - PARAM_R * PARAM_R) / (PARAM_q * PARAM_R * PARAM_q * PARAM_R);
	int16_t * E = R, * S = R + PARAM_M * PARAM_M;
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
	for(int i = 0; i < PARAM_M; i++)
	{
		for(int j = 0 ; j <= i ; ++j)
			tri_mat_element(L_33, i, j) *= PARAM_q * PARAM_R;
		for(int j = 0 ; j < PARAM_N ; ++j)
			mat_element(L_32, PARAM_N, i, j) *= PARAM_q * PARAM_R;
	}
	for(int i = 0; i < PARAM_N; i++)
		for(int j = 0 ; j <= i ; ++j)
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

double norm_square(const int16_t * v, int n){
	double norm_square = 0;
	for(int i = 0; i < n; i++)
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
void unpack_mat_r_int16(int16_t *data, const uint8_t *buf,size_t buf_len)
{
    for (size_t i = 0; i < buf_len; i++)
    {
        data[0] = (buf[i] & 0xc0)>>6;
        data[1] = (buf[i] & 0x30)>>4;
        data[2] = (buf[i] & 0x0c)>>2;
        data[3] = (buf[i] & 0x03);
        data += 4;
    }
}




void unpack_mat_r(int16_t *data, const uint8_t *buf, int buf_len)
{
    for (size_t i = 0; i < (size_t)buf_len; i++)
    {
        data[0] = (buf[i] & 0xc0)>>6;
		if (data[0]==3){
			data[0] = -1;
		}
		data[1] = (buf[i] & 0x30)>>4;
		if (data[1]==3){
			data[1] = -1;
		}
        data[2] = (buf[i] & 0x0c)>>2;
		if (data[2]==3){
			data[2] = -1;
		}
        data[3] = (buf[i] & 0x03);
		if (data[3]==3){
			data[3] = -1;
		}
        data += 4;
    }
}








