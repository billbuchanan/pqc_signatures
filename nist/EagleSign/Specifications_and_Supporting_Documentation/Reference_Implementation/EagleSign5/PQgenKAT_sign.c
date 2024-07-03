#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "api.h"
#include "sign.h"


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

  uint8_t sig[pq_eaglesign5_BYTES +  mlen];
  unsigned long long sig_len = sizeof(sig);

  printf("NAME: %s\n", "Eagle 5");
    printf("Private key size: %d\n", pq_eaglesign5_SECRETKEYBYTES );
    printf("Public key size: %d\n", pq_eaglesign5_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", pq_eaglesign5_BYTES );

    unsigned char sm[256 + pq_eaglesign5_BYTES];
    unsigned char m[256];
    for (unsigned i = 0; i < 256; i++) {
        m[i] = i;
    }


    unsigned char *pk = (unsigned char *)malloc( pq_eaglesign5_PUBLICKEYBYTES );
    unsigned char *sk = (unsigned char *)malloc( pq_eaglesign5_SECRETKEYBYTES );


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
  printf("\nAlice Public key (16th of key): %s\n\n",showhex(pk,pq_eaglesign5_PUBLICKEYBYTES/16));
  printf("Alice Secret key (16th of signature): %s\n\n",showhex(sk,pq_eaglesign5_SECRETKEYBYTES/16 ));

  printf("Signature (16th of signature): %s\n\n",showhex(sig,pq_eaglesign5_BYTES/16));
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
