﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "api.h"





int randombytes1 (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size) ;

int main(void) {
  printf("NAME: %s\n", CRYPTO_ALGNAME);
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

return(0);

    unsigned char       *m,*sm;
         unsigned long long mlen = 33;
    unsigned long long smlen;

        m = (unsigned char *)calloc(mlen, sizeof(unsigned char));

        sm = (unsigned char *)calloc(mlen+CRYPTO_BYTES, sizeof(unsigned char));

    for (unsigned i = 0; i < mlen; i++) {
        m[i] = i;
    }


 unsigned char       *pk, *sk;
        pk = (unsigned char *)calloc(CRYPTO_PUBLICKEYBYTES, sizeof(unsigned char));
        sk = (unsigned char *)calloc(CRYPTO_SECRETKEYBYTES, sizeof(unsigned char));

            int ret_val;

        if ( (ret_val = crypto_sign_keypair(pk, sk)) != 0) {
            printf("crypto_sign_keypair returned <%d>\n", ret_val);
            return 0;
        }

	randombytes1(m, mlen);


        int r1, r2;
        r1 = crypto_sign( sm, &smlen, m, mlen, sk );
        if ( 0 != r1 ) {
            printf("crypto_sign() return %d.\n", r1);
            return -1;
        }

        r2 = crypto_sign_open( m, &mlen, sm, smlen, pk );
        if ( 0 != r2 ) {
            printf("crypto_sign_open() return %d.\n", r2);
            return -1;
        }


 printf("\nMessage: %s\n",showhex(m,mlen));
  printf("\nAlice Public key (16th of key): %s\n\n",showhex(pk,CRYPTO_PUBLICKEYBYTES/16));
  printf("Alice Secret key (16th of key): %s\n\n",showhex(sk,CRYPTO_SECRETKEYBYTES/16 ));

  printf("Signature (16th of signature): %s\n\n",showhex(sm,CRYPTO_BYTES/16));
  if (r2==0) printf("Signature verified");
 


    return 0;
}



char *showhex(uint8_t a[], int size) {


    char *s =malloc(size * 2 + 1);

    for (int i = 0; i < size; i++)
        sprintf(s + i * 2, "%02x", a[i]);

    return(s);
}
int randombytes1 (unsigned char* random_array, unsigned long long num_bytes)
{
 // unsigned char *random_array = calloc (num_bytes);

  size_t i;

srand ((unsigned int) time (NULL));

  for (i = 0; i < num_bytes; i++)
  {
    random_array[i] = rand ();
  }

  return 0;
} 