#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <emmintrin.h>
#include <math.h>
#include "../api.h"
#include "../config.h"
#include "../packing.h"
#include "../random.h"
#include "../sign.h"

#include "../internal_get_cycles.h"

#if __unix
#include <sys/resource.h>
#endif

#if _WIN32
#error "Stack resizing on Windows not implemented yet!"
#endif

uint32_t N_BENCH = 1024;     /* Number of tests. */
uint32_t MSG_LEN = 80;       /* Message length. */

/* This goes outside of 'main' to avoid stack overflows. */
int bench_mirith()
{
    int i;
    uint64_t cc_keyg[N_BENCH];
    uint64_t cc_sign[N_BENCH];
    uint64_t cc_verf[N_BENCH];
    uint64_t begin, end;

#if defined (OFFLINE_CC)
    uint64_t cc_off[N_BENCH];
#endif
    
    // printf("\n\nBenchmarks...\n\n");
    
    // First N iteractions warm the processor in order to get a stable measurement
    for (i = 0; i < N_BENCH * 2; i++)
    {
        uint8_t msg[MSG_LEN];
        uint8_t msg2[MSG_LEN];
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];
        uint8_t sig_msg[CRYPTO_BYTES + MSG_LEN];
        size_t sig_msg_len;
        size_t msg2_len;

        /* Unpacked public key. */
        ff_t M[PAR_K + 1][matrix_bytes_size(PAR_M, PAR_N)];
    
        /* Unpacked secret key. */
        ff_t a[matrix_bytes_size(PAR_K, 1)];
        ff_t K[matrix_bytes_size(PAR_R, PAR_N - PAR_R)];
        ff_t E[matrix_bytes_size(PAR_M, PAR_N)];
        
        /* Generate a random message. */
        randombytes(msg, MSG_LEN);
        
        begin = get_cycles();
        {
            /* Generate public and secret key. */
            crypto_sign_keypair(pk, sk);
        }
        end = get_cycles();

        if (i >= N_BENCH)
        {
            cc_keyg[i - N_BENCH] = end - begin;
        }

        /* Unpack the secret key (and the public key).
         * NOTE: The time to unpack the keys is not taken into account
         * in the benchmark for signing, since it is reasonable to
         * assume that a signer works with unpacked keys. */
        unpack_secret_key(M, a, K, E, sk);
        
#if defined (OFFLINE_CC)
    offline_cc = 0;
#endif

        begin = get_cycles();
        {
            /* Sign the message. */
            crypto_sign(sig_msg, &sig_msg_len, msg, MSG_LEN, sk);
        }
        end = get_cycles();

        if (i >= N_BENCH)
        {

#if defined (OFFLINE_CC)
        cc_off[i - N_BENCH] = offline_cc;
#endif

            cc_sign[i - N_BENCH] = end - begin;
        }

        begin = get_cycles();
        {
            /* Verify the message */
            if (crypto_sign_open(msg2, &msg2_len, sig_msg, sig_msg_len, pk) != 0)
            {
                printf("Error: Verification failed!\n");
                return -1;
            }
        }
        end = get_cycles();

        if (i >= N_BENCH)
        {
            cc_verf[i - N_BENCH] = end - begin;
        }
    }

    printf("keygen ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%lu ", cc_keyg[i]);
    }

#if defined (OFFLINE_CC)    
    printf("\n\noffline ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%lu ", cc_off[i]);
    }
#endif

    printf("\n\nsign ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%lu ", cc_sign[i]);
    }

    printf("\n\nverf ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%lu ", cc_verf[i]);
    }    

// #if defined (OFFLINE_CC)
//     double avg_sign, avg_off;
//     avg_sign = average(cc_sign, N_BENCH);
//     avg_off = average(cc_off, N_BENCH);
//     // printf("\n\nsign: %.2lf average | offline: %.2lf average | speedup: %.2lf \n", avg_sign, avg_off, (avg_off / avg_sign) * 100);
//     printf("%.0lf %.0lf %.0lf \n", avg_off, avg_sign - avg_off, avg_sign);
// #else
//     printf("\n\nsign: %.2lf average\n", average(cc_sign, N_BENCH));
// #endif

    printf("\n\nDONE!");
    
    return 0;
}

int main()
{

/* 'Va, Short' and 'Vb, Short' requires more than 1 MiB of stack memory.
 * All the other modes requires less than 1 MiB. */
#if MIRITH_MODE == 3 || MIRITH_MODE == 9 || MIRITH_MODE == 11

#if __unix

    struct rlimit rl;

    /* Increase stack size to 64 MiB. */
    getrlimit(RLIMIT_STACK, &rl);

    rl.rlim_cur = 128 * 1024 * 1024; /* 16 MiB. */
    
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

#endif /* #if MIRITH_MODE == 9 || MIRITH_MODE == 11 */

    return bench_mirith();
}