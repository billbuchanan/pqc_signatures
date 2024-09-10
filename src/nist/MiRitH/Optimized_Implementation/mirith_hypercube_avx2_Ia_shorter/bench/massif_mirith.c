#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../api.h"
#include "../random.h"
#include "../sign.h"

#if __unix
#include <sys/resource.h>
#endif

#if _WIN32
#error "Stack resizing on Windows not implemented yet!"
#endif

uint8_t MSG_LEN = 1;

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


    uint8_t msg[MSG_LEN];
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t sig_msg[CRYPTO_BYTES + MSG_LEN];
	uint64_t len;

    // randombytes(msg, MSG_LEN);
    // randombytes(sig_msg, CRYPTO_BYTES+MSG_LEN);
	int ret = 0;
#ifdef MASSIF_SIGN
    ret |= crypto_sign(sig_msg, &len, msg, MSG_LEN, sk);
#elif defined(MASSIF_KEYGEN)
	ret |= crypto_sign_keypair(pk, sk);
#else
	ret |= crypto_sign_open(msg, &len, sig_msg, CRYPTO_BYTES + MSG_LEN, pk);
	ret = ret > 0;
#endif 
	return ret;
}
