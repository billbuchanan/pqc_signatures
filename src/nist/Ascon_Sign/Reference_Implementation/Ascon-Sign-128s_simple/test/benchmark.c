#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../thash.h"
#include "../api.h"
#include "../fors.h"
#include "../wotsx1.h"
#include "../params.h"
#include "../randombytes.h"
#include "cycles.h"

#define ASGN_MLEN 32
#define NTESTS 10

static void wots_gen_pkx1(unsigned char *pk, const ascon_sign_ctx* ctx,
                uint32_t addr[8]);

static int cmp_llu(const void *a, const void*b)
{
  if(*(unsigned long long *)a < *(unsigned long long *)b) return -1;
  if(*(unsigned long long *)a > *(unsigned long long *)b) return 1;
  return 0;
}

static unsigned long long median(unsigned long long *l, size_t llen)
{
  qsort(l,llen,sizeof(unsigned long long),cmp_llu);

  if(llen%2) return l[llen/2];
  else return (l[llen/2-1]+l[llen/2])/2;
}

static void delta(unsigned long long *l, size_t llen)
{
    unsigned int i;
    for(i = 0; i < llen - 1; i++) {
        l[i] = l[i+1] - l[i];
    }
}


static void printfcomma (unsigned long long n)
{
    if (n < 1000) {
        printf("%llu", n);
        return;
    }
    printfcomma(n / 1000);
    printf (",%03llu", n % 1000);
}

static void printfalignedcomma (unsigned long long n, int len)
{
    unsigned long long ncopy = n;
    int i = 0;

    while (ncopy > 9) {
        len -= 1;
        ncopy /= 10;
        i += 1;  // to account for commas
    }
    i = i/3 - 1;  // to account for commas
    for (; i < len; i++) {
        printf(" ");
    }
    printfcomma(n);
}

static void display_result(double result, unsigned long long *l, size_t llen, unsigned long long mul)
{
    unsigned long long med;

    result /= NTESTS;
    delta(l, NTESTS + 1);
    med = median(l, llen);
    printf("avg. %11.2lf us (%2.2lf sec); median ", result, result / 1e6);
    printfalignedcomma(med, 12);
    printf(" cycles,  %5llux: ", mul);
    printfalignedcomma(mul*med, 12);
    printf(" cycles\n");
}

#define MEASURE_GENERIC(TEXT, MUL, FNCALL, CORR)\
    printf(TEXT);\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);\
    for(i = 0; i < NTESTS; i++) {\
        t[i] = cpucycles() / CORR;\
        FNCALL;\
    }\
    t[NTESTS] = cpucycles();\
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);\
    result = ((double)(stop.tv_sec - start.tv_sec) * 1e6 + \
        (double)(stop.tv_nsec - start.tv_nsec) / 1e3) / (double)CORR;\
    display_result(result, t, NTESTS, MUL);
#define MEASURT(TEXT, MUL, FNCALL)\
    MEASURE_GENERIC(\
        TEXT, MUL,\
        do {\
          for (int j = 0; j < 1000; j++) {\
            FNCALL;\
          }\
        } while (0);,\
    1000);
#define MEASURE(TEXT, MUL, FNCALL) MEASURE_GENERIC(TEXT, MUL, FNCALL, 1)

int main(void)
{
    /* Make stdout buffer more responsive. */
    setbuf(stdout, NULL);
    init_cpucycles();

    ascon_sign_ctx ctx;

    unsigned char pk[ASGN_PK_BYTES];
    unsigned char sk[ASGN_SK_BYTES];
    unsigned char *m = malloc(ASGN_MLEN);
    unsigned char *sm = malloc(ASGN_BYTES + ASGN_MLEN);
    unsigned char *mout = malloc(ASGN_BYTES + ASGN_MLEN);

    unsigned char fors_pk[ASGN_FORS_PK_BYTES];
    unsigned char fors_m[ASGN_FORS_MSG_BYTES];
    unsigned char fors_sig[ASGN_FORS_BYTES];
    unsigned char addr[ASGN_ADDR_BYTES];
    unsigned char block[ASGN_N];

    unsigned char wots_pk[ASGN_WOTS_PK_BYTES];

    unsigned long long smlen;
    unsigned long long mlen;
    unsigned long long t[NTESTS+1];
    struct timespec start, stop;
    double result;
    int i;

    randombytes(m, ASGN_MLEN);
    randombytes(addr, ASGN_ADDR_BYTES);

    printf("Parameters: n = %d, h = %d, d = %d, b = %d, k = %d, w = %d\n",
           ASGN_N, ASGN_FULL_HEIGHT, ASGN_D, ASGN_FORS_HEIGHT, ASGN_FORS_TREES,
           ASGN_WOTS_W);

    printf("Running %d iterations.\n", NTESTS);

    MEASURT("thash                ", 1, thash(block, block, 1, &ctx, (uint32_t*)addr));
    MEASURE("Generating keypair.. ", 1, crypto_sign_keypair(pk, sk));
    MEASURE("  - WOTS pk gen..    ", (1 << ASGN_TREE_HEIGHT), wots_gen_pkx1(wots_pk, &ctx, (uint32_t *) addr));
    MEASURE("Signing..            ", 1, crypto_sign(sm, &smlen, m, ASGN_MLEN, sk));
    MEASURE("  - FORS signing..   ", 1, fors_sign(fors_sig, fors_pk, fors_m, &ctx, (uint32_t *) addr));
    MEASURE("  - WOTS pk gen..    ", ASGN_D * (1 << ASGN_TREE_HEIGHT), wots_gen_pkx1(wots_pk, &ctx, (uint32_t *) addr));
    MEASURE("Verifying..          ", 1, crypto_sign_open(mout, &mlen, sm, smlen, pk));

    printf("Signature size: %d (%.2f KiB)\n", ASGN_BYTES, ASGN_BYTES / 1024.0);
    printf("Public key size: %d (%.2f KiB)\n", ASGN_PK_BYTES, ASGN_PK_BYTES / 1024.0);
    printf("Secret key size: %d (%.2f KiB)\n", ASGN_SK_BYTES, ASGN_SK_BYTES / 1024.0);

    free(m);
    free(sm);
    free(mout);

    return 0;
}

static void wots_gen_pkx1(unsigned char *pk, const ascon_sign_ctx *ctx,
                  uint32_t addr[8]) {
    struct leaf_info_x1 leaf;
    unsigned steps[ ASGN_WOTS_LEN ] = { 0 };
    INITIALIZE_LEAF_INFO_X1(leaf, addr, steps);
    wots_gen_leafx1(pk, ctx, 0, &leaf);
}

