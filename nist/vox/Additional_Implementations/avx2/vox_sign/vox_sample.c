/******************************************************************************
 * Pseudo-random sampling of VOX objects by extension from a seed
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#include "vox_sample.h"
#include "fips202/fips202.h"
#include "fips202/fips202x4.h"
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
 * so we use a buffer that is large enough for shake256_rate plus
 * a partially sampled F_q element
 */
#if (8*SHAKE256_RATE % VOX_Q_BITS == 0)
#define BUF_LEN  (SHAKE256_RATE)
#else
#define BUF_LEN  (SHAKE256_RATE + (VOX_Q_BITS+7)/8)
#endif
typedef struct {
  keccak_state  keccak_ctx;
  unsigned int  buf_pos;
  unsigned int  buf_len;
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
  state->buf_len = 8*BUF_LEN;
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
  state->buf_len = 8*BUF_LEN;
}

static inline uint16_t read16(const uint8_t buf[2]) {
    return (uint16_t) (buf[0] + (((uint16_t) buf[1]) << 8));
}


/*
 * Perform rejection sampling on the output of prng to get a uniform
 * element of F_q
 */
static Fq RejSample_Fq(prng_state *state)
{
    while (1) {
        /*
         * Refill buffer if it is exhausted
         */
#if   ((8*SHAKE256_RATE) % VOX_Q_BITS == 0)
        if (state->buf_pos == 8*BUF_LEN) {
            shake256_squeezeblocks(state->buf, 1, &(state->keccak_ctx));
            state->buf_pos = 0;
        }
#else /* buffer may still contain data, but not enough to get a new element */
        if (state->buf_len < state->buf_pos + VOX_Q_BITS) {
            int buf_offset = (state->buf_pos)/8;
            int nbytes = (state->buf_len)/8 - buf_offset;
            for (int i=0; i<nbytes; i++)
                state->buf[i] = state->buf[buf_offset+i];
            /* nbytes <= ceil(VOX_Q_BITS/8) -> nbytes + SHAKE256_RATE <= BUF_LEN */
            shake256_squeezeblocks(state->buf+nbytes, 1, &(state->keccak_ctx));
            state->buf_pos = (state->buf_pos)%8;
            state->buf_len = 8*(nbytes+SHAKE256_RATE);
        }
#endif /* (8*SHAKE256_RATE % VOX_Q_BITS == 0) ? */
#if   (VOX_Q_BITS == 8)
        /* Case VOX_Q_BITS == 8 is easier, and this simpler code should be a bit faster */
        uint8_t r = state->buf[state->buf_pos/8];
#elif (VOX_Q_BITS == 10) || (VOX_Q_BITS == 12)
        /* Splitting the buffer in chunks of 10 or 12 bits means each chunk spans on exactly
         * two bytes */
        uint16_t r = read16(state->buf + state->buf_pos/8);
        r >>= state->buf_pos % 8;
        r &= (1U << VOX_Q_BITS) - 1;
#else
#error "Not implemented for this VOX_Q_BITS"
#endif /* VOX_Q_BITS */
        state->buf_pos += VOX_Q_BITS;
        if (r < VOX_Q)
            return r;
    }
}

/*
 * Perform rejection sampling on the output of prng to get a uniform
 * element of F_q^c
 */
static void RejSample_Fqc(Fqc *dst, prng_state *state)
{
    for (int i = 0; i < VOX_C; i++)
        dst->coeffs[i] = RejSample_Fq(state);
}


/*****************************************************************************/


/*
 * Generate the oil-oil block of Pub_i (assumes i < VOX_T)
 */
void VOX_GenPK_oil(Fqc Pub_i[VOX_NC*VOX_NC], const unsigned char seed[VOX_SEED_BYTES], int i) {
    prng_state prng;
    int j, k;

    prng_init_seed(&prng, seed, TAG_PUB_OIL, (uint8_t) i);
    for (j = 0; j < VOX_OC; j++) {
        for (k = 0; k <= j; k++) {
            RejSample_Fqc(&Pub_i[j*VOX_NC + k], &prng);
        }
    }
}


/*
 * Generate the vinegar-oil and vinegar-vinegar blocks of Pub_i
 */
void VOX_GenPK_vin(Fqc Pub_i[VOX_NC*VOX_NC], const unsigned char seed[VOX_SEED_BYTES], int i) {
    prng_state prng;
    int j, k;

    prng_init_seed(&prng, seed, TAG_PUB_VIN, (uint8_t) i);
    for (j = VOX_OC; j < VOX_NC; j++) {
        for (k = 0; k <= j; k++) {
            RejSample_Fqc(&Pub_i[j*VOX_NC + k], &prng);
        }
    }
}


/*
 * Generate the non-constrained parts of the public key
 */
void VOX_GenPK_template(Fqc **Pub, const unsigned char seed[VOX_SEED_BYTES])
{
    int i;
    for (i=0; i+3<VOX_T; i+=4)
        VOX_GenPK_oil_x4(&Pub[i], seed, i);
#if (VOX_T % 4 == 1)
    VOX_GenPK_oil(Pub[i], seed, i);
#elif (VOX_T % 4 != 0)
    VOX_GenPK_oil_x4(&Pub[i], seed, i);
#endif

    for (i=0; i+3<VOX_O; i+=4)
        VOX_GenPK_vin_x4(&Pub[i], seed, i);
#if (VOX_O % 4 == 1)
    VOX_GenPK_vin(Pub[i], seed, i);
#elif (VOX_T % 4 != 0)
    VOX_GenPK_vin_x4(&Pub[i], seed, i);
#endif
}


/*
 * Generate the matrix S used to mix the equations
 */
void VOX_GenS(Fq S[VOX_T*(VOX_O-VOX_T)], const unsigned char seed[VOX_SEED_BYTES])
{
    prng_state prng;
    int i,j;

    prng_init_seed(&prng, seed, TAG_S, 0);
    for (i=0; i<VOX_T; i++) {
        for (j=0; j<VOX_O-VOX_T; j++) {
            S[i*(VOX_O-VOX_T) + j] = RejSample_Fq(&prng);
        }
    }
}


/*
 * Generate the matrix T used to mix the variables
 */
void VOX_GenT(Fqc T[VOX_OC*VOX_VC], const unsigned char seed[VOX_SEED_BYTES])
{
    prng_state prng;
    int i,j;

    prng_init_seed(&prng, seed, TAG_T, 0);
    for (i=0; i<VOX_OC; i++) {
        for (j=0; j<VOX_VC; j++) {
            RejSample_Fqc(&T[i*VOX_VC+j], &prng);
        }
    }
}


/*
 * Generate the target vector corresponding to the input message
 */
void VOX_GenM(Fq M[VOX_O],
              const unsigned char hpk[VOX_HPK_BYTES],
              const unsigned char *msg, size_t mlen)
{
    prng_state prng;
    prng_init_msg(&prng, hpk, msg, mlen);
    for (int i=0; i<VOX_O; i++)
        M[i] = RejSample_Fq(&prng);
}


/*
 * Generate the vinegar variables, plus a hint to select the root in univariate resolution.
 * A counter is used to sample new variables for each signature attempt.
 */
void VOX_GenVinegar(Fq V[VOX_V], Fq *hint, const unsigned char seed[VOX_SEED_BYTES], uint8_t ctr)
{
    prng_state prng;
    prng_init_seed(&prng, seed, TAG_VINEGAR, ctr);
    for (int i=0; i<VOX_V; i++)
        V[i] = RejSample_Fq(&prng);
    *hint = RejSample_Fq(&prng);
}


/*****************************************************************************/

typedef struct {
    keccakx4_state keccak_ctx;
    unsigned int  buf_pos;
    unsigned int  buf_len;
    uint8_t       buf[4][BUF_LEN];
} prngx4_state;

static void prngx4_init(prngx4_state *state,
                        const unsigned char seed[VOX_SEED_BYTES],
                        uint8_t ctx1, uint8_t ctx2)
{
    for (int i=0; i<4; i++) {
        memcpy(state->buf[i], seed, VOX_SEED_BYTES);
        state->buf[i][VOX_SEED_BYTES]   = ctx1;
        state->buf[i][VOX_SEED_BYTES+1] = (unsigned char) (ctx2 + i);
    }
    shake256x4_absorb(&(state->keccak_ctx),
                      state->buf[0], state->buf[1], state->buf[2], state->buf[3],
                      VOX_SEED_BYTES+2);
    state->buf_pos = 8*BUF_LEN;
    state->buf_len = 8*BUF_LEN;
}


/*
 * Using a parallel version of keccak means we have to synchronize the
 * refresh of the buffer. So for each matrix we need to sample all coefficients
 * available from the buffer.
 */

typedef struct {
    Fqc *dst; /* pointer to output matrix. pointer is set to NULL when finished */
    int i,j,k; /* index of the current coefficient (row, column, degree in Fqc elt) */
} rejsample_progress_t;

static void rejsample_update(rejsample_progress_t *out,
                             const unsigned char buf[BUF_LEN],
                             int buf_pos, int buf_len, int maxi)
{
    if (out->dst == NULL)
        return;
    int i = out->i;
    int j = out->j;
    int k = out->k;
    while (buf_pos + VOX_Q_BITS <= buf_len) {
#if   (VOX_Q_BITS == 8)
        /* Case VOX_Q_BITS == 8 is easier, and this simpler code should be a bit faster */
        uint8_t r = buf[buf_pos/8];
#elif (VOX_Q_BITS == 10) || (VOX_Q_BITS == 12)
        /* Splitting the buffer in chunks of 10 or 12 bits means each chunk spans on exactly
         * two bytes */
        uint16_t r = read16(buf + buf_pos/8);
        r >>= buf_pos % 8;
        r &= (1U << VOX_Q_BITS) - 1;
#else
#error "Not implemented for this VOX_Q_BITS"
#endif /* VOX_Q_BITS */
        buf_pos += VOX_Q_BITS;
        if (r < VOX_Q) {
            out->dst[i*VOX_NC+j].coeffs[k] = r;
            k++;
            if (k == VOX_C) { /* finished this Fqc coefficient */
                j++;
                k=0;
                if (j > i) { /* finished this row of the lower triangular matrix */
                    i++;
                    j=0;
                    if (i==maxi) { /* finished this matrix */
                        out->dst = NULL;
                        return;
                    }
                }
            }
        }
    }
    out->i = i;
    out->j = j;
    out->k = k;
}

static void Rejsample_FqcBlock_x4(Fqc *Pub_i[4], int tag,
                                  const unsigned char seed[VOX_SEED_BYTES], int i)
{
    prngx4_state prng;
    rejsample_progress_t out[4];
    int j, k, start, end;

    if (tag == TAG_PUB_OIL) {
        prngx4_init(&prng, seed, TAG_PUB_OIL, (uint8_t) i);
        start = 0;
        end = VOX_OC;
        k = VOX_T;
    }
    else {
        prngx4_init(&prng, seed, TAG_PUB_VIN, (uint8_t) i);
        start = VOX_OC;
        end = VOX_NC;
        k = VOX_O;
    }
    for (j=0; j<4; j++) {
        if (i+j < k) {
            out[j].dst = Pub_i[j];
            out[j].i = start;
            out[j].j = 0;
            out[j].k = 0;
        }
        else {
            out[j].dst = NULL;
        }
    }

    while (out[0].dst || out[1].dst || out[2].dst || out[3].dst) {
        /*
         * Refill buffer
         */
#if   ((8*SHAKE256_RATE) % VOX_Q_BITS == 0)
        shake256x4_squeezeblocks(prng.buf[0], prng.buf[1], prng.buf[2], prng.buf[3],
                                1, &(prng.keccak_ctx));
        prng.buf_pos = 0;
#else /* buffer may still contain data, but not enough to get a new element */
        int buf_offset = (prng.buf_pos)/8;
        int nbytes = (prng.buf_len)/8 - buf_offset;
        unsigned char *outbuf[4];
        for (j=0; j<4; j++) {
            for (k=0; k<nbytes; k++)
                prng.buf[j][k] = prng.buf[j][k+buf_offset];
            outbuf[j] = prng.buf[j]+nbytes;
        }
        shake256x4_squeezeblocks(outbuf[0], outbuf[1], outbuf[2], outbuf[3],
                                1, &(prng.keccak_ctx));
        prng.buf_pos = prng.buf_pos%8;
        prng.buf_len = 8*(nbytes+SHAKE256_RATE);
#endif /* (8*SHAKE256_RATE % VOX_Q_BITS == 0) ? */
        /*
         * Consume data in the buffer to fill the output
         */
        for (j=0; j<4; j++)
            rejsample_update(&out[j], prng.buf[j], prng.buf_pos, prng.buf_len, end);
        prng.buf_pos += VOX_Q_BITS * ((prng.buf_len - prng.buf_pos)/VOX_Q_BITS);
    }
}

/*
 * Generate the oil-oil block of Pub[i:i+4] (assumes i < VOX_T)
 */
void VOX_GenPK_oil_x4(Fqc *Pub_i[4], const unsigned char seed[VOX_SEED_BYTES], int i) {
    Rejsample_FqcBlock_x4(Pub_i, TAG_PUB_OIL, seed, i);
}

/*
 * Generate the vinegar-oil and vinegar-vinegar blocks of Pub[i:i+4] (assumes i < VOX_O)
 */
void VOX_GenPK_vin_x4(Fqc *Pub_i[4], const unsigned char seed[VOX_SEED_BYTES], int i) {
    Rejsample_FqcBlock_x4(Pub_i, TAG_PUB_VIN, seed, i);
}
