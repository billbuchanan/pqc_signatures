#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "params.h"
#include "api.h"
#include "util.h"
#include "sha3/shake.h"
#include "cpucycles.h"
#define NTESTS 10
uint64_t T0[NTESTS];
uint64_t T1[NTESTS];
void test_signature();
void test_cpucycles();
int main()
{
	// test_signature();
	test_cpucycles();
	return 0;
}

void test_signature()
{
	clock_t t0, t1;
	uint8_t *pk = malloc(CRYPTO_PUBLICKEYBYTES);
	uint8_t *sk = malloc(CRYPTO_SECRETKEYBYTES);
	// uint8_t seed[32] = {1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};

	// KeyGen(pk, sk, seed);
	crypto_sign_keypair(pk, sk);
	// for (int i = 0; i < CRYPTO_SECRETKEYBYTES;i++){
	// 	printf("%d ", sk[i]);
	// }
	// Sign
	unsigned long long mlen = 32, smlen;
	uint8_t m[mlen + PARAM_SALT_BYTES]; // = {0, 1, 2, 3, 4, 5, 6, 7};
	for(size_t i = 0; i < mlen; i++)  m[i] = i;
	uint8_t sm[PARAM_COMPRESSED_SIG_BYTES];
	
	// Sign(sig, pk, sk, msg, msg_len, seed);
	t0 = clock();
	crypto_sign(sm, &smlen, m, mlen, sk);
	t1 = clock();
	// printf("\nsign time is %f \n", (t1 - t0) / 1000.0);
	// Vrfy
	// Vrfy(pk, msg, msg_len, sig);
	t0 = clock();
	int t = crypto_sign_open(m, &mlen, sm, smlen, pk);
	t1 = clock();
	// printf("\nvrfy time is %f \n", (t1 - t0) / 1000.0);
	// printf("%d\n", t);

	// |pk| & |sig|
	// printf("\t|pk|: %.0f KB\n", ceil(PARAM_PK_BYTES / 1024.));
	// printf("\t|sig|: %d B\n\n", ((unsigned)sig[0] << 8) | sig[1]); // actual encoded |sig|

	free(pk);
	free(sk);
}

void test_cpucycles(){
	uint8_t *pk = malloc(CRYPTO_PUBLICKEYBYTES);
	uint8_t *sk = malloc(CRYPTO_SECRETKEYBYTES);
	// uint8_t seed[32] = {1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
	clock_t t0, t1;
	// KeyGen 
	
	t0 = clock();
	for (int i = 0; i < NTESTS; i++)
	{
		T0[i] = cpucycles();
		crypto_sign_keypair(pk, sk);
		T1[i] = cpucycles();
	}
	t1 = clock();
	printf("KeyGen time is %f \n", (t1 - t0) / 1000.0);
	print_results("KeyGen :", T0,T1, NTESTS);
	// Sign
	unsigned long long mlen = 32, smlen;
	uint8_t m[mlen + PARAM_SALT_BYTES]; // = {0, 1, 2, 3, 4, 5, 6, 7};
	for(size_t i = 0; i < mlen; i++)  m[i] = i;
	uint8_t sm[PARAM_COMPRESSED_SIG_BYTES];
	
	t0 = clock();
	for (int i = 0; i < NTESTS; i++)
	{
		T0[i] = cpucycles();
		crypto_sign(sm, &smlen, m, mlen, sk);
		T1[i] = cpucycles();
	}
	t1 = clock();
	printf("Sign time is %f \n", (t1 - t0) / 1000.0);
	print_results("Sign :",T0,T1, NTESTS);
	// for(int i = 0; i < PARAM_COMPRESSED_SIG_BYTES; ++i)
	// printf("%X ", sig[i]);
	// puts("\n\n");

	// Vrfy
	
	t0 = clock();
	for (int i = 0; i < NTESTS; i++)
	{
		T0[i] = cpucycles();
		crypto_sign_open(m, &mlen, sm, smlen, pk);
		T1[i] = cpucycles();
	}
	t1 = clock();
	printf("Vrfy time is %f \n", (t1 - t0) / 1000.0);
	print_results("Vrfy :", T0,T1, NTESTS);
	// |pk| & |sig|
	// printf("\t|pk|: %.0f KB\n", ceil(PARAM_PK_BYTES / 1024.));
	// printf("\t|sig|: %d B\n\n", ((unsigned)sig[0] << 8) | sig[1]); // actual encoded |sig|

	free(pk);
	free(sk);
} 