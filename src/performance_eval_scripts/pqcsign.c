/*
This is the main benchmarking code used for gathering performance metrics for the signature schemes. 
This script is copied over to the relevant source code location for the current algorithm/variation currently being compiled. 
Outputted binaries are stored in `bin` directory at the project's root which is created during the setup. 
The compiled binary for the current algorithm variation is stored within the relevant directory for the scheme within the `bin` directory. 
This binary is then used by the `sig-test.sh` script to automatically gather CPU cycle performance metrics for all the implemented algorithms and their variations.
*
The script also makes use of preprocessor directives to determine what testing functions and includes are needed for the current algorithm being tested.
*/

//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>
#include <ctype.h>

//Determine what OS is being used to determine which include lib is needed for stack resizing
#if __unix
    #include <sys/resource.h>
#endif

#if _WIN32
    #error "Stack resizing on Windows not implemented yet!"
#endif

// Include the relevant header files based on the signature scheme being tested
#if defined (EAGLESIGN_MODE)
    #include "api.h"
    #include "sign.h"

#elif defined (KAZ_ALGNAME)

    #include "api.h"
    #include "kaz_api.h"

#elif defined (MIRITH)

    #include "../api.h"
    #include "../random.h"
    #include "../sign.h"

#elif defined (PREON)

    #include "api.h"
    #include "rng.h"

#elif defined (SQUIRRELS_LEVEL)

    #include "../../KAT/generator/katrng.h"
    #include "api.h"
    #include "inner.h"

#elif defined (WAVE)

    #include "api.h"
    #include "prng/prng.h"
    //#include "NIST-kat/rng.h"

# else
    #include "api.h"
#endif

//------------------------------------------------------------------------------
// Defining the randombytes function based on the api being used
int randombytes1 (unsigned char* random_array, unsigned long long num_bytes);
char *showhex(uint8_t a[], int size);

// Determine which randombytes function to use based on the scheme being tested
#if defined (MQOM_API_H) || defined(SDITH)
int randombytes (unsigned char* random_array, unsigned long long num_bytes) {
    // unsigned char *random_array = malloc (num_bytes);
    size_t i;
    srand ((unsigned int) time (NULL));

    for (i = 0; i < num_bytes; i++) {
        random_array[i] = rand ();
    }

    return 0;

}
#else
int randombytes1 (unsigned char* random_array, unsigned long long num_bytes) {

    // unsigned char *random_array = malloc (num_bytes);
    size_t i;
    srand ((unsigned int) time (NULL));

    for (i = 0; i < num_bytes; i++) {
        random_array[i] = rand ();
    }

    return 0;

}

#endif

//------------------------------------------------------------------------------
long long cpucycles(void) {
    // Function to get the current CPU cycle count
    return __rdtsc();

}

//------------------------------------------------------------------------------
char *showhex(uint8_t a[], int size) {
    // Function to convert an array of bytes to a hex string

    char *s =malloc(size * 2 + 1);

    for (int i = 0; i < size; i++) {
        sprintf(s + i * 2, "%02x", a[i]);
    }

    return(s);

}

//------------------------------------------------------------------------------
void lower(char *string) {
    // Function to convert a string to lowercase

    for (int i = 0; string[i]; i++) {
        string[i] = tolower(string[i]);
    }

}

//------------------------------------------------------------------------------
void set_macros(char *alg_name) {
    // Function to set the environment macros for the signature scheme being tested as API usage varies between schemes

    // Set the macro for special cases or schemes that require different API usage and set the algorithm name
    #if defined (AIMER_NAME)
        strcpy(alg_name, AIMER_NAME);

    #elif defined (ALTEQ_ALGNAME)
        strcpy(alg_name, ALTEQ_ALGNAME);

    #elif (EAGLESIGN_MODE == 3)

        // Copy and define NIST API macros due to EagleSign's drastic difference in API usage
        #define CRYPTO_BYTES pq_eaglesign3_BYTES
        #define CRYPTO_SECRETKEYBYTES pq_eaglesign3_SECRETKEYBYTES
        #define CRYPTO_PUBLICKEYBYTES pq_eaglesign3_PUBLICKEYBYTES
        strcpy(alg_name, "EagleSign3");

    #elif (EAGLESIGN_MODE == 5)

        // Copy and define NIST API macros due to EagleSign's drastic difference in API usage     
        #define CRYPTO_BYTES pq_eaglesign5_BYTES
        #define CRYPTO_SECRETKEYBYTES pq_eaglesign5_SECRETKEYBYTES
        #define CRYPTO_PUBLICKEYBYTES pq_eaglesign5_PUBLICKEYBYTES
        strcpy(alg_name, "EagleSign5");

    #elif defined (KAZ_ALGNAME)
        
        // Determine which security level is being used for the KAZ_SIGN algorithm
        if (KAZ_ALGNAME == 1) {
            strcpy(alg_name, "kaz458");
        }
        else if (KAZ_ALGNAME == 3) {
            strcpy(alg_name, "kaz738");
        }
        else if (KAZ_ALGNAME == 5) {
            strcpy(alg_name, "kaz970");
        }
        else {
            printf("error in getting the KAZ_SIGN algorithm security level\n");
            exit(1);
        }

    #elif defined (LESS_ALGNAME)
        strcpy(alg_name, LESS_ALGNAME);
    
    #elif defined (MIRITH)
        strcpy(alg_name, MIRITH_ALGNAME);

    #elif defined(PERK_ALGNAME)
        strcpy(alg_name, PERK_ALGNAME);

    #elif defined (RACCOON)
        strcpy(alg_name, CRYPTO_ALGNAME);
        lower(alg_name);

    #elif defined (SNOVA_ALGNAME)
        strcpy(alg_name, SNOVA_ALGNAME);

    #elif defined (SPHINCS_ALGNAME)
        strcpy(alg_name, SPHINCS_ALGNAME);

    #elif defined (SQI_FLAG)

        // Determine which security level is being used for the SQI algorithm and add sqi prefix
        if (strcmp(CRYPTO_ALGNAME, "lvl1") == 0) {
            strcpy(alg_name, "sqi-lvl1");
        }
        else if (strcmp(CRYPTO_ALGNAME, "lvl3") == 0) {
            strcpy(alg_name, "sqi-lvl3");
        }
        else if (strcmp(CRYPTO_ALGNAME,"lvl5") == 0) {
            strcpy(alg_name, "sqi-lvl5");
        }
        else {
            printf("SQI defined, but not able to get the variation\n");
            exit(1);
        }

    #elif defined (WAVE)
        
        // Get the variation on convert to follow the naming convention in alg_variation_lists for Wave
        if (strcmp(CRYPTO_ALGNAME, "WAVE-822") == 0) {
            strcpy(alg_name, "Wave822");
        }
        else if (strcmp(CRYPTO_ALGNAME, "WAVE-1249") == 0) {
            strcpy(alg_name, "Wave1249");
        }
        else if (strcmp(CRYPTO_ALGNAME, "WAVE-1644") == 0) {
            strcpy(alg_name, "Wave1644");
        }
        else {
            printf("Wave defined, but not able to get the variation\n");
            strcpy(alg_name, CRYPTO_ALGNAME);
        }

    #else
        // Use the default NIST API macros and set the algorithm name
        strcpy(alg_name, CRYPTO_ALGNAME);
    
    #endif

    // Output the algorithm name to the console
    printf("Algorithm: %s\n", alg_name);
    
}

//------------------------------------------------------------------------------
int increase_stack_size(char *alg_name) {
    // Function to increase the stack size for the signature scheme being tested if needed

    // Determine if the stack size needs to be increased for the signature scheme
    char large_stack_schemes[12][40] = {
        "mirith_hypercube_Ia_shortest",
        "mirith_hypercube_Ib_shortest",
        "mirith_hypercube_IIIa_shortest",
        "mirith_hypercube_IIIb_shortest",
        "mirith_hypercube_Va_shortest",
        "mirith_hypercube_Vb_shortest",
        "mirith_Ib_short",
        "mirith_IIIb_short",
        "mirith_Vb_short",
        "perk-256-short-3",
        "perk-256-short-5",
        "Wave1644"
    };

    // Determine the number of elements in the array
    int num_elements = sizeof(large_stack_schemes) / sizeof(large_stack_schemes[0]);

    // Check if the current algorithm is in the list of algorithms that require a larger stack size
    for (int index = 0; index < num_elements; index++) {

        // Compare alg name with current index in the array
        if (strcmp(alg_name, large_stack_schemes[index]) == 0) {

            // Increase the stack size for the signature scheme using code borrowed from MiRitH reference implementation
            #if __unix

                struct rlimit rl;

                /* Increase stack size to 128 MiB. */
                getrlimit(RLIMIT_STACK, &rl);


                rl.rlim_cur = 128 * 1024 * 1024; /* 128 MiB. */
                //rl.rlim_cur = (rlim_t)8192 * 1024 * 1024; /* 128 MiB. */
                
                if (setrlimit(RLIMIT_STACK, &rl) != 0)
                {
                    printf("Error: Cannot increase stack size!\n");
                    return -1;
                }
                else {
                    printf("Stack size increased\n");
                    return 0;
                }

            #elif _WIN32
                printf("Unable to increase stack size on Windows\n");
                return -1;

            #endif

        }

    }

    return 0;
}

//------------------------------------------------------------------------------
void setup_env() {
    // Function to setup the environment for the signature scheme being tested

    // Allocate memory for the algorithm name and verify if the memory allocation was successful
    char *alg_name = malloc(40);

    if ( alg_name == NULL ) {
        printf("Error: Memory allocation failed\n");
        exit(1);
    }

    // Set the environment macros for the signature scheme and change stack size if needed
    set_macros(alg_name);
    int ret = increase_stack_size(alg_name);
    
    // Check if the stack size was successfully increased
    if (ret != 0) {
        printf("Error: Stack size could not be increased and needed to be for the scheme\n");
        exit(1);
    }

    // Free the allocated memory for the algorithm name array
    free(alg_name);

}

//------------------------------------------------------------------------------
#if defined (KAZ_ALGNAME)

int kaz_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function kaz_sign algorithm as it API differs enough to require a separate function.
    */

    // Define return flag variables
    int ret = 0;

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
#elif defined (MQOM_API_H) || defined(SDITH_THRESHOLD)

int mq_sd_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is for the MiRitH and SDITH_Threshold algorithm as their API differs 
    *  enough to require a separate function but the two schemes have a similar bench.c file in their reference implementations.
    */

    // Define return flag variable
    int ret = 0;

    // Define message length and signature length variables
    #define MLEN 32
    uint8_t m[MLEN] = {1, 2, 3, 4};
    uint8_t m2[MLEN] = {0};
    unsigned long long m2len = 0;
    uint8_t sm[MLEN + CRYPTO_BYTES];
    unsigned long long smlen = 0;

    // Define public and secret key variables
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];

    // Define timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the secret key size, public key size, and signature size
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

    // Sign the message using the generated secret key
    sign_time = -cpucycles();
    ret = crypto_sign(sm, &smlen, m, MLEN, sk);
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
    ret = crypto_sign_open(m2, &m2len, sm, smlen, pk);
    verify_time += cpucycles();

    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    /* NOTE - Extra verification steps taken from benchmark.c in the reference implementation */
    // Test of correction of the primitives
    if(m2len != MLEN) {
        printf("Failure (num %d): message size does not match\n");
    }

    for(int h=0; h<MLEN; h++) {

        if(m[h] != m2[h]) {
            printf("Failure (num %d): message does not match (char %d)\n", h);
        }

    }

    // Outputting the performance metrics gathered for the scheme
    printf("CPU Cycles Performance Metrics:\n");
    printf("Keygen:\t%llu cycles\n",  keygen_time);
    printf("Sign:\t%llu cycles\n",  sign_time);
    printf("Verify:\t%llu cycles\n\n",  verify_time);

    return 0;

}

//------------------------------------------------------------------------------
#elif defined (PREON)

int preon_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is for the MiRitH algorithm as it API differs enough to require a separate function.
    *  The code closely follows some of the functionality in thePQCGenKAT.c code as this method does not cause segmentation 
    *  issues that occur when using the standard benchmark_cycles function.
    */

    // Define the return flag variable
    int ret = 0;

    // Define message length and signature length variables
    unsigned char seed[48];
    unsigned char entropy_input[48];
    unsigned long long mlen = 3300; 
    unsigned long long smlen, mlen1;

    // Define and allocate memory for the message, signature, public and secret keys
    unsigned char *m = (unsigned char *)malloc(mlen);
    unsigned char *sm = (unsigned char *)malloc(mlen + CRYPTO_BYTES);
    unsigned char *m1 = (unsigned char *)malloc(mlen + CRYPTO_BYTES);
    unsigned char *pk = (unsigned char *)malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char *sk = (unsigned char *)malloc(CRYPTO_SECRETKEYBYTES);

    // Define timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Verify if memory allocation was successful
    if (!m || !sm || !m1 || !pk || !sk) {
        printf("Memory allocation of core scheme variables failed\n");
        free(m);
        free(sm);
        free(m1);
        free(pk);
        free(sk);
        return -1;
    }

    // Outputting the algorithm name, secret key size, public key size, and signature size
    printf("Private key size: %d\n", CRYPTO_SECRETKEYBYTES);
    printf("Public key size: %d\n", CRYPTO_PUBLICKEYBYTES );
    printf("Signature size: %d\n\n", CRYPTO_BYTES );

    // Initialize entropy input and seed for the rng function
    for (int i = 0; i < 48; i++) {
        entropy_input[i] = i;
    }
    randombytes_init(entropy_input, NULL, 256);
    randombytes(m, mlen);

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
    ret = crypto_sign(sm, &smlen, m, mlen, sk);
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
    ret = crypto_sign_open(m1, &mlen1, sm, smlen, pk);
    verify_time += cpucycles();


    // Verify if the signature was successfully verified
    if (ret == 0) {
        printf("Signature verified\n\n");
    }
    else {
      fprintf(stderr, "ERROR! Signature did not open!\n\n\n");
      exit(-1);
    }

    // Output performance metrics
    printf("Performance Metrics:\n");
    printf("Keygen: %lld cycles\n", keygen_time);
    printf("Sign: %lld cycles\n", sign_time);
    printf("Verify: %lld cycles\n", verify_time);

    // Free the allocated memory for the message, signature, public and secret keys
    free(m);
    free(sm);
    free(m1);
    free(pk);
    free(sk);

    return 0;

}

//------------------------------------------------------------------------------
#elif defined (SQUIRRELS_LEVEL)

int squirrels_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is for the Squirrels algorithm as it API differs enough to require a separate function.
    *  The code closely follows the benchmarks.c file in the reference implementation in order to function correctly.
    */

    // Define return flag variables
    int ret = 0;

    // // Initialise rng for the signature scheme
    // NOTE - RANDOM NUMBER GENERATOR NEEDS CHECKED AS BORROWED FROM benchmark.c IN REFERENCE IMPLEMENTATION
    // BUT PQCGenKAT_sign.c DOESN'T OPERATE IN THE SAME WAY

    //Benchmarking.c does it this way
    // unsigned char seed[48];
    // static unsigned char entropy_input[48];
    // inner_shake256_context rng;
    // randombytes(seed, sizeof seed);
    // inner_shake256_init(&rng);
    // inner_shake256_inject(&rng, seed, sizeof seed);
    // inner_shake256_flip(&rng);

    // for (int i=0; i<48; i++)
    //     entropy_input[i] = i;

    // randombytes_init(entropy_input, NULL, 256);

    // Define message length and signature length variables
    unsigned char m[3300]; //m value taken from PQCgenKAT_sign.c
    unsigned char sig[CRYPTO_BYTES];
    // unsigned char m[] = "test";
    unsigned long long siglen, mlen = sizeof(m);

    // Create random message
    randombytes(m, sizeof(m));
    //randombytes1(m, sizeof(m));
    
    // Defined and allocate memory for the public and secret keys
    unsigned char *pk = malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char *sk = malloc(CRYPTO_SECRETKEYBYTES);

    // Define timing variables used for benchmarking
    long long keygen_time = 0;
    long long sign_time = 0;
    long long verify_time = 0;

    // Outputting the secret key size, public key size, and signature size
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

    // Sign the message using the generated secret key
    sign_time = -cpucycles();
    crypto_sign(sig, &siglen, m, sizeof(m), sk);
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
    crypto_sign(sig, &siglen, m, mlen, sk);
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

    // // Free the allocated memory for the public and secret keys
    free(pk);
    free(sk);

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
int standard_benchmark_cycles() {
    /* Function to benchmark the key generation, signing, and verification functions of the signature scheme
    *  being tested.This function is used for all signature schemes that use the standard API.
    */

    // Define return flag for the cryptographic operations
    int ret = 0;

    // // Define message length and signature length variables
    unsigned char m[256];
    //unsigned char sm[256 + CRYPTO_BYTES];
    unsigned long long mlen = sizeof(m);

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

    // Determine which function name to use for random message generation
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
    // Main function to control the benchmarking of the signature schemes

    // Setup the benchmarking environment for the signature scheme being tested
    setup_env();

    // Call the relevant benchmarking function based on the signature scheme being tested
    #if defined(KAZ_ALGNAME)

        // Calling CPU cycles benchmarking function for the Kaz signature scheme
        if (kaz_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined(MIRITH)
    
        // Calling CPU cycles benchmarking function for the MiRitH signature scheme
        if (mirith_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined (MQOM_API_H) || defined (SDITH_THRESHOLD)

        // Calling CPU cycles benchmarking function for the MQOM or SDITH_Threshold signature scheme
        if (mq_sd_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined (PREON)

        // Calling CPU cycles benchmarking function for the Preon signature scheme
        if (preon_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #elif defined (SQUIRRELS_LEVEL)

        // Calling CPU cycles benchmarking function for the Squirrels signature scheme
        if (squirrels_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    
    #elif defined(WAVE)

        // Calling CPU cycles benchmarking function for the Wave signature scheme
        if (wave_benchmark_cycles() != 0) {
            printf("Benchmarking failed\n");
            return -1;
        }

    #else

        // Calling cpu cycles benchmarking function for all other algorithms
        if (standard_benchmark_cycles() != 0) {
            printf("Benchmarking failed");
            return -1;
        }

    #endif

    return 0;

}