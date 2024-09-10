#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "randombytes.h"
#include "api.h"



inline static uint64_t cpucyclesStart (void) {
    unsigned hi, lo;
    __asm__ __volatile__ (	"CPUID\n\t"
                "RDTSC\n\t"
                "mov %%edx, %0\n\t"
                "mov %%eax, %1\n\t"
                : "=r" (hi), "=r" (lo)
                :
                : "%rax", "%rbx", "%rcx", "%rdx");

    return ((uint64_t) lo) ^ (((uint64_t) hi) << 32);
}



inline static uint64_t cpucyclesStop (void) {
    unsigned hi, lo;
    __asm__ __volatile__(	"RDTSCP\n\t"
                "mov %%edx, %0\n\t"
                "mov %%eax, %1\n\t"
                "CPUID\n\t"
                : "=r" (hi), "=r" (lo)
                :
                : "%rax", "%rbx", "%rcx", "%rdx");

    return ((uint64_t) lo) ^ (((uint64_t) hi) << 32);
}



int main(void) {

    unsigned long long mira128f_mlen = 7;
    unsigned long long mira128f_smlen;

    unsigned char mira128f_m[] = {0x4d, 0x49, 0x6e, 0x52, 0x41, 0x6e, 0x6b};

    unsigned char mira128f_pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char mira128f_sk[CRYPTO_SECRETKEYBYTES];
    unsigned char mira128f_sm[CRYPTO_BYTES + mira128f_mlen];

    unsigned long long t1, t2, t3, t4, t5, t6;



    unsigned char seed[48] = {0};
//    (void)syscall(SYS_getrandom, seed, 48, 0);
    randombytes_init(seed, NULL, 256);



    /*************/
    /* MIRA-128F */
    /*************/



    t1 = cpucyclesStart();
    if (crypto_sign_keypair(mira128f_pk, mira128f_sk) == -1) {
        printf("\nnFailed\n\n");
        return -1;
    }
    t2 = cpucyclesStop();

    t3 = cpucyclesStart();
    if (crypto_sign(mira128f_sm, &mira128f_smlen, mira128f_m, mira128f_mlen, mira128f_sk) != 0) {
        printf("\nnFailed\n\n");
        return -1;
    };
    t4 = cpucyclesStart();

    t5 = cpucyclesStart();
    if (crypto_sign_open(mira128f_m, &mira128f_mlen, mira128f_sm, mira128f_smlen, mira128f_pk) == -1) {
        printf("\nnFailed\n\n");
        return -1;
    }
    t6 = cpucyclesStart();



    #ifndef VERBOSE
    printf("\n MIRA-128F");
    printf("\n  crypto_sign_keypair: %lld CPU cycles", t2 - t1);
    printf("\n  crypto_sign:         %lld CPU cycles", t4 - t3);
    printf("\n  crypto_sign_open:    %lld CPU cycles", t6 - t5);
    printf("\n\n");

    printf("\n sk: "); for(int k = 0 ; k < CRYPTO_SECRETKEYBYTES ; ++k) printf("%02x", mira128f_sk[k]);
    printf("\n pk: "); for(int k = 0 ; k < CRYPTO_PUBLICKEYBYTES ; ++k) printf("%02x", mira128f_pk[k]);
    printf("\n  m: "); for(int k = 0 ; k < (int)mira128f_mlen ; ++k) printf("%02x", mira128f_m[k]);
    #endif

    // To avoid warning in VERBOSE mode
    t1 -= t1;
    t2 -= t2;
    t3 -= t3;
    t4 -= t4;
    t5 -= t5;
    t6 -= t6;

    printf("\n\n");
    return 0;
}

