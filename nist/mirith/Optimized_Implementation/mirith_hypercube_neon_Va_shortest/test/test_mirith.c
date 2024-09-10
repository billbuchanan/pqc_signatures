
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

uint32_t N_TESTS = 100;  /* Number of tests. */
uint32_t MSG_LEN = 80;   /* Message length. */

/* This goes outside of 'main' to avoid stack overflows. */
int test_mirith()
{
    uint32_t i;
    
    printf("\n\ntest_mirith...\n");
    
    for (i = 0; i < N_TESTS; i++)
    {
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
    
        printf("\nTest %d of %d...\n\n", i + 1, N_TESTS);
        
        /* Generate a random message. */
        randombytes(msg, MSG_LEN);
        
        /* Generate public and secret key. */
        crypto_sign_keypair(pk, sk);

        /* Sign the message.*/
        crypto_sign(sig_msg, &sig_msg_len, msg, MSG_LEN, sk);

        /* Verify the message */
        if (crypto_sign_open(msg2, &msg2_len, sig_msg, sig_msg_len, pk) != 0)
        {
            printf("Error: Verification failed!\n");
            return -1;
        }

        /* Check the message length. */
        if (msg_len != msg2_len)
        {
            printf("Error: Message lengths don't match!\n");
            return -1;
        }
        
        /* Check the message. */
        if (memcmp(msg, msg2, msg_len) != 0)
        {
            printf("Error: Messages don't match!\n");
            return -1;
        }

        /* Change one random byte of the signature. */
        randombytes((uint8_t *)&pos, sizeof(pos));
        pos %= sig_msg_len;

        do
        {
            randombytes(&byte, sizeof(byte));
        }
        while (byte == 0);

        sig_msg[pos] ^= byte;
        /* * */

        /* Verify the forged signature. */
        if (crypto_sign_open(msg, &msg_len, sig_msg, sig_msg_len, pk) == 0)
        {
            printf("Error: Trivial forgery possible!\n");
            return -1;
        }

        printf("OK!\n");
    }
    
    printf("\nEverything's OK!\n\n");
    
    return 0;
}

int main()
{

/* Some parameter sets requires a stack larger than 8 MiB. */
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

    return test_mirith();
}
