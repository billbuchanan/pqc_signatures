#include "sha3.h"

uint64_t get64le(const unsigned char *x)
{
  int i;
  uint64_t r = 0;
  for (i=0; i<8; i++)
  {
    r <<= 8;
    r |= x[7-i];
  }
  return r;
}

void put64le(unsigned char *x, uint64_t r)
{
  int i;
  for (i=0; i<8; i++)
  {
    x[i] = r & 0xff;
    r >>= 8;
  }
}

uint64_t lrot64(uint64_t x, unsigned int c)
{
  return (x << c) | (x >> (64-c));
}

void sha3_keccakf(uint64_t st[25])
{
    const uint64_t keccakf_rndc[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };
    const int keccakf_rotc[24] = {
        1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
        27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
    };
    const int keccakf_piln[24] = {
        10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
        15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
    };

    unsigned int i, j, r;
    uint64_t t, bc[5];

    for (i = 0; i < 25; i++)
        st[i] = get64le((unsigned char *) &st[i]);

    for (r = 0; r < KECCAKF_ROUNDS; r++)
    {
        // Theta
        for (i = 0; i < 5; i++)
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

        for (i = 0; i < 5; i++) {
            t = bc[(i + 4) % 5] ^ lrot64(bc[(i + 1) % 5], 1);
            for (j = 0; j < 25; j += 5)
                st[j + i] ^= t;
        }
        // Rho Pi
        t = st[1];
        for (i = 0; i < 24; i++) {
            j = keccakf_piln[i];
            bc[0] = st[j];
            st[j] = lrot64(t, keccakf_rotc[i]);
            t = bc[0];
        }
        //  Chi
        for (j = 0; j < 25; j += 5) {
            for (i = 0; i < 5; i++)
                bc[i] = st[j + i];
            for (i = 0; i < 5; i++)
                st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
        }
        //  Iota
        st[0] ^= keccakf_rndc[r];
    }

    for (i = 0; i < 25; i++)
        put64le((unsigned char *) &st[i], st[i]);
}

void sha3_init_ctx(struct sha3_ctx *ctx, int hashlen)
{
    int i;
    for (i = 0; i < 25; i++)
        ctx->st.q[i] = 0;
    ctx->hashlen = hashlen;
    ctx->rsiz = 200 - 2 * hashlen;
    ctx->pt = 0;
}

void sha3_process_bytes(const char *buf, size_t len, struct sha3_ctx *ctx)
{
    size_t i;
    int j;
    j = ctx->pt;
    for (i = 0; i < len; i++)
    {
        ctx->st.b[j++] ^= buf[i];
        if (j >= ctx->rsiz)
        {
            sha3_keccakf(ctx->st.q);
            j = 0;
        }
    }
    ctx->pt = j;
}

void sha3_finish_ctx(struct sha3_ctx *ctx, char *result)
{
    int i;
    ctx->st.b[ctx->pt] ^= 0x06;
    ctx->st.b[ctx->rsiz - 1] ^= 0x80;
    sha3_keccakf(ctx->st.q);
    for (i = 0; i < ctx->hashlen; i++)
        result[i] = ctx->st.b[i];
}
