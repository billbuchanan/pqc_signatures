#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "impl.h"
#include "rng.h"
#include "cpucycles.h"

const size_t n = 64;

int main(void)
{
	size_t i;
	long long cycle1, cycle2, cycle3, cycle4, cycle5;
    pubkey_t pk;
    privkey_t sk;
	signature_t sig;
	uint8_t seed[32];
	uint8_t msg[32];

	int v;
	
	for (i = 0; i < 1000; i++)
	{
		randombytes(seed, 32);
		cycle1 = cpucycles();
		keygen(&pk, &sk, n, seed);
		cycle2 = cpucycles();
		randombytes(seed, 32);
		randombytes(msg, 32);
		cycle3 = cpucycles();
		sign(&sig, &sk, msg, 32, n, seed);
		cycle4 = cpucycles();
		v = verify(&pk, msg, 32, &sig, n);
		cycle5 = cpucycles();
		printf("%lld,%lld,%lld,%d\n", cycle2 - cycle1, cycle4 - cycle3, cycle5 - cycle4, v);
	}

	return 0;
}
