#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "api.h"
#include "rng.h"
#include "vox_sign_core.h"

#define NBENCH 1000
#define MLEN 100
#ifndef NAN
#define NAN 0
#endif

#define BENCH_FORMAT_CLASSIC

static void print_verify_res(int res, int res1, int res2, int res3, int res4) {
    if (res1) {
        printf("  [FAIL] returned INVALID for a valid signature\n");
    }
    else if (res & 2) {
        printf("  [FAIL] crypto_sign_open returned the wrong message\n");
    }
    else {
        printf("  [OK]   returned VALID for a valid signature\n");
    }
    if (res2) {
        printf("  [OK]   returned INVALID for a modified message\n");
    }
    else {
        printf("  [FAIL] returned VALID for a modified message\n");
    }
    if (res3) {
        printf("  [OK]   returned INVALID for a modified signature\n");
    }
    else {
        printf("  [FAIL] returned VALID for a modified signature\n");
    }
    if (res4) {
        printf("  [OK]   returned INVALID for a modified padding\n");
    }
    else {
        printf("  [FAIL] returned VALID for a modified padding\n");
    }
}


typedef long long unsigned PERF_TYPE;
static inline PERF_TYPE cycles(void)
{
#ifdef __x86_64__
  PERF_TYPE result;
  __asm__ volatile("rdtsc; shlq $32,%%rdx; orq %%rdx,%%rax"
      : "=a" (result) : : "%rdx");
  return result;
#else
  return 0;
#endif /* def __x86_64__ */
}

void print_bench(double kgen, PERF_TYPE kgen_cycles, double sign, PERF_TYPE sign_cycles, double ver, PERF_TYPE ver_cycles, double sign2, PERF_TYPE sign2_cycles, double ver2, PERF_TYPE ver2_cycles)
{
  printf("\n======================================================================\n%s: Q=%u (%d bits) C=%d T=%d OC=%d VC=%d\n",CRYPTO_ALGNAME,VOX_Q,VOX_Q_BITS,VOX_C,VOX_T,VOX_OC,VOX_VC);
  printf("Benchmarks: average of %d iterations\n", NBENCH);
#ifdef BENCH_FORMAT_CLASSIC
  printf("Keygen             : %8.2fms\t%llu cycles\n", 1000*kgen / NBENCH, kgen_cycles/ NBENCH);
  printf("Sign               : %8.2fms\t%llu cycles\n", 1000*sign / NBENCH,sign_cycles/NBENCH);
  printf("Sign (cached SK)   : %8.2fms\t%llu cycles\n", 1000*sign2 / NBENCH, sign2_cycles/ NBENCH);
  printf("Verify             : %8.2fms\t%llu cycles\n", 1000*ver / NBENCH,ver_cycles/NBENCH);
  printf("Verify (cached PK) : %8.2fms\t%llu cycles\n", 1000*ver2 / NBENCH,ver2_cycles/NBENCH);
#else /* Report number of signatures/verifications per second */
  kgen  = 1000*kgen / NBENCH;
  sign  = NBENCH / sign;
  ver   = NBENCH / ver;
  sign2 = NBENCH / sign2;
  ver2  = NBENCH / ver2;
  
  printf("        |%12s|%12s|%12s|%12s|%12s\n",
	 "keygen (ms)", "sign/s", "sign/s", "verify/s", "verify/s");
  printf("        |%12s|%12s|%12s|%12s|%12s\n",
	 "", "", "(cached SK)", "", "(cached PK)");
  printf("--------");
  for (int i=0; i<5; i++)
    printf("|------------");
  printf("\n%-8s|%12.2f|%12.1f|%12.1f|%12.1f|%12.1f\n",
	 CRYPTO_ALGNAME, kgen, sign, sign2, ver, ver2);
#endif /* BENCH_FORMAT_CLASSIC ? */
}

int main() {
    int i, res, res1, res2, res3, res4, status;
    unsigned char       *pk = malloc(CRYPTO_PUBLICKEYBYTES);
    unsigned char       *sk = malloc(CRYPTO_SECRETKEYBYTES);
    unsigned char       seed[48];
    unsigned char       msg[MLEN], msg2[MLEN];
    unsigned char       sm[CRYPTO_BYTES+MLEN];
    unsigned long long  smlen, mlen;
    clock_t t;
    PERF_TYPE sign2_cycles,kgen_cycles,sign_cycles,ver_cycles,ver2_cycles;
    double kgen, sign, ver, sign2, ver2;

    for (i = 0; i < 48; i++) {
        seed[i] = i;
    }
    randombytes_init(seed, NULL, 0);
    randombytes(msg, MLEN);

    status = 0;
    printf("Test for " CRYPTO_ALGNAME "\n");
    printf("===========================\n\n");
    /******************************************************
     * Key generation
     *****************************************************/
    t = clock();
    kgen_cycles = cycles();
    for (i=0; i<NBENCH; i++)
        res = crypto_sign_keypair(pk, sk);
    kgen_cycles = cycles()-kgen_cycles;
    t = clock() - t;
    kgen = 1. * t / (CLOCKS_PER_SEC);

    if (res) {
        printf("KeyGen : KO\n");
        goto abort;
    }
    printf("KeyGen : OK\n");

    /******************************************************
     * Signature and verification with NIST API
     *****************************************************/
    t = clock();
    sign_cycles = cycles();
    for (i=0; i<NBENCH; i++) {
        /* Change the message at each iteration to average the number of retries */
        msg[0] = (unsigned char) (msg[0] + 1);
        res = crypto_sign(sm, &smlen, msg, MLEN, sk);
    }
    sign_cycles = cycles()-sign_cycles;
    t = clock() - t;
    sign = 1. * t / (CLOCKS_PER_SEC);

    if (res) {
        printf("Sign   : KO\n");
        goto abort;
    }
    printf("Sign   : OK\n");

    /* Check valid signature */
    res = 0;
    t = clock();
    ver_cycles = cycles();
    for (i=0; i<NBENCH; i++)
        res1 = crypto_sign_open(msg2, &mlen, sm, smlen, pk);
    ver_cycles = cycles()-ver_cycles;
    t = clock() - t;
    ver = 1. * t / (CLOCKS_PER_SEC);

    if (res1) {
        res = 1;
    }
    else if ( (mlen != MLEN) || memcmp(msg2, msg, MLEN) ) {
        printf("  msg  = ");
        for (i=0; i<MLEN; i++)
            printf("%02x", msg[i]);
        printf("\n  msg2 = ");
        for (unsigned long long j=0; j<mlen; j++)
            printf("%02x", msg2[j]);
        printf("\n");
        res = 2;
    }
    /* Check modifying the message */
    sm[CRYPTO_BYTES] ^= 1;
    res2 = crypto_sign_open(msg2, &mlen, sm, smlen, pk);
    if (res2 == 0)
        res |= 1;
    sm[CRYPTO_BYTES] ^= 1;
    /* Check modifying the signature */
    sm[0] ^= 1;
    res3 = crypto_sign_open(msg2, &mlen, sm, smlen, pk);
    if (res3 == 0)
        res |= 1;
    sm[0] ^= 1;
    /* Check modifying the padding */
    sm[CRYPTO_BYTES-1] ^= 128;
    res4 = crypto_sign_open(msg2, &mlen, sm, smlen, pk);
    if (res4 == 0)
        res |= 1;
    sm[CRYPTO_BYTES-1] ^= 128;
    if (res) {
        status |= 1;
        printf("Verify : KO\n");
    }
    else {
        printf("Verify : OK\n");
    }
    print_verify_res(res, res1, res2, res3, res4);

    /******************************************************
     * Signature and verification with cached keys
     *****************************************************/
    vox_cached_sk_t *SK = VOX_expand_sk(sk);
    if (SK == NULL) {
        printf("Sign (cached SK)   : Failed alloc\n");
        print_bench(kgen, 0, sign, 0, ver, 0, NAN, 0, NAN, 0);
        goto abort;
    }
    t = clock();
    sign2_cycles = cycles();
    for (i=0; i<NBENCH; i++) {
        /* Change the message at each iteration to average the number of retries */
        msg[0] = (unsigned char) (msg[0] + 1);
        res = VOX_sign_cachedSK(sm, msg, MLEN, SK);
    }
    sign2_cycles = cycles()-sign2_cycles;
    t = clock() - t;
    sign2 = 1. * t / (CLOCKS_PER_SEC);
    if (res) {
        printf("Sign (cached SK)   : KO\n");
        print_bench(kgen, 0, sign, 0, ver, 0, NAN, 0, NAN, 0);
        goto abort;
    }
    printf("Sign (cached SK)   : OK\n");
    VOX_clear_cached_sk(SK);

    vox_cached_pk_t *PK = VOX_expand_pk(pk);
    if (PK == NULL) {
        printf("Verify (cached PK) : Failed alloc\n");
        print_bench(kgen, kgen_cycles, sign, sign_cycles, ver, ver_cycles, sign2, sign2_cycles, NAN,0);
        goto abort;
    }
    /* Check valid signature */
    res = 0;
    t = clock();
    ver2_cycles= cycles();
    for (i=0; i<NBENCH; i++)
        res1 = VOX_verify_cachedPK(sm, msg, MLEN, PK);
    ver2_cycles= cycles()-ver2_cycles;
    t = clock() - t;
    ver2 = 1. * t / (CLOCKS_PER_SEC);
    if (res1)
        res = 1;
    /* Check modifying the message */
    msg[0] ^= 1;
    res2 = VOX_verify_cachedPK(sm, msg, MLEN, PK);
    if (res2 == 0)
        res |= 1;
    msg[0] ^= 1;
    /* Check modifying the signature */
    sm[0] ^= 1;
    res3 = VOX_verify_cachedPK(sm, msg, MLEN, PK);
    if (res3 == 0)
        res |= 1;
    sm[0] ^= 1;
    /* Check modifying the padding */
    sm[CRYPTO_BYTES-1] ^= 128;
    res4 = VOX_verify_cachedPK(sm, msg, MLEN, PK);
    if (res3 == 0)
        res |= 1;
    sm[CRYPTO_BYTES-1] ^= 128;
    if (res) {
        status |= 1;
        printf("Verify (cached PK) : KO\n");
    }
    else {
        printf("Verify (cached PK) : OK\n");
    }
    print_verify_res(res, res1, res2, res3, res4);
    VOX_clear_cached_pk(PK);

    if (res1)
        ver2 = NAN;
    print_bench(kgen, kgen_cycles, sign, sign_cycles, ver, ver_cycles, sign2, sign2_cycles, ver2, ver2_cycles);
    free(pk);
    free(sk);
    return status;

  abort:
    free(pk);
    free(sk);
    return 1;
}
