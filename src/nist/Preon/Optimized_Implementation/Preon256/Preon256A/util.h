#ifndef _UTIL_H__
#define _UTIL_H__

#include <unistd.h>
#include <stdint.h>
#include <openssl/evp.h>

#include "params.h"
#include "aes.h"

void init_SHA3_CTX(EVP_MD_CTX *mdctx, size_t bitSize);

void SHA3(uint8_t *hm, const uint8_t *msg, size_t msg_len, size_t bitSize);

void SHA3s(uint8_t *hm, const uint8_t **msgs, size_t *msgs_len, size_t items, size_t bitSize);

struct AES_ctx *AES(
    const uint8_t key[AES_KEYLEN],
    uint8_t *msg);
void free_AES_ctx(struct AES_ctx *ctx);

int is_power_of_2(const size_t n);

#endif