
#ifndef ASCON_API_H_
#define ASCON_API_H_

#include <stdint.h>

#define ASCON_VERSION "1.2.7"
#define ASCON_BYTES 32
#define ASCON_HASH_BYTES 32 /* HASH */
#define ASCON_HASH_ROUNDS 12

int ascon_hash(unsigned char* out, unsigned long long outlen, const unsigned char* in,
                unsigned long long inlen);

#endif