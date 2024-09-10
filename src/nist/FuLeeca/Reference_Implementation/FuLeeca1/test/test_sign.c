#include "../sign.h"
#include "../poly.h"
#include "../fips202.h"
#include "../utils.h"
#include "../params.h"
#include "../randombytes.h"
#include <cpucycles.h>
#include <stdio.h>

#define NTESTS 1000

int main()
{
	size_t j;
	size_t mlen = NTESTS, mlen2;
	uint8_t pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES], sm[CRYPTO_BYTES + NTESTS], m2[NTESTS];
	size_t smlen;
	uint8_t m[NTESTS];
	long long keygencounts[NTESTS] = {0}, signcounts[NTESTS], verifycounts[NTESTS];
	long long count1, count2, permillisecond;
	long long signcounts_sum, signmilli_sum, attempts_sum, opts_sum;
	for(j = 0; j < NTESTS; j++)
	{
		count1 = cpucycles();
		crypto_sign_keypair(pk, sk);
		count2 = cpucycles();
		keygencounts[j] = count2 - count1;
		count1 = cpucycles();
		crypto_sign(sm, &smlen, m, mlen, sk);
		count2 = cpucycles();
		signcounts[j] = count2 - count1;
		count1 = cpucycles();
		crypto_sign_open(m2, &mlen2, sm, smlen, pk);
		count2 = cpucycles();
		verifycounts[j] = count2 - count1;
		m[j] = 1;
	}
	printf("Mode,Keygencycles,Keygenmilli,Signcycles,Signmilli,Verifycycles,Verifymilli\n");
	permillisecond = cpucycles_persecond() / 1000;
	signcounts_sum = 0;
	signmilli_sum = 0;
	attempts_sum = 0;
	opts_sum = 0;
	for(j = 0; j < NTESTS; j++)
	{
		printf("%i,", FULEECA_MODE);
		printf("%lli,%lli,%lli,%lli,%lli,%lli\n", keygencounts[j], keygencounts[j]/permillisecond, signcounts[j], signcounts[j]/permillisecond, verifycounts[j], verifycounts[j]/permillisecond);
		signcounts_sum += signcounts[j];
		signmilli_sum += signcounts[j]/permillisecond;
	}
	printf("Summary (Sign): Attempts (avg.): %lli, Optimizations (avg.): %lli, Cycles (avg.): %lli, Milliseconds(avg.): %lli\n", attempts_sum/NTESTS, opts_sum/NTESTS, signcounts_sum/NTESTS, signmilli_sum/NTESTS);
}
