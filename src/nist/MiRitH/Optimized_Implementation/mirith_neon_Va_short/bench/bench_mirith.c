#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../api.h"
#include "../config.h"
#include "../packing.h"
#include "../random.h"
#include "../sign.h"

#if __unix
#include <sys/resource.h>
#endif

#if _WIN32
#error "Stack resizing on Windows not implemented yet!"
#endif

uint32_t N_BENCH = 2024;     /* Number of tests. */
uint32_t MSG_LEN = 80;       /* Message length. */

#if __APPLE__
#define _MAC_OS_
#define _M1CYCLES_
#endif

/*
 * Read the cycle counter. The 'lfence' call should guarantee enough
 * serialization without adding too much overhead (contrary to what,
 * say, 'cpuid' would do).
 */
#if defined(_MAC_OS_)&&defined(_M1CYCLES_)
#include "m1cycles.h"
#endif

#if !defined(CONFIG_BENCH_SYSTIME)
#if defined(__aarch64__)
    static inline uint64_t get_cycles(void) {
        uint64_t val;
#if defined(_MAC_OS_)
        #if defined(_M1CYCLES_)
    return __m1_rdtsc();
  #else
    // counter-timer virtual count register
    // better than nothing.
    __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r" (val));
  #endif
#else
        // performance monitors cycle count register. likely to be banned in user space.
        // see https://github.com/mupq/pqax/tree/main/enable_ccr
        __asm__ __volatile__ ("mrs %0, PMCCNTR_EL0" : "=r" (val));
#endif  // _MAC_OS_
        return val;
    }
#else
    /* Copied from http://en.wikipedia.org/wiki/RDTSC */
static inline uint64_t get_cycles(void) {
	uint32_t lo, hi;
	/* We cannot use "=A", since this would use %rax on x86_64 */
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return (uint64_t)hi << 32 | lo;
}
#endif  // __aarch64__
#endif  // CONFIG_BENCH_SYSTIME

/* This goes outside of 'main' to avoid stack overflows. */
int bench_mirith()
{
    int i;
    uint64_t cc_keyg[N_BENCH];
    uint64_t cc_sign[N_BENCH];
    uint64_t cc_verf[N_BENCH];
    uint64_t begin, end;
#if defined(_MAC_OS_)&&defined(_M1CYCLES_)
    __m1_setup_rdtsc();
#endif

    printf("\n\nBenchmarks...\n\n");
    
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
        
        begin = get_cycles();
        {
            /* Sign the message. */
            crypto_sign(sig_msg, &sig_msg_len, msg, MSG_LEN, sk);
        }
        end = get_cycles();

        if (i >= N_BENCH)
        {
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
        printf("%llu ", cc_keyg[i]);
    }
    
    printf("\n\nsign ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%llu ", cc_sign[i]);
    }

    printf("\n\nverf ");
    for (i = 0; i < N_BENCH; i++)
    {
        printf("%llu ", cc_verf[i]);
    }    
    printf("\n\nDONE!");
    
    return 0;
}

int main()
{

/* 'Va, Short' and 'Vb, Short' requires more than 1 MiB of stack memory.
 * All the other modes requires less than 1 MiB. */
#if MIRITH_MODE == 9 || MIRITH_MODE == 11

#if __unix

    struct rlimit rl;

    /* Increase stack size to 64 MiB. */
    getrlimit(RLIMIT_STACK, &rl);

    rl.rlim_cur = 16 * 1024 * 1024; /* 16 MiB. */
    
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
