#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpucycles.h"
#include "speed_print.h"

#define NTESTS 1000

uint64_t t[NTESTS];

int main(void) {
    printf("%s\n", TUOV_ALGNAME );
    printf("sk size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("pk size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("signature overhead: %d\n\n", CRYPTO_BYTES );

    unsigned char sm[256 + CRYPTO_BYTES]; 
    unsigned char m[256];
    for (unsigned i = 0; i < 256; i++) {
        m[i] = i;
    }
    unsigned long long mlen = 256;
    unsigned long long smlen;

    unsigned char *pk = (unsigned char *)malloc( CRYPTO_PUBLICKEYBYTES );
    unsigned char *sk = (unsigned char *)malloc( CRYPTO_SECRETKEYBYTES );

    for(unsigned i=0; i<NTESTS; i++) {
      t[i] = cpucycles();
      crypto_sign_keypair( pk, sk);
    }
    print_results("crypto_sign_keypair: ", t, NTESTS);

    for(unsigned i=0; i<NTESTS; i++) {
      t[i] = cpucycles();
      crypto_sign( sm, &smlen, m, mlen, sk );
    }
    print_results("crypto_sign: ", t, NTESTS);

    for(unsigned i=0; i<NTESTS; i++) {
      t[i] = cpucycles();
      crypto_sign_open( m, &mlen, sm, smlen, pk );
    }
    print_results("crypto_sign_open: ", t, NTESTS);

    free( pk );
    free( sk );
    printf("\n\n");
    return 0;
}