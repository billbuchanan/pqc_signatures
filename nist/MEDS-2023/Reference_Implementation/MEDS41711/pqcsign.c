#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "api.h"

long long cpucycles(void)
{
  return __rdtsc();
}




int randombytes1 (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size) ;

int main(void) {
  long long keygen_time = 0;
  long long sign_time = 0;
  long long verify_time = 0;
    unsigned long long mlen = 256;
    unsigned long long smlen;

  uint8_t sig[CRYPTO_BYTES +  mlen];
  unsigned long long sig_len = sizeof(sig);

  printf("NAME: %s\n", CRYPTO_ALGNAME);
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    unsigned char sm[256 + CRYPTO_BYTES];
    unsigned char m[256];
    for (unsigned i = 0; i < 256; i++) {
        m[i] = i;
    }


    unsigned char *pk = (unsigned char *)malloc( CRYPTO_PUBLICKEYBYTES );
    unsigned char *sk = (unsigned char *)malloc( CRYPTO_SECRETKEYBYTES );


            int r0;

	randombytes1(m, mlen);


    keygen_time = -cpucycles();
    crypto_sign_keypair(pk, sk);
    keygen_time += cpucycles();


    sign_time = -cpucycles();
    crypto_sign(sig, &sig_len, (const unsigned char *)m, sizeof(m), sk);
    sign_time += cpucycles();



    unsigned char msg_out[17];
    unsigned long long msg_out_len = sizeof(msg_out);

    verify_time = -cpucycles();
    int ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);
    verify_time += cpucycles();


    if (ret != 0)
    {
      fprintf(stderr, "\n\n   ERROR! Signature did not verify!\n\n\n");

      exit(-1);
    }

    printf("Keygen:\t%llu cycles\n ",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n",  verify_time);
  
 printf("\nMessage: %s\n",showhex(m,mlen));
  printf("\nAlice Public key (16th of key): %s\n\n",showhex(pk,CRYPTO_PUBLICKEYBYTES/16));
  printf("Alice Secret key (128th of signature): %s\n\n",showhex(sk,CRYPTO_SECRETKEYBYTES/128 ));

  printf("Signature (128th of signature): %s\n\n",showhex(sig,CRYPTO_BYTES/128));
  if (ret==0) printf("Signature verified");
 

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
 // unsigned char *random_array = malloc (num_bytes);

  size_t i;

srand ((unsigned int) time (NULL));

  for (i = 0; i < num_bytes; i++)
  {
    random_array[i] = rand ();
  }

  return 0;
} 
