#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "rng.h"
#include <stdint.h>



#define MLEN 59

int randombytes (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size) ;

char *showhex(uint8_t a[], int size) {


    char *s =malloc(size * 2 + 1);

    for (int i = 0; i < size; i++)
        sprintf(s + i * 2, "%02x", a[i]);

    return(s);
}


int main(void)
{
  size_t j;
  int ret;
  uint8_t b;
  uint8_t m[MLEN + CRYPTO_BYTES];
  uint8_t m2[MLEN + CRYPTO_BYTES];

  uint8_t pk[CRYPTO_PUBLICKEYBYTES+1];
  uint8_t sk[CRYPTO_SECRETKEYBYTES+1];
 unsigned char sm[sizeof(m) + CRYPTO_BYTES];
 long long unsigned int smlen = sizeof(sm);

    unsigned char mprime[50] = { 0 };
 
    long long unsigned int mlen = sizeof(mprime);

  printf("NAME: %s\n", CRYPTO_ALGNAME);
  printf("CRYPTO_PUBLICKEYBYTES = %d\n", CRYPTO_PUBLICKEYBYTES);
  printf("CRYPTO_SECRETKEYBYTES = %d\n", CRYPTO_SECRETKEYBYTES);
  printf("CRYPTO_BYTES = %d\n", CRYPTO_BYTES);
    randombytes(m, MLEN);

  printf("\nMessage: %s\n",showhex(m,MLEN));

    crypto_sign_keypair(pk, sk);

    crypto_sign(sm, &smlen, m, MLEN, sk);

    ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);

    if(ret) {
      fprintf(stderr, "Verification failed\n");
      return -1;
    }
    if(smlen != MLEN + CRYPTO_BYTES) {
      fprintf(stderr, "Signed message lengths wrong\n");
      return -1;
    } 
    if(mlen != MLEN) {
      fprintf(stderr, "Message lengths wrong\n");
      return -1;
    }
    for(j = 0; j < MLEN; ++j) {
      if(m2[j] != m[j]) {
        fprintf(stderr, "Messages don't match\n");
        return -1;
      }
    }


  

  printf("\nAlice Public key: %s\n",showhex(pk,CRYPTO_PUBLICKEYBYTES));
  printf("Alice Secret key: %s\n",showhex(pk,CRYPTO_SECRETKEYBYTES));

  printf("Signature (showing 1/512th): %s\n",showhex(sm,smlen/512));
  return 0;
}

int randombytes (unsigned char* random_array, unsigned long long num_bytes)
{
 // unsigned char *random_array = malloc (num_bytes);

  size_t i;

srand ((unsigned int) time (NULL));

  for (i = 0; i < num_bytes; i++)
  {
    random_array[i] = rand ();
  }

  return 0;
} 