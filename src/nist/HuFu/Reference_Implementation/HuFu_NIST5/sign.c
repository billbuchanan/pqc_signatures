#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <time.h>

#include "api.h"
#include "params.h"
#include "random/random.h"
#include "aes/aes.h"
#include "sha3/fips202.h"
#include "gen_mat.h"
#include "util.h"
#include "rANS/compress.h"
#include "sampling/sampling.h"
#include "pack.h"

int crypto_sign_keypair(
    unsigned char *pk,
    unsigned char *sk
){
	// generate seed for mat A_hat and mat R
	// uint8_t * seed = NULL;   
	uint8_t seed[32] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
					 	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
					 	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
					 	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	prng rng;
	get_seed(32, &rng, seed);
	uint8_t randomness[2 * PARAM_SEED_BYTES];
	prng_get_bytes(&rng, randomness, 2 * PARAM_SEED_BYTES);

	// Step 1: generate pseudorandom A_hat
	shake128(pk, PARAM_SEED_BYTES, randomness, PARAM_SEED_BYTES);
	uint8_t * A_hat_u8 =  malloc((PARAM_M * PARAM_N *17)>>3); 
	gen_mat(A_hat_u8, (PARAM_M * PARAM_N *17)>>3, pk, PARAM_SEED_BYTES);
	
	int32_t *A_hat=malloc(PARAM_M * PARAM_N*sizeof(int32_t));
	unpack((uint32_t*)A_hat,PARAM_M * PARAM_N,A_hat_u8,A_hat_u8+2*PARAM_M * PARAM_N,(PARAM_M * PARAM_N *17)>>3);
	

	// Step 2 - 5: sample R and compute auxiliary matrices L_22, L_32, and L_33
	uint8_t seed_R[PARAM_SEED_BYTES];
	memcpy(seed_R, randomness + PARAM_SEED_BYTES, PARAM_SEED_BYTES);
	int32_t * R = malloc((PARAM_M + PARAM_N) * PARAM_M * sizeof(int32_t)); 
	double * L_22 = malloc(PARAM_N * (1 + PARAM_N) / 2 * sizeof(double)); 
	double * L_32 = malloc(PARAM_M * PARAM_N * sizeof(double)); 
	double * L_33 = malloc(PARAM_M * (1 + PARAM_M) / 2 * sizeof(double)); 

	int keygen_repeat = 0;
	 do{
		cbd(R, seed_R);
		shake128(seed_R, PARAM_SEED_BYTES, seed_R, PARAM_SEED_BYTES); 
		keygen_repeat++;
	} while(generate_auxiliary_mat(L_22, L_32, L_33, R) == 0);

	// Step 6: compute matrix B = A_hat * S + E
	int32_t * B = malloc(PARAM_M * PARAM_M * sizeof(int32_t)); 
	mat_mul(B, A_hat, R + PARAM_M * PARAM_M, PARAM_M, PARAM_N, PARAM_M);
	mat_add(B, B, R, PARAM_M, PARAM_M);
	mat_neg(B, PARAM_M, PARAM_M);
	for(int i = 0; i < PARAM_M; ++i) mat_element(B, PARAM_M, i, i) += PARAM_p;

	// pack pk
	for(int i = 0; i < PARAM_M * PARAM_M; i++) {
		B[i] = modq(B[i]); if(B[i] < 0) B[i] += PARAM_Q;
	}
	pack(pk + PARAM_SEED_BYTES, PARAM_B_BYTES, (uint32_t *)B, PARAM_M * PARAM_M);
	
	// pack sk
	memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);  

	for(int i  = 0; i < (PARAM_N + PARAM_M) * PARAM_M; i++) if(R[i] < 0) R[i] += 4;
	pack_mat_r(sk + CRYPTO_PUBLICKEYBYTES, R);  

	int shift = CRYPTO_PUBLICKEYBYTES + PARAM_R_BYTES; 
	memcpy(sk + shift, L_22, PARAM_N * (1 + PARAM_N) / 2 * sizeof(double));
	shift += PARAM_N * (1 + PARAM_N) / 2 * sizeof(double);
	memcpy(sk + shift, L_32, PARAM_M * PARAM_N * sizeof(double));
	shift += PARAM_M * PARAM_N * sizeof(double);
	memcpy(sk + shift, L_33, PARAM_M * (1 + PARAM_M) / 2 * sizeof(double));

	free(A_hat_u8); free(R); free(B);
	free(A_hat);
	free(L_22); free(L_32); free(L_33); 

	return 0;
}


int crypto_sign(
    unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk
){
	const uint8_t * pk = sk;   
	// Step 1: recover A_hat
	uint8_t * A_hat_u8 =  malloc((PARAM_M * PARAM_N *17)>>3); 
	gen_mat(A_hat_u8, (PARAM_M * PARAM_N *17)>>3, pk, PARAM_SEED_BYTES);
	
	int32_t *A_hat=malloc(PARAM_M * PARAM_N*sizeof(int32_t));
	unpack((uint32_t*)A_hat,PARAM_M * PARAM_N,A_hat_u8,A_hat_u8+2*PARAM_M * PARAM_N,(PARAM_M * PARAM_N *17)>>3);
	
	// Step 1: recover B
	int32_t * B = malloc(PARAM_M * PARAM_M * sizeof(int32_t));
	unpack((uint32_t *)B, PARAM_M * PARAM_M, pk + PARAM_SEED_BYTES, pk + PARAM_SEED_BYTES+2*PARAM_M * PARAM_M,(PARAM_M * PARAM_M*17)>>3);

	// recover R and auxiliary matrices
	int32_t * R = malloc((PARAM_M + PARAM_N) * PARAM_M * sizeof(int32_t)); 
	unpack_mat_r(R, sk + CRYPTO_PUBLICKEYBYTES);  
	for(int i = 0; i < (PARAM_M + PARAM_N) * PARAM_M; i++) if(R[i] == 3) R[i] = -1;	   

	double * L_22 = malloc(PARAM_N * (1 + PARAM_N) / 2 * sizeof(double)); 
	double * L_32 = malloc(PARAM_M * PARAM_N * sizeof(double)); 
	double * L_33 = malloc(PARAM_M * (1 + PARAM_M) / 2 * sizeof(double)); 
	int shift = CRYPTO_PUBLICKEYBYTES + PARAM_R_BYTES;  
	memcpy(L_22, sk + shift, PARAM_N * (1 + PARAM_N) / 2 * sizeof(double));
	shift += PARAM_N * (1 + PARAM_N) / 2 * sizeof(double);
	memcpy(L_32, sk + shift, PARAM_M * PARAM_N * sizeof(double));
	shift += PARAM_M * PARAM_N * sizeof(double);
	memcpy(L_33, sk + shift, PARAM_M * (1 + PARAM_M) / 2 * sizeof(double));

	// offline task: 1. sample p; 2. compute v = Ap = [I, A_hat, B](p0, p1, p2) = p0 + A_hat * p1 + B * p2
	int32_t * p = malloc((PARAM_N + 2 * PARAM_M) * sizeof(int32_t));
	int32_t * v = malloc(PARAM_M * sizeof(int32_t));

	// online task:  1. compute u = hash(msg||salt); 2. compute v = u - Ap; 3. sample gadget ang get z
	int32_t *u = malloc(PARAM_M * sizeof(int32_t));
	int32_t *z = malloc(PARAM_M * sizeof(int32_t));
	int32_t *signature0 = malloc(PARAM_M * sizeof(int32_t));
	int32_t *signature = malloc((PARAM_M + PARAM_N) * sizeof(int32_t));
	int32_t *error = malloc(PARAM_M * sizeof(int32_t));
	int32_t *temp = malloc(PARAM_M * sizeof(int32_t));

	double preimage_norm_square;
	unsigned sig_len;
	int sign_repeat = 0;

	// generate seed for salt, p and z
	// uint8_t * seed = NULL;  
	uint8_t seed[32] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
					 	0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
					 	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
					 	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	prng rng;
	get_seed(32, &rng, seed);
	uint8_t randomness[3 * PARAM_SEED_BYTES]; 
	prng_get_bytes(&rng, randomness, 3 * PARAM_SEED_BYTES);

	uint8_t * seed_salt = randomness;
	uint8_t * seed_p = randomness + PARAM_SEED_BYTES;
	uint8_t * seed_z = randomness + 2 * PARAM_SEED_BYTES;
	uint8_t *signbytes = malloc(PARAM_SIG_ENCODE_MAX_BYTES);
	prng rng_salt;
	get_seed(32, &rng_salt, seed_salt);
	uint8_t mexp[mlen+PARAM_SALT_BYTES];
	memcpy(mexp, m, mlen);
	do{    
		// sample salt
		prng_get_bytes(&rng_salt, (void *)(mexp + mlen), PARAM_SALT_BYTES); 
		shake128((uint8_t *)u, PARAM_M * sizeof(int32_t), mexp, mlen + PARAM_SALT_BYTES);  

		// Step 2 - 3: sample p and compute Ap
		samplep(p, R, L_22, L_32, L_33, seed_p); 
		memcpy(v, p, PARAM_M * sizeof(int32_t));
		mat_mul(temp, A_hat, p + PARAM_M, PARAM_M, PARAM_N, 1);

		mat_add(v, v, temp, PARAM_M, 1);
		mat_mul(temp, B, p + PARAM_M + PARAM_N, PARAM_M, PARAM_M, 1);
		mat_add(v, v, temp, PARAM_M, 1);
		mat_sub(v, u, v, PARAM_M, 1); 

		// Step 6 - 9: sample z and compute the approx. error
		sampleF(z, v, seed_z);

		for(int i = 0; i < PARAM_M; i++) error[i] = modq(v[i] - z[i] * PARAM_p);

		// Step 10: compute signature = [signature1, signature2] = [p1 + R2 * z, p2 + z]
		mat_mul(signature, R + PARAM_M * PARAM_M, z, PARAM_N, PARAM_M, 1);
		memcpy(signature + PARAM_N, z, PARAM_M * sizeof(int32_t));
		mat_add(signature, signature, p + PARAM_M, PARAM_M + PARAM_N, 1);
		// recover signature0 = p0 + R1 * z + e
		mat_mul(signature0, R, z, PARAM_M, PARAM_M, 1);
		mat_add(signature0, signature0, p, PARAM_M, 1);
		mat_add(signature0, signature0, error, PARAM_M, 1);

		// Step 11: compute norm of preimage
		preimage_norm_square = norm_square(signature0, PARAM_M) + norm_square(signature, PARAM_N + PARAM_M);

		// Step 14: compress signature by rANS
		sig_len = compress_sig(signbytes, signature); 

		sign_repeat++; 

	} while(preimage_norm_square > PARAM_BOUND_SQUARE || sig_len == 0 || sig_len > PARAM_SIG_BYTES - 2 - PARAM_SALT_BYTES - mlen); ////
	
	// Step 14: compress signature --> sm, consistint of 2 bytes for |sm|, sig_encode_size bytes, and salt
	sm[0] = sig_len >> 8; sm[1] = sig_len & 0xFF;
	memcpy(sm + 2, signbytes + PARAM_SIG_ENCODE_MAX_BYTES - sig_len, sig_len);
	memcpy(sm + 2 + sig_len, mexp, mlen+PARAM_SALT_BYTES);
	*smlen = sig_len + 2 + PARAM_SALT_BYTES + mlen;
	memset(sm + *smlen, 0xFF, PARAM_SIG_BYTES - *smlen); // padding
	

	free(R); free(B);
	free(L_22); free(L_32); free(L_33); 
	free(p); free(v); 
	free(u); free(error);
	free(z); free(signature0); free(signature); free(temp);
	free(signbytes);
	free(A_hat_u8);
	free(A_hat);

	return 0;
}

int crypto_sign_open(
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk
){
	// Step 1: recover A_hat
	uint8_t * A_hat_u8 =  malloc((PARAM_M * PARAM_N *17)>>3); 
	gen_mat(A_hat_u8, (PARAM_M * PARAM_N *17)>>3, pk, PARAM_SEED_BYTES);

	int32_t *A_hat=malloc(PARAM_M * PARAM_N*sizeof(int32_t));	
	unpack((uint32_t*)A_hat,PARAM_M * PARAM_N,A_hat_u8,A_hat_u8+2*PARAM_M * PARAM_N,(PARAM_M * PARAM_N *17)>>3);
	
	// recover B
	int32_t * B = malloc(PARAM_M * PARAM_M * sizeof(int32_t));
	unpack((uint32_t *)B, PARAM_M * PARAM_M, pk + PARAM_SEED_BYTES, pk + PARAM_SEED_BYTES+2*PARAM_M * PARAM_M, (PARAM_M * PARAM_M *17)>>3);

	// recover sig_len
	unsigned sig_len = ((unsigned)sm[0] << 8) | sm[1];
	*mlen = smlen - sig_len - PARAM_SALT_BYTES - 2;
	
	// Step 6: recover u
	uint8_t mexp[*mlen+PARAM_SALT_BYTES];
	memcpy(mexp , sm + 2 + sig_len, *mlen+PARAM_SALT_BYTES);
	memcpy(m, mexp, *mlen);
	int32_t *u = malloc(PARAM_M * sizeof(int32_t));
	shake128((uint8_t *)u, PARAM_M * sizeof(int32_t), mexp, *mlen + PARAM_SALT_BYTES); 

	// recover signature0 = u - [A_hat, B] * signature and check validity  
	int32_t * signature0 = malloc(PARAM_M * sizeof(int32_t));
	int32_t * signature = malloc((PARAM_M + PARAM_N) * sizeof(int32_t));
	// Step 5: decompress signature 
	int decompress_result = decompress_sig(signature, sm + 2, sig_len); 


	if(decompress_result){
		// Step 6: recover signature0
		int32_t temp[PARAM_M];
		mat_mul(temp, A_hat, signature, PARAM_M, PARAM_N, 1);
		mat_sub(signature0, u, temp, PARAM_M, 1);
		mat_mul(temp, B, signature + PARAM_N, PARAM_M, PARAM_M, 1);
		mat_sub(signature0, signature0, temp, PARAM_M, 1);
		for(int i = 0; i < PARAM_M; i++) signature0[i] %= PARAM_Q;
		// Step 7: check norm
		double preimage_norm_square = norm_square(signature0, PARAM_M) + norm_square(signature, PARAM_N + PARAM_M);
		if(preimage_norm_square > PARAM_BOUND_SQUARE) 
			return 1; 
	}else{
		return -1; 
	}
	
	free(B); 
	free(u); free(signature0); free(signature);
	free(A_hat_u8);
	free(A_hat);
	return 0; 
}