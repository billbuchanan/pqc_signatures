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

// Include the relevant header files based on the signature scheme being tested
# if defined (MIRITH)

    // Define relevant header files for the MiRitH signature scheme
    #include "../api.h"
    #include "../random.h"
    #include "../sign.h"

    //Determine what OS is being used to include lib that allows for stack resizing
    #if __unix
        #include <sys/resource.h>
    #endif

    #if _WIN32
        #error "Stack resizing on Windows not implemented yet!"
    #endif

#elif defined (WAVE)
    #include "api.h"
    #include "prng/prng.h"
    //#include "NIST-kat/rng.h"

#elif defined (KAZ_ALGNAME)
    #include "api.h"
    #include "kaz_api.h"
# else
    #include "api.h"

#endif

#if defined (EAGLESIGN_MODE)
    #include "sign.h"
#endif

//------------------------------------------------------------------------------
// Defining the randombytes function based on the api being used
int randombytes1 (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size);


#if defined(SDITH) || defined (MQOM_API_H) || defined(SQUIRRELS_LEVEL)
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

    for (int i = 0; i < size; i++) {
        sprintf(s + i * 2, "%02x", a[i]);
    }

    return(s);

}

//------------------------------------------------------------------------------
void set_env_macros() {
    // Function to set the environment macros for the signature scheme being tested as API usage varies between schemes

    // Copy and define macros if the signature schemes is EagleSign due to its drastic difference in API usage
    #if (EAGLESIGN_MODE == 3)
        printf("Algorithm: %s\n", "EagleSign3");
        #define CRYPTO_BYTES pq_eaglesign3_BYTES
        #define CRYPTO_SECRETKEYBYTES pq_eaglesign3_SECRETKEYBYTES
        #define CRYPTO_PUBLICKEYBYTES pq_eaglesign3_PUBLICKEYBYTES

    #elif (EAGLESIGN_MODE == 5)
        printf("Algorithm: %s\n", "EagleSign5");
        #define CRYPTO_BYTES pq_eaglesign5_BYTES
        #define CRYPTO_SECRETKEYBYTES pq_eaglesign5_SECRETKEYBYTES
        #define CRYPTO_PUBLICKEYBYTES pq_eaglesign5_PUBLICKEYBYTES

    #elif defined (SPHINCS_ALGNAME)
        printf("Algorithm: %s\n", SPHINCS_ALGNAME);
        printf("Algorithm: %s\n", CRYPTO_ALGNAME);

    #elif defined (ALTEQ_ALGNAME)
        printf("Algorithm: %s\n", ALTEQ_ALGNAME);
    
    #elif defined (AIMER_NAME)
        printf("Algorithm: %s\n", AIMER_NAME);

    #elif defined(PERK_ALGNAME)
        printf("Algorithm: %s\n", PERK_ALGNAME);

    #elif defined (KAZ_ALGNAME)
        
        // Determine which security level is being used for the algorithm
        if (KAZ_ALGNAME == 1) {
            printf("Algorithm: %s\n", "kaz458");
        }
        else if (KAZ_ALGNAME == 3) {
            printf("Algorithm: %s\n", "kaz738");
        }
        else if (KAZ_ALGNAME == 5) {
            printf("Algorithm: %s\n", "kaz970");
        }
        else {
            printf("error in getting the KAZ_SIGN algorithm security level\n");
            exit(1);
        }

    #elif defined (MIRITH)
        printf("Algorithm: %s\n", MIRITH_ALGNAME);

    #else

        // Check if the algorithm is SQI and get the variation, otherwise output the default algorithm name
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
    
}

//------------------------------------------------------------------------------
#if defined (KAZ_ALGNAME)

int kaz_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function kaz_sign algorithm as it API differs enough to require a separate function.
    */

    // Define return flag variables
    int ret = 0;
    // int r0;
    // int r1;
    // int r2;

    // Define message length and signature length variables
    unsigned char m[KAZ_DS_SALTBYTES + KAZ_DS_S1BYTES + KAZ_DS_S2BYTES];
    unsigned long long mlen = KAZ_DS_SALTBYTES + KAZ_DS_S1BYTES + KAZ_DS_S2BYTES;
    unsigned long long smlen;

    for (unsigned i = 0; i < KAZ_DS_SALTBYTES + KAZ_DS_S1BYTES + KAZ_DS_S2BYTES; i++) {
        m[i] = i;
    }

    // Define and allocate memory for public and secret keys
    unsigned char *pk, *sk;
    pk = (unsigned char *)calloc(CRYPTO_PUBLICKEYBYTES, sizeof(unsigned char));
    sk = (unsigned char *)calloc(CRYPTO_SECRETKEYBYTES, sizeof(unsigned char));

    // Define timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("NAME: %s\n", CRYPTO_ALGNAME);
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES);
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Outputting that the cryptographic operations are being benchmarked
    printf("Performing cryptographic operations:\n");

    // Generate keypair for the signature scheme
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

    // if ( 0 != r0 ) {
    //     printf("generating key return %d.\n", r0);
    //     return -1;
    // }

    // Sign the message using the generated secret key
    randombytes1(m, mlen);
    unsigned char sm[mlen + KAZ_DS_SALTBYTES + KAZ_DS_S1BYTES + KAZ_DS_S2BYTES];

    sign_time = -cpucycles();
    ret = crypto_sign( sm, &smlen, m, mlen, sk);
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
    verify_time = -cpucycles();
    ret = crypto_sign_open(m, &mlen, sm, smlen, pk);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    // Outputting the performance metrics gathered for the scheme
    printf("CPU Cycles Performance Metrics:\n");
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n\n",  verify_time);

    // Free the allocated memory for the public and secret keys
    free(pk);
    free(sk);
    
    return 0;

}

//------------------------------------------------------------------------------
#elif defined (MIRITH)

int mirith_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is for the MiRitH algorithm as it API differs enough to require a separate function.
    *  The code closely follows the test_mirith.c file in the reference implementation in order to function correctly.
    */

    /* From test_mirith.c - Some parameter sets requires a stack larger than 8 MiB. */
    #if MIRITH_MODE == 3 || MIRITH_MODE == 7 || MIRITH_MODE == 11 \
        || MIRITH_MODE == 15 || MIRITH_MODE == 19 || MIRITH_MODE == 23

    #if __unix

        struct rlimit rl;

        /* Increase stack size to 128 MiB. */
        getrlimit(RLIMIT_STACK, &rl);

        rl.rlim_cur = 128 * 1024 * 1024; /* 128 MiB. */
        
        if (setrlimit(RLIMIT_STACK, &rl) != 0)
        {
            printf("Error: Cannot increase stack size!\n");
            return -1;
        }
        /* * */
    #endif

    #if _WIN32
    #error "Stack resizing on Windows not implemented yet!"
    #endif

    #endif

    // Define return flag variables
    int ret = 0;

    // Declare Message length and signature length variables
    int MSG_LEN = 80;
    uint8_t msg[MSG_LEN];
    uint8_t msg2[MSG_LEN];
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t sig_msg[CRYPTO_BYTES + MSG_LEN];
    size_t sig_msg_len;
    size_t msg_len = MSG_LEN;
    size_t msg2_len;
    size_t pos;
    uint8_t byte;

    // Define timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Outputting that the cryptographic operations are being benchmarked
    printf("Performing cryptographic operations:\n");

    // Create a random message
    randombytes(msg, MSG_LEN);

    // // Generate keypair for the signature scheme
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
    ret = crypto_sign(sig_msg, &sig_msg_len, msg, MSG_LEN, sk);
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
    verify_time = -cpucycles();
    ret = crypto_sign_open(msg2, &msg2_len, sig_msg, sig_msg_len, pk);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    // Outputting the performance metrics gathered for the scheme
    printf("CPU Cycles Performance Metrics:\n");
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n\n",  verify_time);

    // Free the allocated memory for the public and secret keys
    // free(pk);
    // free(sk);
    
    return 0;


}

//------------------------------------------------------------------------------
#elif defined (WAVE)

int wave_benchmark_cycles() {

    // Define return flag for the cryptographic operations
    int ret = 0;

    // Define message length and signature length variables
    #define MSG_SIZE 32
    #define SEED_SIZE 32

    uint8_t seed[SEED_SIZE];
    memset(seed, 0, SEED_SIZE);
    seed_prng(seed, SEED_SIZE);

    unsigned long long mlen = 32;
    unsigned long long output_sig = 0;
    uint8_t PK[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t SK[CRYPTO_SECRETKEYBYTES] = {0};
    uint8_t sig[CRYPTO_BYTES + MSG_SIZE] = {0};

    // Declare timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Outputting that the cryptographic operations are being benchmarked
    printf("Performing cryptographic operations:\n");

    // Generate random message
    uint8_t msg[MSG_SIZE] = {0};
    rng_bytes(msg, MSG_SIZE);

    // Generate keypair for the signature scheme
    keygen_time = -cpucycles();
    ret = crypto_sign_keypair(PK, SK);
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
    ret = crypto_sign(sig, &output_sig, msg, MSG_SIZE, SK);
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
    verify_time = -cpucycles();
    ret = crypto_sign_open(msg, &mlen, sig, output_sig, PK);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    // Outputting the performance metrics gathered for the scheme
    printf("CPU Cycles Performance Metrics:\n");
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n\n",  verify_time);
    
    return 0;


}

//------------------------------------------------------------------------------
#else
int benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is used for all signature schemes that use the standard API.
    */

    // Define return flag for the cryptographic operations
    int ret = 0;

    // Define message length and signature length variables
    unsigned char m[256];
    //unsigned char sm[256 + CRYPTO_BYTES];
    unsigned long long mlen = 256;

    uint8_t sm[CRYPTO_BYTES +  mlen];
    unsigned long long smlen = sizeof(sm);

    for (unsigned i = 0; i < 256; i++) {
        m[i] = i;
    }

    // Allocate memory for public and secret keys
    unsigned char *pk = (unsigned char *)malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char *sk = (unsigned char *)malloc(CRYPTO_SECRETKEYBYTES);

    // Declare timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES );
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Determine which function name to use for random number generation based on the API being used
    #if defined(SDITH) || defined (MQOM_API_H) || defined(SQUIRRELS_LEVEL) || defined (WAVE)
        randombytes(m, mlen);
    #else
        randombytes1(m, mlen);
    #endif

    // Outputting that the cryptographic operations are being benchmarked
    printf("Performing cryptographic operations:\n");

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
    ret = crypto_sign(sm, &smlen, (const unsigned char *)m, sizeof(m), sk);
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
    unsigned char msg_out[256];
    unsigned long long msg_out_len = sizeof(msg_out);

    verify_time = -cpucycles();
    ret = crypto_sign_open(msg_out, &msg_out_len, sm, sizeof(sm), pk);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    // Outputting the performance metrics gathered for the scheme
    printf("CPU Cycles Performance Metrics:\n");
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n\n",  verify_time);

    // Free the allocated memory for the public and secret keys
    free(pk);
    free(sk);
    
    return 0;
  
}
#endif

//------------------------------------------------------------------------------
int main(void) {

    // Setting the environment macros for the signature scheme
    set_env_macros();

    #if defined(WAVE)
        if (wave_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined(KAZ_ALGNAME)

        if (kaz_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined(MIRITH)
    
        if (mirith_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #else

        // Calling cpu cycles benchmarking function for all other algorithms
        if (benchmark_cycles() != 0) {
            printf("Benchmarking failed");
            return -1;
        }

    #endif

    return 0;

}