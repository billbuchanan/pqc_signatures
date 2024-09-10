/*
NIST-developed software is provided by NIST as a public service. You may use, copy, and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify, and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.

NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT, OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.

You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
*/

//   This is a sample 'api.h' for use 'sign.c'

#ifndef api_h
#define api_h

//  Set these three values apropriately for your algorithm
#if defined(USE_PREON128A)
#define PREON_SIG_TYPE 0
#define RAW_CRYPTO_SECRETKEYBYTES 16
#define CRYPTO_PUBLICKEYBYTES 32
#define RAW_CRYPTO_BYTES 178415
#define CRYPTO_ALGNAME "Preon128A"
#elif defined(USE_PREON128B)
#define PREON_SIG_TYPE 0
#define RAW_CRYPTO_SECRETKEYBYTES 16
#define CRYPTO_PUBLICKEYBYTES 32
#define RAW_CRYPTO_BYTES 446015
#define CRYPTO_ALGNAME "Preon128B"
#elif defined(USE_PREON128C)
#define PREON_SIG_TYPE 0
#define RAW_CRYPTO_SECRETKEYBYTES 16
#define CRYPTO_PUBLICKEYBYTES 32
#define RAW_CRYPTO_BYTES 2849135
#define CRYPTO_ALGNAME "Preon128C"
#elif defined(USE_PREON192A)
#define PREON_SIG_TYPE 1
#define RAW_CRYPTO_SECRETKEYBYTES 24
#define CRYPTO_PUBLICKEYBYTES 56
#define RAW_CRYPTO_BYTES 385648
#define CRYPTO_ALGNAME "Preon192A"
#elif defined(USE_PREON192B)
#define PREON_SIG_TYPE 1
#define RAW_CRYPTO_SECRETKEYBYTES 24
#define CRYPTO_PUBLICKEYBYTES 56
#define RAW_CRYPTO_BYTES 934512
#define CRYPTO_ALGNAME "Preon192B"
#elif defined(USE_PREON192C)
#define PREON_SIG_TYPE 1
#define RAW_CRYPTO_SECRETKEYBYTES 24
#define CRYPTO_PUBLICKEYBYTES 56
#define RAW_CRYPTO_BYTES 5857136
#define CRYPTO_ALGNAME "Preon192C"
#elif defined(USE_PREON256A)
#define PREON_SIG_TYPE 2
#define RAW_CRYPTO_SECRETKEYBYTES 32
#define CRYPTO_PUBLICKEYBYTES 64
#define RAW_CRYPTO_BYTES 688561
#define CRYPTO_ALGNAME "Preon256A"
#elif defined(USE_PREON256B)
#define PREON_SIG_TYPE 2
#define RAW_CRYPTO_SECRETKEYBYTES 32
#define CRYPTO_PUBLICKEYBYTES 64
#define RAW_CRYPTO_BYTES 1532305
#define CRYPTO_ALGNAME "Preon256B"
#elif defined(USE_PREON256C)
#define PREON_SIG_TYPE 2
#define RAW_CRYPTO_SECRETKEYBYTES 32
#define CRYPTO_PUBLICKEYBYTES 64
#define RAW_CRYPTO_BYTES 9343329
#define CRYPTO_ALGNAME "Preon256C"
#endif

#define CRYPTO_BYTES (RAW_CRYPTO_BYTES + 1 /* size_t len */ + 16 /* size_t */)
#define CRYPTO_SECRETKEYBYTES (RAW_CRYPTO_SECRETKEYBYTES + CRYPTO_PUBLICKEYBYTES)

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk);

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);

#endif /* api_h */
