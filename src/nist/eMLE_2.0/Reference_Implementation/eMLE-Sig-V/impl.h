#ifndef _IMPL_H
#define _IMPL_H

#include <stdint.h>
#include <stddef.h>

#define N_MAX 128

typedef struct{
    int64_t x1[N_MAX];
    int64_t x2[N_MAX];
    int64_t F1[2][N_MAX];
    int64_t F2[2][N_MAX];
    uint8_t pkh[64];

} privkey_t;

typedef struct{
    int64_t h1[N_MAX];
    int64_t h2[N_MAX];

} pubkey_t;

typedef struct{
    int64_t s[N_MAX];
    int64_t u[N_MAX];
} signature_t;

int verify(const pubkey_t* pk, const uint8_t* m, const size_t mlen, const signature_t* sig, const size_t n);
void sign(signature_t* sig, const privkey_t* sk, const uint8_t* m, const size_t mlen, const size_t n, const uint8_t seed[32]);
void keygen(pubkey_t* pk, privkey_t* sk, const size_t n, const uint8_t seed[32]);
size_t pack_u(unsigned char *r, const int64_t u[N_MAX], const size_t n);
void pack_pk(unsigned char *pk_out, const pubkey_t *pk_in, const size_t n);

#endif
