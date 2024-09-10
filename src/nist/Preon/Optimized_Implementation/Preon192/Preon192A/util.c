#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/evp.h>

#include "util.h"
#include "aes.h"

void init_SHA3_CTX(EVP_MD_CTX *mdctx, size_t bitSize)
{
    const EVP_MD *md;
    switch (bitSize)
    {
    case 256:
        md = EVP_sha3_256();
        break;
    case 384:
        md = EVP_sha3_384();
        break;
    case 512:
        md = EVP_sha3_512();
        break;
    default:
        assert(0 && "Unknown bitsize");
    }
    EVP_DigestInit_ex(mdctx, md, NULL);
}

void SHA3(uint8_t *hm, const uint8_t *msg, size_t msg_len, size_t bitSize)
{
    SHA3s(hm, (const uint8_t *[]){msg}, (size_t[]){msg_len}, 1, bitSize);
}

void SHA3s(uint8_t *hm, const uint8_t **msgs, size_t *msgs_len, size_t items, size_t bitSize)
{
    EVP_MD_CTX *mdctx;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    mdctx = EVP_MD_CTX_new();
    init_SHA3_CTX(mdctx, bitSize);
    for (size_t i = 0; i < items; ++i)
    {
        EVP_DigestUpdate(mdctx, msgs[i], msgs_len[i]);
    }
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    memcpy(hm, md_value, bitSize / 8);
    EVP_MD_CTX_free(mdctx);
}

struct AES_ctx *AES(
    const uint8_t key[AES_KEYLEN],
    uint8_t *msg)
{
    struct AES_ctx *ctx = (struct AES_ctx *)malloc(sizeof(struct AES_ctx));
    AES_init_ctx(ctx, key);
    AES_ECB_encrypt(ctx, msg);
    return ctx;
}

void free_AES_ctx(struct AES_ctx *ctx)
{
    free(ctx);
}

int AES_keylen()
{
    return AES_KEYLEN;
}

int is_power_of_2(const size_t n)
{
    return ((n != 0) && ((n & (n - 1)) == 0));
}
