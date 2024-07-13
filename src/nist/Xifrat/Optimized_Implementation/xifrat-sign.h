/* 2021-03-31, DannyNiu/NJF. Public Domain */

#ifndef xifrat_sign_h
#define xifrat_sign_h 1

#include "xifrat-funcs.h"

typedef struct {
    uint64dup_t C, K, Q, P1, P2;
    uint64dup_t signature;
} xifrat_sign_privkey_context_t;

typedef struct {
    uint64dup_t C, P1, P2;
    uint64dup_t signature;
} xifrat_sign_pubkey_context_t;

// returns x on success and NULL on failure.
void *xifrat_sign_keygen(
    xifrat_sign_privkey_context_t *restrict x,
    GenFunc_t prng_gen, void *restrict prng);

// returns x on success and NULL on failure.
void *xifrat_sign_sign(
    xifrat_sign_privkey_context_t *restrict x,
    void const *restrict msg, size_t msglen);

// returns msg on success and NULL on failure.
void const *xifrat_sign_verify(
    xifrat_sign_pubkey_context_t *restrict x,
    void const *restrict msg, size_t msglen);

typedef struct {
    uint64dup_t C, P1, P2;
} xifrat_sign_pubkey_t;

// returns out on success and NULL on failure (e.g. outlen too short).
void *xifrat_sign_export_pubkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_pubkey_t *restrict out, size_t outlen);

// returns x on success and NULL on failure.
void *xifrat_sign_decode_pubkey(
    xifrat_sign_pubkey_context_t *restrict x,
    xifrat_sign_pubkey_t const *restrict in, size_t inlen);

typedef struct {
    uint64dup_t C, K, Q, P1, P2;
} xifrat_sign_privkey_t;

// returns out on success and NULL on failure (e.g. outlen too short).
void *xifrat_sign_encode_privkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_privkey_t *restrict out, size_t outlen);

// returns x on success on NULL on failure.
void *xifrat_sign_decode_privkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_privkey_t const *restrict in, size_t inlen);

typedef struct {
    uint64dup_t signature;
} xifrat_sign_signature_t;

// returns out on success and NULL on failure (e.g. outlen too short).
void *xifrat_sign_encode_signature(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_signature_t *restrict out, size_t len);

// returns x on success and NULL on failure.
void *xifrat_sign_decode_signature(
    xifrat_sign_pubkey_context_t *restrict x,
    xifrat_sign_signature_t const *restrict in, size_t inlen);

#endif /* xifrat_sign_h */
