#ifndef SDITH_KEYGEN_H
#define SDITH_KEYGEN_H

#include <stdint.h>
#include <stddef.h>
#include "parameters.h"
#include "witness.h"

#define PARAM_PUBLICKEYBYTES (PARAM_INSTANCE_SIZE)
#ifndef SHORT_SECRET_KEY
#define PARAM_SECRETKEYBYTES (PARAM_INSTANCE_SIZE + PARAM_SOL_SIZE)
#else
#define PARAM_SECRETKEYBYTES (PARAM_SEED_SIZE)
#endif

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign_valid_keys(unsigned char *pk, unsigned char *sk);

int sdith_keygen(unsigned char *pk, unsigned char *sk);
int sdith_validate_keys(const unsigned char *pk, const unsigned char *sk);

// SDitH Internal
typedef struct sdith_public_key_t {
    instance_t* inst;
} sdith_public_key_t;

typedef struct sdith_secret_key_t {
    instance_t* inst;
    solution_t* wit;
} sdith_secret_key_t;

int sdith_keygen_internal(sdith_public_key_t* pk, sdith_secret_key_t* sk);
int sdith_validate_keys_internal(const sdith_public_key_t* pk, const sdith_secret_key_t* sk);

int deserialize_public_key(sdith_public_key_t* key, const uint8_t* buf, size_t buflen);
int serialize_public_key(uint8_t* buf, const sdith_public_key_t* key, size_t buflen);

int deserialize_secret_key(sdith_secret_key_t* key, const uint8_t* buf, size_t buflen);
int serialize_secret_key(uint8_t* buf, const sdith_secret_key_t* key, size_t buflen);

int sdith_free_keys_internal(sdith_public_key_t* pk, sdith_secret_key_t* sk);

#endif /* SDITH_KEYGEN_H */
