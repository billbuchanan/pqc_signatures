#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "sampling.h"
#include "../sha3/shake.h"
#include "../params.h"
#include "../util.h"
#include "../normaldist/normaldist.h"
#include "samplez.h"

#define FACTOR 49.78042331916893 // sqrt(sigma_prime_square - 1)

void samplep(int16_t * p, int16_t * R, double * L_22, double * L_32, double * L_33,uint8_t seed[32])
{
	prng rng;
	get_seed(32, &rng, seed);
	uint8_t randomness[2 * PARAM_SEED_BYTES];
	prng_get_bytes(&rng, randomness, 2 * PARAM_SEED_BYTES);

	double factor1 = PARAM_q * PARAM_R / FACTOR, factor2 = PARAM_q * PARAM_R * FACTOR;
	int16_t *E = R, *S = R + PARAM_M * PARAM_M;

	// sample y ~ N(0, 1)
	double *y = malloc((PARAM_N + 2 * PARAM_M) * sizeof(double));
	normaldist(y, PARAM_N + 2 * PARAM_M, randomness); 
	double *y0 = y, *y1 = y + PARAM_M, *y2 = y + PARAM_M + PARAM_N;

	double * L_31 = malloc(PARAM_M * PARAM_M * sizeof(double));
	for(int i = 0; i < PARAM_M * PARAM_M; i++) L_31[i] = -E[i] * factor1;

	double *L_21 = malloc(PARAM_N * PARAM_M * sizeof(double));
	for(int i = 0; i < PARAM_N * PARAM_M; i++) L_21[i] = -S[i] * factor1; 
	
	double * temp0 = malloc(PARAM_M * sizeof(double));
	double * temp1 = malloc(PARAM_N * sizeof(double));
	double * c = malloc((PARAM_N + 2 * PARAM_M) * sizeof(double));
	double * c0 = c, * c1 = c + PARAM_M, * c2 = c + PARAM_M + PARAM_N;
	for(int i = 0; i < PARAM_M; i++) c2[i] = y2[i] * factor2;
	
	tri_mat_mul(temp1, L_22, y1, PARAM_N, 1);

	// memcpy(c1, temp1, PARAM_N * sizeof(double));
	for(int i = 0; i < PARAM_N; i++)  c1[i] = temp1[i];
	real_mat_mul(temp1, L_21, y2, PARAM_N, PARAM_M, 1);
	real_mat_add(c1, c1, temp1, PARAM_N, 1);
	
	
	tri_mat_mul(temp0, L_33, y0, PARAM_M, 1);
	// memcpy(c0, temp0, PARAM_M * sizeof(double));
	for(int i = 0; i < PARAM_M; i++) c0[i] = temp0[i];
	real_mat_mul(temp0, L_32, y1, PARAM_M, PARAM_N, 1);
	real_mat_add(c0, c0, temp0, PARAM_M, 1);
	real_mat_mul(temp0, L_31, y2, PARAM_M, PARAM_M, 1);
	real_mat_add(c0, c0, temp0, PARAM_M, 1);

	prng rng_samplez;
	get_seed(32, &rng_samplez, randomness + PARAM_SEED_BYTES);
	// sample p with c[i] being the center
	for (int i = 0; i < PARAM_N + 2 * PARAM_M; i++)
		p[i] = samplez(c[i], &rng_samplez);


	free(y); free(temp0); free(temp1); free(c);
	free(L_31); free(L_21); 

}

void sampleF(int16_t * z, int16_t * v,uint8_t seed[32]){
	prng rng;
	get_seed(32, &rng,seed);
	for(int i = 0; i < PARAM_M; ++i)
		samplef(&z[i], v[i], &rng);
}

void samplef(int16_t * z, int16_t v, prng *rng ){
	int16_t roundedv = (int16_t)floor(1.0 * v / PARAM_p + 0.5) % PARAM_q;
	if(roundedv < 0) roundedv += PARAM_q;
	*z = PARAM_q * base_sampler_RCDT_discrete_center(PARAM_q - roundedv, rng) - PARAM_q + roundedv;
}
