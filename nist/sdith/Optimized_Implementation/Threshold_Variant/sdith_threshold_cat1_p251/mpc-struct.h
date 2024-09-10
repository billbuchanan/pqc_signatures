#ifndef MPC_STRUCT_H
#define MPC_STRUCT_H

#include "parameters.h"
#include "witness.h"

typedef solution_t mpc_wit_t;

typedef struct mpc_unif_t {
    uint8_t a[PARAM_SPLITTING_FACTOR][PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
    uint8_t b[PARAM_SPLITTING_FACTOR][PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
} mpc_unif_t;

typedef struct mpc_corr_t {
    uint8_t c[PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
} mpc_corr_t;

typedef struct mpc_challenge_t {
    uint8_t eval[PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
    uint8_t eps[PARAM_SPLITTING_FACTOR][PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
    uint8_t powers_of_ch[PARAM_CHUNK_LENGTH+1][PARAM_ALL_EVALS_BYTESIZE_CEIL16]; // Pre-computed
    uint8_t f_eval[PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE]; // Pre-computed
} mpc_challenge_t;

typedef struct mpc_broadcast_t {
    uint8_t alpha[PARAM_SPLITTING_FACTOR][PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
    uint8_t beta[PARAM_SPLITTING_FACTOR][PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
    uint8_t v[PARAM_NB_EVALS_PER_POLY][PARAM_EXT_DEGREE];
} mpc_broadcast_t;

#define PARAM_COMPRESSED_BR_SIZE (sizeof(mpc_unif_t))

#include "field.h"
static inline void vec_add(void* x, const void* y, uint32_t size) {
    add_tab_points(x, y, size);
}
static inline void vec_sub(void* x, const void* y, uint32_t size) {
    sub_tab_points(x, y, size);
}
static inline void vec_neg(void* x, uint32_t size) {
    neg_tab_points(x, size);
}
static inline void vec_rnd(void* x, uint32_t size, samplable_t* entropy) {
    random_points(x, size, entropy);
}
#ifdef PARAM_RND_EXPANSION_X4
static inline void vec_rnd_x4(void* const* x, uint32_t size, samplable_x4_t* entropy) {
    random_points_x4(x, size, entropy);
}
#endif /* PARAM_RND_EXPANSION_X4 */
static inline void vec_muladd2(void* z, uint16_t y, const void* x, uint32_t size) {
    mul_and_add_tab_points(z, (uint8_t) y, x, size);
}
static inline void vec_normalize(void* x, uint32_t size) {
    (void) x;
    (void) size;
}

#endif /* MPC_STRUCT_H */
