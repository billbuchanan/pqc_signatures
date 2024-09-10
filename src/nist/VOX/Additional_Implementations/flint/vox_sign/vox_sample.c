/******************************************************************************
 * Pseudo-random sampling of VOX objects by extension from a seed
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#include "vox.h"
#include "fips202.h"
#include <stdint.h>
#include <string.h>

/*
 * Domain-separation tags to sample the various objects
 */
#define TAG_PUB_OIL 0
#define TAG_PUB_VIN 1
#define TAG_S       2
#define TAG_T       3
#define TAG_M       4
#define TAG_VINEGAR 5


/*
 * For performance reasons it is preferable to squeeze full blocks,
 * so we use a buffer that is a multiple of both the bit size of p
 * and the shake256 rate.
 */
#if   (VOX_Q_BITS % 8 == 0)
#define NBLOCKS  (VOX_Q_BITS / 8)
#elif (VOX_Q_BITS % 4 == 0)
#define NBLOCKS  (VOX_Q_BITS / 4)
#elif (VOX_Q_BITS % 2 == 0)
#define NBLOCKS  (VOX_Q_BITS / 2)
#else /* VOX_Q_BITS % 2 == 1 (should not happen) */
#define NBLOCKS  (VOX_Q_BITS )
#endif
#define BUF_LEN  (NBLOCKS * SHAKE256_RATE)
typedef struct {
  keccak_state  keccak_ctx;
  unsigned int  buf_pos;
  uint8_t       buf[BUF_LEN];
} prng_state;

static void prng_init_seed(prng_state *state,
                           const unsigned char seed[VOX_SEED_BYTES],
                           uint8_t tag, uint8_t ctr)
{
  uint8_t in[VOX_SEED_BYTES+2];
  memcpy(in, seed, VOX_SEED_BYTES);
  in[VOX_SEED_BYTES+0] = tag;
  in[VOX_SEED_BYTES+1] = ctr;
  shake256_absorb_once(&(state->keccak_ctx), in, VOX_SEED_BYTES+2);
  state->buf_pos = 8*BUF_LEN;
}

static void prng_init_msg(prng_state *state,
                          const unsigned char hpk[VOX_HPK_BYTES],
                          const unsigned char m[], size_t mlen)
{
  static const uint8_t ctx[2] = {TAG_M, 0};
  shake256_init(&(state->keccak_ctx));
  shake256_absorb(&(state->keccak_ctx), hpk, VOX_HPK_BYTES);
  shake256_absorb(&(state->keccak_ctx), m, mlen);
  shake256_absorb(&(state->keccak_ctx), ctx, 2);
  shake256_finalize(&(state->keccak_ctx));
  state->buf_pos = 8*BUF_LEN;
}

static inline uint16_t read16(uint8_t buf[2]) {
    return (uint16_t) (buf[0] + (((uint16_t) buf[1]) << 8));
}

static uint16_t SampleRejection(prng_state *state)
{
    while (1) {
        /* Refill buffer if it is exhausted */
        if (state->buf_pos == 8*BUF_LEN) {
            shake256_squeezeblocks(state->buf, NBLOCKS, &(state->keccak_ctx));
            state->buf_pos = 0;
        }
#if   (VOX_Q_BITS == 8)
        /* Case VOX_Q_BITS == 8 is easier, and this simpler code should be a bit faster */
        uint8_t r = state->buf[state->buf_pos/8];
#elif (VOX_Q_BITS == 10) || (VOX_Q_BITS == 12)
        /* Splitting the buffer in chunks of 10 or 12 bits means each chunk spans on exactly
         * two bytes */
        uint16_t r = read16(state->buf + state->buf_pos/8);
        r >>= state->buf_pos % 8;
        r &= (1U << VOX_Q_BITS) - 1;
#elif (VOX_Q_BITS == 16)
        uint16_t r = read16(state->buf + state->buf_pos/8);
#else
#error "Not implemented for this VOX_Q_BITS"
#endif /* VOX_Q_BITS */
        state->buf_pos += VOX_Q_BITS;
        if (r < VOX_Q)
            return r;
    }
}

static void fq_nmod_my_rand(fq_nmod_t e, prng_state *state)
{
    int i;
    nmod_poly_t poly;
    nmod_poly_init2(poly, VOX_Q, VOX_C);
    for (i = 0; i < VOX_C; i++) {
        nmod_poly_set_coeff_ui(poly, i, SampleRejection(state));
    }
    fq_nmod_set_nmod_poly(e, poly, Fqc_ctx);
    nmod_poly_clear(poly);
}

/* Sample a matrix in Fp from an already seeded prng */
static void nmod_mat_my_rand(nmod_mat_t e, prng_state *state)
{
    int i, j;
    int r, c;
    r = nmod_mat_nrows(e);
    c = nmod_mat_ncols(e);
    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            nmod_mat_set_entry(e, i, j, SampleRejection(state));
        }
    }
    return;
}

/* Sample a matrix in Fq from an already seeded prng */
static void fq_nmod_mat_my_rand(fq_nmod_mat_t mat, prng_state *state)
{
    int i, j;
    int r, c;
    r = fq_nmod_mat_nrows(mat, Fqc_ctx);
    c = fq_nmod_mat_ncols(mat, Fqc_ctx);
    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            fq_nmod_my_rand(fq_nmod_mat_entry(mat, i, j), state);
        }
    }
    return;
}


/*
 * Samples the non-constrained parts of the public key, that is
 * the oil-oil block for the first VOX_T matrices, plus the
 * vinegar-oil and vinegar-vinegar blocks in all matrices.
 */
void GenQRKey(fq_nmod_mat_t* Key, const unsigned char *seed_Pub)
{
    prng_state prng;
    int i, j, k;

    /* Oil-oil blocks */
    for (i = 0; i < VOX_T; i++) {
        prng_init_seed(&prng, seed_Pub, TAG_PUB_OIL, (uint8_t) i);
        /* matrice complete */
        for (j = 0; j < VOX_OC; j++){
            for (k = 0; k <= j; k++) {
                fq_nmod_my_rand(fq_nmod_mat_entry(Key[i], j, k), &prng);
            }
        }
    }
    /* vinegar-oil and vinegar-vinegar blocks */
    for (i = 0; i < VOX_O; i++) {
        prng_init_seed(&prng, seed_Pub, TAG_PUB_VIN, (uint8_t) i);
        for (j = VOX_OC; j < VOX_NC; j++){
            for (k = 0; k <= j; k++) {
                fq_nmod_my_rand(fq_nmod_mat_entry(Key[i], j, k), &prng);
            }
        }
    }
    return;
}

/*
 * Generate S of the form
 *   [ I_t     S'   ]
 *   [  0   I_(h-t) ]
 * With S' ( of size t x (h-t) ) sampled at random from the seed
 */
void VOX_GenS(nmod_mat_t S, const unsigned char seed_S[VOX_SEED_BYTES])
{
    nmod_mat_t block;
    prng_state prng;
    prng_init_seed(&prng, seed_S, TAG_S, 0);
    /* Write the unit diagonal */
    for (int i=0; i<nmod_mat_ncols(S); i++)
        nmod_mat_set_entry(S, i, i, 1);
    /* Generate the upper block */
    nmod_mat_window_init(block, S, 0, VOX_T, VOX_T, VOX_O);
    nmod_mat_my_rand(block, &prng);
    nmod_mat_window_clear(block);
}

/*
 * Generate T of the form
 *   [ I_h   T' ]
 *   [  0   I_v ]
 * With T' ( of size h x v ) sampled at random from the seed
 */
void VOX_GenT(fq_nmod_mat_t T, const unsigned char seed_T[VOX_SEED_BYTES])
{
    fq_nmod_mat_t block;
    prng_state prng;
    prng_init_seed(&prng, seed_T, TAG_T, 0);
    /* Write the unit diagonal */
    for (int i=0; i<fq_nmod_mat_ncols(T, Fqc_ctx); i++)
        fq_nmod_set_si(fq_nmod_mat_entry(T, i, i), 1, Fqc_ctx);
    /* Generate the upper block */
    fq_nmod_mat_window_init(block, T, 0, VOX_OC, VOX_OC, VOX_NC, Fqc_ctx);
    fq_nmod_mat_my_rand(block, &prng);
    fq_nmod_mat_window_clear(block, Fqc_ctx);
}

void VOX_GenM(nmod_mat_t M,
              const unsigned char hpk[VOX_HPK_BYTES],
              const unsigned char *msg, size_t mlen)
{
    prng_state prng;
    prng_init_msg(&prng, hpk, msg, mlen);
    nmod_mat_my_rand(M, &prng);
}

void VOX_GenVinegar(nmod_mat_t V, mp_limb_t *hint, const unsigned char seed_V[VOX_SEED_BYTES], uint8_t ctr)
{
    prng_state prng;
    prng_init_seed(&prng, seed_V, TAG_VINEGAR, ctr);
    nmod_mat_my_rand(V, &prng);
    *hint = SampleRejection(&prng);
}
