#ifndef SHA3_H
#define SHA3_H

#include <stddef.h>
#include <stdint.h>

#define KECCAKF_ROUNDS 24

uint64_t get64le(const unsigned char *x);
void put64le(unsigned char *x, uint64_t r);

struct sha3_ctx
{
    union
    {
        unsigned char b[200];
        uint64_t q[25];
    } st;
    int pt, rsiz, hashlen;
};

void sha3_keccakf(uint64_t st[25]);

void sha3_init_ctx(struct sha3_ctx *ctx, int hashlen);
void sha3_process_bytes(const char *buf, size_t len, struct sha3_ctx *ctx);
void sha3_finish_ctx(struct sha3_ctx *ctx, char *result);

#endif
