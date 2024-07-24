/*
This is the main benchmarking code used for gathering performance metrics for the signature schemes. 
This script is copied over to the relevant source code location for the current algorithm/variation currently being compiled. 
Outputted binaries are stored in `bin` directory at the project's root which is created during the setup. 
The compiled binary for the current algorithm variation is stored within the relevant directory for the scheme within the `bin` directory. 
This binary is then used by the `sig-test.sh` script to automatically gather CPU cycle performance metrics for all the implemented algorithms and their variations.
*/

//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>
#include "api.h"

// Defining the randombytes function based on the api being used
int randombytes1 (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size);


#if defined(SDITH)
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
#else
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
#endif

//------------------------------------------------------------------------------
long long cpucycles(void) {
  return __rdtsc();

}

//------------------------------------------------------------------------------
char *showhex(uint8_t a[], int size) {
    char *s =malloc(size * 2 + 1);

    for (int i = 0; i < size; i++)
        sprintf(s + i * 2, "%02x", a[i]);

    return(s);

}


//------------------------------------------------------------------------------
int benchmark_cycles() {
    // Function to benchmark the key generation, signing, and verification functions of the signature scheme

    // Define message legnth and signature length variables
    int ret = 0;
    unsigned long long mlen = 32;
    //unsigned long long smlen;
    uint8_t sig[CRYPTO_BYTES +  mlen];
    unsigned long long sig_len = sizeof(sig);

    // Setting the algorithm name, based on the algorithm being used as not all schemes include this or the variation name in their API
    #if defined (SPHINCS_ALGNAME)
        printf("Algorithm: %s\n", SPHINCS_ALGNAME);
        printf("Algorithm: %s\n", CRYPTO_ALGNAME);

    #elif defined (ALTEQ_ALGNAME)
        printf("Algorithm: %s\n", ALTEQ_ALGNAME);
    
    #else
        if (strcmp(CRYPTO_ALGNAME, "lvl1") == 0) {
            printf("Algorithm: %s\n", "sqi-lvl1");
        }
        else if (strcmp(CRYPTO_ALGNAME, "lvl3") == 0) {
            printf("Algorithm: %s\n", "sqi-lvl3");
        }
        else if (strcmp(CRYPTO_ALGNAME,"lvl5") == 0) {
            printf("Algorithm: %s\n", "sqi-lvl5");
        }
        else {
            printf("Algorithm: %s\n", CRYPTO_ALGNAME);
        }
    #endif

    #if defined(PERK_ALGNAME)
        printf("Algorithm: %s\n", PERK_ALGNAME);

    #else
        printf("NAME: %s\n", CRYPTO_ALGNAME);
    #endif


    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Declare timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    /*
    Temporary comment out as not all schemes use this variable, but some may need it, will be reviewed further
    */
    //unsigned char sm[32 + CRYPTO_BYTES];
    unsigned char m[32];
    for (unsigned i = 0; i < 32; i++) {
        m[i] = i;
    }
	//printf("Signature not shown as too slow\n");
    //return(0);

    // Allocate memory for public and secret keys
    unsigned char *pk = (unsigned char *)malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char *sk = (unsigned char *)malloc(CRYPTO_SECRETKEYBYTES);

    //int r0;

    // Determine which function name to use for random number generation based on the API being used
    #if defined(SDITH)
        randombytes(m, mlen);
    #else
        randombytes1(m, mlen);
    #endif

    // Generated keypair for the signature scheme
    keygen_time = -cpucycles();
    ret = crypto_sign_keypair(pk, sk);
    keygen_time += cpucycles();

    // Verify if the key generation was successful
    if (ret == 0) {
        printf("Key generation was successful\n");
    }
    else {
        fprintf(stderr, "ERROR! Key generation failed!\n");
        exit(-1);

    }

    // Sign the message using the generated secret key
    sign_time = -cpucycles();
    ret = crypto_sign(sig, &sig_len, (const unsigned char *)m, sizeof(m), sk);
    sign_time += cpucycles();

    // Verify if the signing was successful
    if (ret == 0) {
        printf("Signing was successful\n");
    }
    else {
        fprintf(stderr, "ERROR! Signing failed!\n");
        exit(-1);

    }

    // Verify the signature using the public key
    unsigned char msg_out[17];
    unsigned long long msg_out_len = sizeof(msg_out);

    verify_time = -cpucycles();
    ret = crypto_sign_open(msg_out, &msg_out_len, sig, sizeof(sig), pk);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n");

    }
    else {
      fprintf(stderr, "\n\n   ERROR! Signature did not open!\n\n\n");
      exit(-1);

    }

    // Outputting the performance metrics gathered for the scheme
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n",  verify_time);

    // Temporaily output the message and key values for debugging purposes
    // printf("\nMessage: %s\n",showhex(m, mlen));
    // printf("\nAlice Public key (16th of key): %s\n\n",showhex(pk,CRYPTO_PUBLICKEYBYTES/16));
    // printf("Alice Secret key (128th of signature): %s\n\n",showhex(sk,CRYPTO_SECRETKEYBYTES/128 ));
    // printf("Signature (128th of signature): %s\n\n",showhex(sig,CRYPTO_BYTES/128));

    // Free the allocated memory for the public and secret keys
    free(pk);
    free(sk);
    
    return 0;
  
}

//------------------------------------------------------------------------------
int main(void) {

    // Calling cpu cycles benchmarking function
    if (benchmark_cycles() != 0) {
        printf("Benchmarking failed\n");
        return -1;
    }

    return 0;

}

