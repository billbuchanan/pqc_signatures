#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

#include "biscuit.h"

#include "batch_tools.h"

static void
extract_arg (void *output, size_t outlen, void *arg)
{
  expand_extract (output, outlen, arg);
}

#define batch_sample(dest, q, n, tape) \
  batch_generate (dest, q, n, extract_arg, tape)

#define circuit_sample(dest, q, n, m, d, tape) \
  circuit_generate (dest, q, n, m, d, extract_arg, tape)

static uint16_t
index_sample (int N, int lN, h_ctx_t h_ctx)
{
  const uint16_t mask = (1 << lN) - 1;
  uint8_t v8[2] = {0, 0};
  uint16_t v;
  int n = (lN <= 8) ? 1 : 2;

  do
    {
      expand_extract (v8, n, h_ctx);
      v = (uint16_t) (v8[0] | (v8[1] << 8)) & mask;
    }
  while (v >= N);

  return v;
}

#ifdef SEC_LEVEL
#define lambda SEC_LEVEL
#endif
#ifdef NB_ITERATIONS
#define tau NB_ITERATIONS
#endif
#ifdef NB_PARTIES
#define N NB_PARTIES
#define lN LOG2(NB_PARTIES)
#endif
#ifdef FIELD_SIZE
#define q FIELD_SIZE
#endif
#ifdef NB_VARIABLES
#define n NB_VARIABLES
#endif
#ifdef NB_EQUATIONS
#define m NB_EQUATIONS
#endif
#ifdef DEGREE
#define d DEGREE
#endif
#if defined(DEGREE) && defined(NB_EQUATIONS)
#define C ((DEGREE-1)*NB_EQUATIONS)
#define Cm ((DEGREE-1)*NB_EQUATIONS)
#endif
#if defined(FIELD_SIZE) && defined(NB_VARIABLES)
#define sklen ((LOG2(FIELD_SIZE)*NB_VARIABLES+7)>>3)
#define sklenX CONVX(sklen)
#endif
#if defined(FIELD_SIZE) && defined(NB_EQUATIONS)
#define pklen ((LOG2(FIELD_SIZE)*NB_EQUATIONS+7)>>3)
#define pklenX CONVX(pklen)
#endif
#if defined(FIELD_SIZE) && defined(DEGREE) && defined(NB_EQUATIONS)
#define Clen ((LOG2(FIELD_SIZE)*(DEGREE-1)*NB_EQUATIONS+7)>>3)
#define Cmlen ((LOG2(FIELD_SIZE)*(DEGREE-2)*NB_EQUATIONS+7)>>3)
#define ClenX CONVX(Clen)
#endif

int
keygen (uint8_t *sk, uint8_t *pk, const uint8_t *entropy,
        const params_t *params)
{
#ifndef SEC_LEVEL
  const int lambda = params->lambda;
#endif
#ifndef FIELD_SIZE
  const int q = params->q;
#endif
#ifndef NB_VARIABLES
  const int n = params->n;
#endif
#ifndef NB_EQUATIONS
  const int m = params->m;
#endif
#if !defined(FIELD_SIZE) || !defined(NB_VARIABLES)
  const int sklenX = batch_getlenX (q, n);
#endif
#if !defined(FIELD_SIZE) || !defined(NB_EQUATIONS)
  const int pklenX = batch_getlenX (q, m);
#endif
#ifndef COMPACT_SK
#if !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int C = (d - 1) * m;
#endif
#if !defined(FIELD_SIZE) || !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int ClenX = batch_getlenX (q, C);
#endif
#endif

  const uint8_t *const seedF = entropy;
  const uint8_t *const seedS = entropy + (lambda >> 3);

#ifndef COMPACT_SK
  uintX_t y[ClenX];
#endif
  uintX_t s[sklenX];
  uintX_t t[pklenX];
  uintX_t f[3 * (pklenX + sklenX * m)];

  h_ctx_t expand_ctx;

  BATCH_PARAMS (q, n, m, 2);

  /* Expand seedF */
  expand_init (expand_ctx, seedF, lambda >> 3);
  circuit_sample (f, q, n, m, 2, expand_ctx);

  /* Expand seedS into s */
  expand_init (expand_ctx, seedS, lambda >> 3);
  batch_sample (s, q, n, expand_ctx);

  /* Evaluate the circuit */
#ifndef COMPACT_SK
  eval_circuit (y, NULL, t, s, q, n, m, 2, f);
#else
  eval_circuit (NULL, NULL, t, s, q, n, m, 2, f);
#endif

#ifndef COMPACT_SK
  /* Copy seedF, s, t, y and z in sk */
  memcpy (sk, seedF, (lambda >> 3));
  batch_export (sk + (lambda >> 3), s, q, n, 0);
  batch_export (sk + (lambda >> 3), t, q, m, n);
  batch_export (sk + (lambda >> 3), y, q, C, n + m);
#else
  /* Copy seedF and seedS in sk */
  memcpy (sk, entropy, lambda >> 2);
#endif

  /* Copy seedF and t in pk */
  memcpy (pk, seedF, lambda >> 3);
  batch_export (pk + (lambda >> 3), t, q, m, 0);

  return 0;
}

int
sign (uint8_t *sig, const uint8_t *msg, uint64_t msglen, const uint8_t *sk,
      const uint8_t *entropy, const params_t *params)
{
  int e, i;
  int offset;

#ifndef SEC_LEVEL
  const int lambda = params->lambda;
#endif
#ifndef NB_ITERATIONS
  const int tau = params->tau;
#endif
#ifndef NB_PARTIES
  const int N = params->N;
  const int lN = ilog2 (N);
#endif
#ifndef FIELD_SIZE
  const int q = params->q;
#endif
#ifndef NB_VARIABLES
  const int n = params->n;
#endif
#ifndef NB_EQUATIONS
  const int m = params->m;
#endif
#if !defined(FIELD_SIZE) || !defined(NB_VARIABLES)
  const int sklen = batch_getlen (q, n);
  const int sklenX = batch_getlenX (q, n);
#endif
#if !defined(FIELD_SIZE) || !defined(NB_EQUATIONS)
  const int pklenX = batch_getlenX (q, m);
#endif
#if !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int C = (d - 1) * m;
#endif
#if !defined(FIELD_SIZE) || !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int Clen = batch_getlen (q, C);
  const int ClenX = batch_getlenX (q, C);
#endif

  const uint8_t *const seedF = sk;
  const uint8_t *const sk_data = sk + (lambda >> 3);

  h_ctx_t hash_ctx;
  h_ctx_t expand_ctx;

  uint8_t salt[lambda >> 2];
  uint8_t h1[lambda >> 2];
  uint8_t h2[lambda >> 2];

  uint8_t root[tau][lambda >> 3];
  uint8_t com[tau][N][lambda >> 2];
  uint8_t path[tau][lN * (lambda >> 3)];
  uint8_t seed[tau][N][lambda >> 3];

  uintX_t s[tau][N][sklenX];
  uintX_t a[tau][N][ClenX], c[tau][N][ClenX];
  uintX_t x[tau][N][ClenX], y[tau][N][ClenX], z[tau][N][ClenX];
  uintX_t alpha[tau][N][ClenX], v[tau][N][ClenX];
  uintX_t open_alpha[tau][ClenX];

  uintX_t epsilon[tau][ClenX];
  uint16_t ibar[tau];

  uintX_t delta_s[tau][sklenX];
  uintX_t delta_c[tau][ClenX];

  uintX_t sk_s[sklenX], sk_y[ClenX], sk_t[pklenX];
  uintX_t f[3 * (pklenX + sklenX * m)];

  BATCH_PARAMS (q, n, m, 2);

  /* Expand seedF to obtain the system */
  expand_init (expand_ctx, seedF, lambda >> 3);
  circuit_sample (f, q, n, m, 2, expand_ctx);

#ifndef COMPACT_SK
  batch_import (sk_s, sk_data, q, n, 0);
  batch_import (sk_t, sk_data, q, m, n);
  batch_import (sk_y, sk_data, q, C, n + m);
#else
  /* Expand seedS from sk to obtain secret s */
  expand_init (expand_ctx, sk_data, lambda >> 3);
  batch_sample (sk_s, q, n, expand_ctx);

  /* Evaluate the circuit for intermediate values */
  eval_circuit (sk_y, NULL, sk_t, sk_s, q, n, m, 2, f);
#endif


  /* Phase 1: Commiting to the seeds and views of the parties */

  /* Prepare PRF for salt and root seeds */
  prf_init (expand_ctx, entropy, lambda >> 2);
#ifndef COMPACT_SK
  prf_update (expand_ctx, sk, (lambda >> 3)
              + CDIV8 (batch_getbitlen (q, n + m + C)));
#else
  prf_update (expand_ctx, sk, lambda >> 2);
#endif
  prf_update (expand_ctx, msg, msglen);
  prf_ready (expand_ctx);

  /* Generate salt and root seeds from PRF */
  prf_generate (salt, lambda >> 2, expand_ctx);
  prf_generate ((void *) root, tau * (lambda >> 3), expand_ctx);

  for (e = 0; e < tau; e++)
    {
      /* use temporary buffer to compute delta values */
      uintX_t tmp_e[ClenX];

      /* Get N seeds from root_e */
      get_seeds ((void *) seed[e], root[e], salt, e, N, lambda);

      for (i = 0; i < N; i++)
        {
          h_ctx_t tape_ei_ctx;

          /* Compute commitment on seed[e][i] */
          commit (com[e][i], salt, e, i, seed[e][i], lambda);

          /* Expand random from tape_ei */
          expandtape_init (tape_ei_ctx, salt, e, i, seed[e][i], lambda);
          batch_sample (s[e][i], q, n, tape_ei_ctx);
          batch_sample (a[e][i], q, C, tape_ei_ctx);
          batch_sample (c[e][i], q, C, tape_ei_ctx);
        }

      /* Prepare computation of delta_s[e], init with sk_s */
      batch_copy (delta_s[e], sk_s, q, n);
      for (i = 0; i < N; i++)
        {
          /* Iterated to obtain delta_s[e] = sk_s - sum(s[e][i]) */
          batch_sub (delta_s[e], s[e][i], q, n);
        }

      /* Prepare computation of delta_c[e] with tmp_e, init with 0 */
      batch_clear (delta_c[e], q, C);
      batch_clear (tmp_e, q, C);
      for (i = 0; i < N; i++)
        {
          /* Iterated to obtain tmp_e = sum(a_e[i]) */
          batch_add (tmp_e, a[e][i], q, C);
          /* Iterated to obtain delta_c[e] = - sum(c_e[i]) */
          batch_sub (delta_c[e], c[e][i], q, C);
        }
      /* Obtain delta_c[e] = sk_y * sum(a_e[i]) - sum(c_e[i]) */
      batch_mul (tmp_e, sk_y, q, C);
      batch_add (delta_c[e], tmp_e, q, C);

      /* Correct s[e][0], z[e][0] and c[e][0] */
      batch_add (s[e][0], delta_s[e], q, n);
      batch_add (c[e][0], delta_c[e], q, C);

      for (i = 0; i < N; i++)
        {
          /* Compute the shares x[e][i], y[e][i], z[e][i] */
          /* involved in multiplications */
          linear_circuit (x[e][i], y[e][i], z[e][i], s[e][i], sk_t, i,
                          q, n, m, 2, f);
        }
    }

  /* Initialize sigma1 with salt and msg */
  H1_init (hash_ctx, salt, msg, msglen, lambda);
  for (e = 0; e < tau; e++)
    {
      for (i = 0; i < N; i++)
        {
          /* Add com[e][i] to sigma1 */
          H1_update (hash_ctx, com[e][i], lambda >> 2);
        }
      /* Add delta values to sigma1 */
      H1_update (hash_ctx, (void *) delta_s[e], sklen);
      H1_update (hash_ctx, (void *) delta_c[e], Clen);
    }
  /* Finalize computation of h1 using sigma1 */
  H1_final (h1, hash_ctx, lambda);


  /* Phase 2: Challenging the checking protocol */

  /* Prepare expansion of epsilon[e] challenges */
  expand_init (expand_ctx, h1, lambda >> 2);
  for (e = 0; e < tau; e++)
    {
      /* Obtain challenge epsilon[e] */
      batch_sample (epsilon[e], q, C, expand_ctx);
    }


  /* Phase 3: Commit to simulation of the checking protocol */

  for (e = 0; e < tau; e++)
    {
      for (i = 0; i < N; i++)
        {
          /* Compute alpha[e][i] = x[e][i] * epsilon[e] + a[e][i] */
          batch_copy (alpha[e][i], x[e][i], q, C);
          batch_mul (alpha[e][i], epsilon[e], q, C);
          batch_add (alpha[e][i], a[e][i], q, C);
        }

      /* Prepare computation of open_alpha[e], init with 0 */
      batch_clear (open_alpha[e], q, C);
      for (i = 0; i < N; i++)
        {
          /* Iterated to obtain open_alpha[e] = sum(alpha[e][i]) */
          batch_add (open_alpha[e], alpha[e][i], q, C);
        }

      for (i = 0; i < N; i++)
        {
          /* Compute v[e][i] = y[e][i] * open_alpha[e] */
          /*                 - z[e][i] * epsilon[e] - c[e][i] */
          batch_copy (v[e][i], y[e][i], q, C);
          batch_mul (v[e][i], open_alpha[e], q, C);
          batch_mul (z[e][i], epsilon[e], q, C);
          batch_sub (v[e][i], z[e][i], q, C);
          batch_sub (v[e][i], c[e][i], q, C);
        }
    }

  /* Initialize sigma2 with salt and h1 */
  H2_init (hash_ctx, salt, h1, lambda);
  for (e = 0; e < tau; e++)
    {
      for (i = 0; i < N; i++)
        {
          /* Add alpha[e][i] in sigma2 */
          H2_update (hash_ctx, (void *) alpha[e][i], Clen);
        }
      for (i = 0; i < N; i++)
        {
          /* Add v[e][i] in sigma2 */
          H2_update (hash_ctx, (void *) v[e][i], Clen);
        }
    }
  /* Finalize computation of h2 using sigma2 */
  H2_final (h2, hash_ctx, lambda);


  /* Phase 4: Challenging the views of the MPC protocol */

  /* Prepare expansion of ibar_e challenges */
  expand_init (expand_ctx, h2, lambda >> 2);
  for (e = 0; e < tau; e++)
    {
      /* Obtain challenge ibar_e */
      ibar[e] = index_sample (N, lN, expand_ctx);
    }

  /* Phase 5: Opening the views of the MPC and checking protocols */

  for (e = 0; e < tau; e++)
    {
      /* Compute path to recover seed[e][i], for i != ibar[e] in signature */
      get_path (path[e], root[e], salt, e, ibar[e], N, lambda);
    }

  /* Copy salt in signature */
  memcpy (sig, salt, lambda >> 2);
  sig += lambda >> 2;

  /* Copy h1 in signature */
  memcpy (sig, h1, lambda >> 2);
  sig += lambda >> 2;

  /* Copy h2 in signature */
  memcpy (sig, h2, lambda >> 2);
  sig += lambda >> 2;
  
  for (e = 0; e < tau; e++)
    {
      /* Copy path[e] in signature */
      memcpy (sig, path[e], lN * (lambda >> 3));
      sig += lN * (lambda >> 3);
      /* Copy com[e][ibar[e]] in signature */
      memcpy (sig, com[e][ibar[e]], lambda >> 2);
      sig += lambda >> 2;
    }
  offset = 0;
  for (e = 0; e < tau; e++)
    {
      /* Export delta_s in signature */
      batch_export (sig, delta_s[e], q, n, offset);
      offset += n;
      /* Export delta_c in signature */
      batch_export (sig, delta_c[e], q, C, offset);
      offset += C;
    }
  for (e = 0; e < tau; e++)
    {
      /* Export alpha[e][ibar[e]] in signature */
      batch_export (sig, alpha[e][ibar[e]], q, C, offset);
      offset += C;
    }

  return 0;
}

int
verify (const uint8_t *sig, const uint8_t *msg, uint64_t msglen,
        const uint8_t *pk, const params_t *params)
{
  int e, i;
  int offset;

#ifndef SEC_LEVEL
  const int lambda = params->lambda;
#endif
#ifndef NB_ITERATIONS
  const int tau = params->tau;
#endif
#ifndef NB_PARTIES
  const int N = params->N;
  const int lN = ilog2 (N);
#endif
#ifndef FIELD_SIZE
  const int q = params->q;
#endif
#ifndef NB_VARIABLES
  const int n = params->n;
#endif
#ifndef NB_EQUATIONS
  const int m = params->m;
#endif
#if !defined(FIELD_SIZE) || !defined(NB_VARIABLES)
  const int sklen = batch_getlen (q, n);
  const int sklenX = batch_getlenX (q, n);
#endif
#if !defined(FIELD_SIZE) || !defined(NB_EQUATIONS)
  const int pklenX = batch_getlenX (q, m);
#endif
#if !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int C = (d - 1) * m;
#endif
#if !defined(FIELD_SIZE) || !defined(DEGREE) || !defined(NB_EQUATIONS)
  const int Clen = batch_getlen (q, C);
  const int ClenX = batch_getlenX (q, C);
#endif

  const uint8_t *const seedF = pk;
  const uint8_t *const pk_data = pk + (lambda >> 3);

  const uint8_t *const sigma
    = sig + 3 * (lambda >> 2) + tau * (lN * (lambda >> 3) + (lambda >> 2));

  h_ctx_t hash_ctx;
  h_ctx_t expand_ctx;

  uint8_t salt[lambda >> 2];
  uint8_t h1[lambda >> 2];
  uint8_t h2[lambda >> 2];

  uint8_t h1p[lambda >> 2];
  uint8_t h2p[lambda >> 2];

  uint8_t com[tau][N][lambda >> 2];
  uint8_t path[tau][lN * (lambda >> 3)];
  uint8_t seed[tau][N][lambda >> 3];

  uintX_t s[tau][N][sklenX];
  uintX_t a[tau][N][ClenX], c[tau][N][ClenX];
  uintX_t x[tau][N][ClenX], y[tau][N][ClenX], z[tau][N][ClenX];
  uintX_t alpha[tau][N][ClenX], v[tau][N][ClenX];
  uintX_t open_alpha[tau][ClenX];

  uintX_t epsilon[tau][ClenX];
  uint16_t ibar[tau];

  uintX_t delta_s[tau][sklenX];
  uintX_t delta_c[tau][ClenX];

  uintX_t pk_t[pklenX];
  uintX_t f[3 * (pklenX + sklenX * m)];

  BATCH_PARAMS (q, n, m, 2);

  /* Recover t from pk */
  batch_import (pk_t, pk_data, q, m, 0);

  /* Expand seedF to obtain the system */
  expand_init (expand_ctx, seedF, lambda >> 3);
  circuit_sample (f, q, n, m, 2, expand_ctx);

  /* Extract salt from signature */
  memcpy (salt, sig, lambda >> 2);
  sig += lambda >> 2;

  /* Extract h1 from signature */
  memcpy (h1, sig, lambda >> 2);
  sig += lambda >> 2;

  /* Extract h2 from signature */
  memcpy (h2, sig, lambda >> 2);
  sig += lambda >> 2;

  /* Prepare expansion of epsilon[e] challenges for the checking protocol */
  expand_init (expand_ctx, h1, lambda >> 2);
  for (e = 0; e < tau; e++)
    {
      /* Obtain challenge epsilon[e] */
      batch_sample (epsilon[e], q, C, expand_ctx);
    }

  /* Prepare expansion of ibar[e] challenges for the views of MPC protocol */
  expand_init (expand_ctx, h2, lambda >> 2);
  for (e = 0; e < tau; e++)
    {
      /* Obtain challenge ibar[e] */
      ibar[e] = index_sample (N, lN, expand_ctx);
    }

  for (e = 0; e < tau; e++)
    {
      /* Extract path from signature */
      memcpy (path[e], sig, lN * (lambda >> 3));
      sig += lN * (lambda >> 3);

      /* Extract missing commitment com[e][ibar[e]] from signature */
      memcpy (com[e][ibar[e]], sig, lambda >> 2);
      sig += lambda >> 2;

      offset = e * (n + C);

      /* Extract delta_s[e] from signature */
      batch_import (delta_s[e], sigma, q, n, offset);
      offset += n;

      /* Extract delta_c[e] from signature */
      batch_import (delta_c[e], sigma, q, C, offset);
      offset += C;

      /* Recompute all the seeds from path (extracted from signature) */
      get_path_seeds ((void *) seed[e], path[e], salt, e, ibar[e], N, lambda);

      for (i = 0; i < N; i++)
        {
          if (i != ibar[e])
            {
              h_ctx_t tape_ei_ctx;

              /* Compute commitment on seed[e][i] */
              commit (com[e][i], salt, e, i, seed[e][i], lambda);

              /* Expand random from tape_ei */
              expandtape_init (tape_ei_ctx, salt, e, i, seed[e][i], lambda);
              batch_sample (s[e][i], q, n, tape_ei_ctx);
              batch_sample (a[e][i], q, C, tape_ei_ctx);
              batch_sample (c[e][i], q, C, tape_ei_ctx);

              if (i == 0)
                {
                  /* Correct s[e][0], z[e][0] and c[e][0] */
                  batch_add (s[e][0], delta_s[e], q, n);
                  batch_add (c[e][0], delta_c[e], q, C);
                }

              /* Compute the shares x[e][i], y[e][i], z[e][i] */
              /* involved in multiplications */
              linear_circuit (x[e][i], y[e][i], z[e][i], s[e][i], pk_t, i,
                              q, n, m, 2, f);
            }
        }

      offset = tau * (n + C) + e * C;

      /* Extract alpha[e][ibar[e]] from signature */
      batch_import (alpha[e][ibar[e]], sigma, q, C, offset);
      offset += C;

      for (i = 0; i < N; i++)
        {
          if (i != ibar[e])
            {
              /* Compute alpha[e][i] = x[e][i] * epsilon[e] + a[e][i] */
              batch_copy (alpha[e][i], x[e][i], q, C);
              batch_mul (alpha[e][i], epsilon[e], q, C);
              batch_add (alpha[e][i], a[e][i], q, C);
            }
        }

      /* Prepare computation of open_alpha[e], init with 0 */
      batch_clear (open_alpha[e], q, C);
      for (i = 0; i < N; i++)
        {
          /* Iterated to obtain open_alpha[e] = sum(alpha[e][i]) */
          batch_add (open_alpha[e], alpha[e][i], q, C);
        }

      for (i = 0; i < N; i++)
        {
          if (i != ibar[e])
            {
              /* Compute v[e][i] = y[e][i] * open_alpha[e] */
              /*                 - z[e][i] * epsilon[e] - c[e][i] */
              batch_copy (v[e][i], y[e][i], q, C);
              batch_mul (v[e][i], open_alpha[e], q, C);
              batch_mul (z[e][i], epsilon[e], q, C);
              batch_sub (v[e][i], z[e][i], q, C);
              batch_sub (v[e][i], c[e][i], q, C);
            }
        }

      /* Prepare computation of v[e][ibar[e]], init with 0 */
      batch_clear (v[e][ibar[e]], q, C);
      for (i = 0; i < N; i++)
        {
          if (i != ibar[e])
            {
              /* Iterated to obtain v[e][ibar[e]] = 0 - sum(v[e][i]) */
              /* for i != ibar[e] */
              batch_sub (v[e][ibar[e]], v[e][i], q, C);
            }
        }
    }

  /* Initialize sigma1 with salt and msg */
  H1_init (hash_ctx, salt, msg, msglen, lambda);
  for (e = 0; e < tau; e++)
    {
      for (i = 0; i < N; i++)
        {
          /* Add com[e][i] to sigma1 */
          H1_update (hash_ctx, com[e][i], lambda >> 2);
        }
      /* Add delta values to sigma1 */
      H1_update (hash_ctx, (void *) delta_s[e], sklen);
      H1_update (hash_ctx, (void *) delta_c[e], Clen);
    }
  /* Finalize computation of h1p using sigma1 */
  H1_final (h1p, hash_ctx, lambda);

  /* Initialize sigma2 with salt and h1 */
  H2_init (hash_ctx, salt, h1, lambda);
  for (e = 0; e < tau; e++)
    {
      for (i = 0; i < N; i++)
        {
          /* Add alpha[e][i] in sigma2 */
          H2_update (hash_ctx, (void *) alpha[e][i], Clen);
        }
      for (i = 0; i < N; i++)
        {
          /* Add v[e][i] in sigma2 */
          H2_update (hash_ctx, (void *) v[e][i], Clen);
        }
    }
  /* Finalize computation of h2p using sigma2 */
  H2_final (h2p, hash_ctx, lambda);

  /* Return comparison result of hash values */
  return memcmp (h1, h1p, lambda >> 3) | memcmp (h2, h2p, lambda >> 3);
}
