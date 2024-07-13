#include "../sign.h"
#include "../poly.h"
#include "../fips202.h"
#include "../utils.h"
#include "../params.h"
#include "../randombytes.h"
#include <stdio.h>

#define NTESTS 1000

int main()
{
	size_t j;
	size_t mlen = NTESTS, mlen2;
	uint8_t pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES] = {0}, sm[CRYPTO_BYTES + NTESTS], m2[NTESTS];
	size_t smlen;
	uint8_t m[NTESTS];
	uint8_t results[NTESTS];
	for(j = 0; j < NTESTS; j++)
	{
		crypto_sign_keypair(pk, sk);
		//printf("Keypair done\n");
		crypto_sign(sm, &smlen, m, mlen, sk);
		//printf("Sign done\n");
		results[j] = crypto_sign_open(m2, &mlen2, sm, smlen, pk);
		//printf("Verify done\n");
	}
	for(j = 0; j < NTESTS; j++)
        {
		printf("Result: %u\n", results[j]);
	}
	printf("Mode,Keygencycles,Keygenmilli,Signcycles,Signmilli,Verifycycles,Verifymilli\n");
	//printf("Mode,Keygencycles,Keygenmilli,Signcycles,Signmilli,Verifycycles,Verifymilli,LMP,Weight,Attempts,Opts,MULT_X,LOOP_THRESHOLD\n");
}
