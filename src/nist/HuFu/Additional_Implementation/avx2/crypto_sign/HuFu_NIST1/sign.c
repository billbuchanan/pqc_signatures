#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <time.h>

#include "params.h"
#include "random/random.h"
#include "aes/aes.h"
#include "sha3/fips202.h"
#include "gen_mat.h"
#include "util.h"
#include "rANS/compress.h"
#include "sampling/sampling.h"
#include "api.h"
#include "cpucycles.h"

// void KeyGen(uint8_t * pk, uint8_t * sk, uint8_t seed[32])
int crypto_sign_keypair(
    unsigned char *pk,
    unsigned char *sk
){
	// generate seed for mat A_hat and mat R
	// uint8_t seed[PARAM_SEED_BYTES] = {0};
	// uint8_t * seed = NULL;   
	uint8_t seed[32] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
						0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
						0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
						0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	prng rng;
	get_seed(32, &rng, seed);
	uint8_t randomness[2 * PARAM_SEED_BYTES];
	prng_get_bytes(&rng, randomness, 2 * PARAM_SEED_BYTES);

	// generate pseudorandom A_hat
	shake128(pk, PARAM_SEED_BYTES, randomness, PARAM_SEED_BYTES);
	int16_t A_hat[PARAM_M * PARAM_N];
	gen_mat(A_hat, PARAM_M * PARAM_N, pk, PARAM_SEED_BYTES);
	// sample R and compute auxiliary matrices L_22, L_32, and L_33
	int16_t R[(PARAM_M + PARAM_N) * PARAM_M];
	int shift = CRYPTO_PUBLICKEYBYTES + PARAM_R_BYTES;  ////////////
	double *L_22 = (double *)(sk + shift);
	shift += PARAM_N * (1 + PARAM_N) / 2 * sizeof(double);
	double *L_32 = (double *)(sk + shift);
	shift += PARAM_M * PARAM_N * sizeof(double);
	double *L_33 = (double *)(sk + shift);

	int keygen_repeat = 0;
	do
	{			
        cbd(R, randomness + PARAM_SEED_BYTES);
		shake128(randomness + PARAM_SEED_BYTES, PARAM_SEED_BYTES, randomness + PARAM_SEED_BYTES, PARAM_SEED_BYTES); 
		keygen_repeat++;
	} while (generate_auxiliary_mat(L_22, L_32, L_33, R) == 0);
	
		// compute matrix B = A_hat * S + E
	int16_t *B = (int16_t *)(pk + PARAM_SEED_BYTES);
	mat_mul_avx2(B, A_hat, R + PARAM_M * PARAM_M, PARAM_M, PARAM_N, PARAM_M);
	mat_add_avx2(B, B, R, PARAM_M, PARAM_M);
	mat_neg(B, PARAM_M, PARAM_M);
	for (int i = 0; i < PARAM_M; ++i)
		mat_element(B, PARAM_M, i, i) += PARAM_p;

	// pack sk
	memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);  
	for (int i = 0; i < (PARAM_N + PARAM_M) * PARAM_M; i++)
		if (R[i] < 0) R[i] += 4;
	
	pack_mat_r(sk + CRYPTO_PUBLICKEYBYTES, R);
	// for (int i = CRYPTO_PUBLICKEYBYTES+PARAM_R_BYTES+ PARAM_N * (1 + PARAM_N) / 2 * sizeof(double); i < CRYPTO_PUBLICKEYBYTES+PARAM_R_BYTES+ PARAM_N * (1 + PARAM_N) / 2 * sizeof(double)+PARAM_M * PARAM_N * sizeof(double); i++)
	// {
	// 	printf("%d ", sk[i]);
	// }
	return 0;
}

// void Sign(uint8_t *sig, uint8_t *pk, uint8_t *sk, uint8_t *msg, size_t msg_len, uint8_t seed[32]){
int crypto_sign(
    unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk
){
	const uint8_t * pk = sk;   
	// recover A_hat
	int16_t A_hat[PARAM_M * PARAM_N];
	gen_mat(A_hat, PARAM_M * PARAM_N, pk, PARAM_SEED_BYTES);

	// recover B
	int16_t *B = (int16_t *)(pk + PARAM_SEED_BYTES);

	int shift = CRYPTO_PUBLICKEYBYTES + PARAM_R_BYTES;  
	double *L_22 = (double *)(sk + shift);
	shift += PARAM_N * (1 + PARAM_N) / 2 * sizeof(double);
	double *L_32 = (double *)(sk + shift);
	shift += PARAM_M * PARAM_N * sizeof(double);
	double *L_33 = (double *)(sk + shift);
	
	// offline task: 1. sample p; 2. compute v = Ap = [I, A_hat, B](p0, p1, p2) = p0 + A_hat * p1 + B * p2
	int16_t *p = malloc((PARAM_N + 2 * PARAM_M) * sizeof(int16_t));
	int16_t *v = malloc(PARAM_M * sizeof(int16_t));

	// online task:  1. compute u = hash(msg||salt); 2. compute v = u - Ap; 3. sample gadget ang get z
	int16_t *u = malloc(PARAM_M * sizeof(int16_t));
	int16_t *z = malloc(PARAM_M * sizeof(int16_t));
	int16_t *signature0 = malloc(PARAM_M * sizeof(int16_t));
	int16_t *signature = malloc((PARAM_M + PARAM_N) * sizeof(int16_t));
	int16_t *error = malloc(PARAM_M * sizeof(int16_t));
	int16_t *temp = malloc(PARAM_M * sizeof(int16_t));

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
	do
	{
		prng_get_bytes(&rng_salt, (void *)(mexp + mlen), PARAM_SALT_BYTES); 
		shake128((uint8_t *)u, PARAM_M * sizeof(int16_t), mexp, mlen + PARAM_SALT_BYTES); 

		// offline: sample p and compute v = u - [I, A_hat, B] * p;
		samplep(p, sk + CRYPTO_PUBLICKEYBYTES, L_22, L_32, L_33, seed_p); 

		memcpy(v, p, PARAM_M * sizeof(int16_t));
		mat_mul_avx2_con1(temp, A_hat, p + PARAM_M, PARAM_M, PARAM_N);
		mat_add_avx2(v, v, temp, PARAM_M, 1);
		mat_mul_avx2_con1(temp, B, p + PARAM_M + PARAM_N, PARAM_M, PARAM_M);
		mat_add_avx2(v, v, temp, PARAM_M, 1);
		mat_sub_avx2(v, u, v, PARAM_M, 1);

		// online: sample gadget and compute the approx. error
		sampleF(z, v, seed_z);

		for (int i = 0; i < PARAM_M; i++) error[i] = v[i] - z[i] * PARAM_p;

		// signature = [signature1, signature2] = [p1 + R2 * z, p2 + z]
		for (int i = 0; i < PARAM_N/4;i++){
			int16_t S[4*PARAM_M];
			unpack_mat_r_avx2(S, sk + CRYPTO_PUBLICKEYBYTES +2 * PARAM_M * PARAM_M / 8+PARAM_M*i, PARAM_M); 
			mat_mul_avx2_con1(signature+4*i, S, z, 4, PARAM_M);
		}
		memcpy(signature + PARAM_N, z, PARAM_M * sizeof(int16_t));
		mat_add_avx2(signature, signature, p + PARAM_M, PARAM_M + PARAM_N, 1);
		// recover signature0 = p0 + R1 * z + e
		for (int i = 0; i < PARAM_M / 4;i++){
			int16_t E[4*PARAM_M];
			unpack_mat_r_avx2(E, sk + CRYPTO_PUBLICKEYBYTES+PARAM_M*i, PARAM_M); 
			mat_mul_avx2_con1(signature0+4*i, E, z, 4, PARAM_M);
		}
		mat_add_avx2(signature0, signature0, p, PARAM_M, 1);
		mat_add_avx2(signature0, signature0, error, PARAM_M, 1);
		preimage_norm_square = norm_square(signature0, PARAM_M) + norm_square(signature, PARAM_N + PARAM_M);

		// compress signature --> sig, consistint of 2 bytes for |sig|, sig_encode_size bytes, and salt
		sig_len = compress_sig(signbytes, signature);  

		sign_repeat++;

	} while (preimage_norm_square > PARAM_BOUND_SQUARE || sig_len == 0 || sig_len > PARAM_SIG_BYTES - 2 - PARAM_SALT_BYTES-mlen);///
	
	sm[0] = sig_len >> 8; sm[1] = sig_len & 0xFF;
	memcpy(sm + 2, signbytes + PARAM_SIG_ENCODE_MAX_BYTES - sig_len, sig_len);
	memcpy(sm + 2 + sig_len, mexp, mlen+PARAM_SALT_BYTES);
	*smlen = sig_len + 2 + PARAM_SALT_BYTES + mlen;
	memset(sm + *smlen, 0xFF, PARAM_SIG_BYTES - *smlen); // padding
	

	free(p);
	free(v);
	free(u);
	free(error);
	free(z);
	free(signature0);
	free(signature);
	free(temp);
	free(signbytes);
	return 0;
}

// void Vrfy(uint8_t *pk, uint8_t *msg, size_t msg_len, uint8_t *sig){
int crypto_sign_open(
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk
){
	// recover A_hat
	int16_t A_hat[PARAM_M * PARAM_N];
	gen_mat(A_hat, PARAM_M * PARAM_N, pk, PARAM_SEED_BYTES);

	// recover B
	int16_t B[PARAM_M * PARAM_M];
	memcpy(B, pk + PARAM_SEED_BYTES, PARAM_M * PARAM_M * sizeof(int16_t));

	// recover sig_len
	unsigned sig_len = ((unsigned)sm[0] << 8) | sm[1];
	*mlen = smlen - sig_len - PARAM_SALT_BYTES - 2;
	
	// compute u
	uint8_t mexp[*mlen+PARAM_SALT_BYTES];
	memcpy(mexp , sm + 2 + sig_len, *mlen+PARAM_SALT_BYTES);
	memcpy(m, mexp, *mlen);
	int16_t *u = malloc(PARAM_M * sizeof(int16_t));
	shake128((uint8_t *)u, PARAM_M * sizeof(int16_t), mexp, *mlen + PARAM_SALT_BYTES); 

	// recover signature0 = u - [A_hat, B] * signature and check validity
	int16_t *signature0 = malloc(PARAM_M * sizeof(int16_t));
	int16_t *signature = malloc((PARAM_M + PARAM_N) * sizeof(int16_t));
	int decompress_result = decompress_sig(signature, sm + 2, sig_len);  

	if (decompress_result){
		int16_t temp[PARAM_M];
		mat_mul_avx2_con1(temp, A_hat, signature, PARAM_M, PARAM_N);
		mat_sub_avx2(signature0, u, temp, PARAM_M, 1);
		mat_mul_avx2_con1(temp, B, signature + PARAM_N, PARAM_M, PARAM_M);
		mat_sub_avx2(signature0, signature0, temp, PARAM_M, 1);
		for (int i = 0; i < PARAM_M; i++)
			signature0[i] %= PARAM_Q;

		double preimage_norm_square = norm_square(signature0, PARAM_M) + norm_square(signature, PARAM_N + PARAM_M);
		if (preimage_norm_square > PARAM_BOUND_SQUARE)
			return 1; 
	}
	else
	{
		return -1; 
	}
	free(u);
	free(signature0);
	free(signature);

	

	return 0; 
}