#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../api.h"
#include "../params.h"
#include "../randombytes.h"

#define ASGN_MLEN 32
#define ASGN_SIGNATURES 1

int main(void)
{
    int ret = 0;
    int i;

    /* Make stdout buffer more responsive. */
    setbuf(stdout, NULL);

    unsigned char pk[ASGN_PK_BYTES];
    unsigned char sk[ASGN_SK_BYTES];
    unsigned char *m = malloc(ASGN_MLEN);
    unsigned char *sm = malloc(ASGN_BYTES + ASGN_MLEN);
    unsigned char *mout = malloc(ASGN_BYTES + ASGN_MLEN);
    unsigned long long smlen;
    unsigned long long mlen;

    randombytes(m, ASGN_MLEN);

    printf("Generating keypair.. ");

    if (crypto_sign_keypair(pk, sk)) {
        printf("failed!\n");
        return -1;
    }
    printf("successful.\n");

    printf("Testing %d signatures.. \n", ASGN_SIGNATURES);

    for (i = 0; i < ASGN_SIGNATURES; i++) {
        printf("  - iteration #%d:\n", i);

        crypto_sign(sm, &smlen, m, ASGN_MLEN, sk);

        if (smlen != ASGN_BYTES + ASGN_MLEN) {
            printf("  X smlen incorrect [%llu != %u]!\n",
                   smlen, ASGN_BYTES);
            ret = -1;
        }
        else {
            printf("    smlen as expected [%llu].\n", smlen);
        }

        /* Test if signature is valid. */
        if (crypto_sign_open(mout, &mlen, sm, smlen, pk)) {
            printf("  X verification failed!\n");
            ret = -1;
        }
        else {
            printf("    verification succeeded.\n");
        }

        /* Test if the correct message was recovered. */
        if (mlen != ASGN_MLEN) {
            printf("  X mlen incorrect [%llu != %u]!\n", mlen, ASGN_MLEN);
            ret = -1;
        }
        else {
            printf("    mlen as expected [%llu].\n", mlen);
        }
        if (memcmp(m, mout, ASGN_MLEN)) {
            printf("  X output message incorrect!\n");
            ret = -1;
        }
        else {
            printf("    output message as expected.\n");
        }

        /* Test if signature is valid when validating in-place. */
        if (crypto_sign_open(sm, &mlen, sm, smlen, pk)) {
            printf("  X in-place verification failed!\n");
            ret = -1;
        }
        else {
            printf("    in-place verification succeeded.\n");
        }

        /* Test if flipping bits invalidates the signature (it should). */

        /* Flip the first bit of the message. Should invalidate. */
        sm[smlen - 1] ^= 1;
        if (!crypto_sign_open(mout, &mlen, sm, smlen, pk)) {
            printf("  X flipping a bit of m DID NOT invalidate signature!\n");
            ret = -1;
        }
        else {
            printf("    flipping a bit of m invalidates signature.\n");
        }
        sm[smlen - 1] ^= 1;

#ifdef ASGN_TEST_INVALIDSIG
        int j;
        /* Flip one bit per hash; the signature is entirely hashes. */
        for (j = 0; j < (int)(smlen - ASGN_MLEN); j += ASGN_N) {
            sm[j] ^= 1;
            if (!crypto_sign_open(mout, &mlen, sm, smlen, pk)) {
                printf("  X flipping bit %d DID NOT invalidate sig + m!\n", j);
                sm[j] ^= 1;
                ret = -1;
                break;
            }
            sm[j] ^= 1;
        }
        if (j >= (int)(smlen - ASGN_MLEN)) {
            printf("    changing any signature hash invalidates signature.\n");
        }
#endif
    }

    free(m);
    free(sm);
    free(mout);

    return ret;
}
