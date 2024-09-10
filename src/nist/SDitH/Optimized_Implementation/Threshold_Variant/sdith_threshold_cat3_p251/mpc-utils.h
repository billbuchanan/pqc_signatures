#ifndef SDITH_MPC_UTILS_H
#define SDITH_MPC_UTILS_H

#include "mpc-struct.h"
#include <stdlib.h>

// No pointer, to have a way to have concatenate
typedef struct mpc_share_t {
    mpc_wit_t wit;
    mpc_unif_t unif;
    mpc_corr_t corr;
} mpc_share_t;

#define PARAM_WIT_SIZE (sizeof(mpc_wit_t))
#define PARAM_UNIF_SIZE (sizeof(mpc_unif_t))
#define PARAM_CORR_SIZE (sizeof(mpc_corr_t))
#define PARAM_SHARE_SIZE (PARAM_WIT_SIZE+PARAM_UNIF_SIZE+PARAM_CORR_SIZE)
#define PARAM_BR_SIZE (sizeof(mpc_broadcast_t))

static inline mpc_share_t* new_share(void) {
    return (mpc_share_t*) malloc(PARAM_SHARE_SIZE);
}

static inline mpc_unif_t* new_unif(void) {
    return (mpc_unif_t*) malloc(PARAM_UNIF_SIZE);
}

static inline mpc_corr_t* new_corr(void) {
    return (mpc_corr_t*) malloc(PARAM_CORR_SIZE);
}

static inline mpc_broadcast_t* new_br(void) {
    return (mpc_broadcast_t*) malloc(PARAM_BR_SIZE);
}

static inline mpc_challenge_t* new_challenge(void) {
    return (mpc_challenge_t*) malloc(sizeof(mpc_challenge_t));
}

static inline mpc_wit_t* get_wit(mpc_share_t* sh) {
    return (mpc_wit_t*) sh;
}

static inline mpc_unif_t* get_unif(mpc_share_t* sh) {
    return (mpc_unif_t*) (((uint8_t*)sh) + PARAM_WIT_SIZE);
}

static inline mpc_corr_t* get_corr(mpc_share_t* sh) {
    return (mpc_corr_t*) (((uint8_t*)sh) + PARAM_WIT_SIZE + PARAM_UNIF_SIZE);
}

static inline void vec_set(void* dst, const void* src, int size) {
    memcpy(dst, src, size);
}

static inline void vec_set_zero(void* x, int size) {
    memset(x, 0, size);
}

#endif /* SDITH_MPC_UTILS_H */
