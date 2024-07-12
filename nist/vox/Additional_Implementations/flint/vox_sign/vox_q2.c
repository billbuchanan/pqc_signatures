/******************************************************************************
 * Mathematical description of the VOX cryptosystem
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Gilles Macario-Rat, Robin Larrieu
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "vox.h"


/*
 * Counters for the main loops in the signature algorithms
 * to evaluate the number of retries for each step
 */
#define MAXI_COUNT 32

#if VERBOSE>0
int count1[MAXI_COUNT] = { 0 };
int count2[MAXI_COUNT] = { 0 };
void display_counters()
{
  printf("retries 1:");
  for(int i=1;i<MAXI_COUNT;i++)
    printf(" %d",count1[i]);
  printf("\nretries 2:");
  for(int i=1;i<MAXI_COUNT;i++)
    printf(" %d",count2[i]);
  printf("\n");
}
#endif /*  VERBOSE>0 */


#if VERBOSE>0
#define DEBUG_PRINTF      flint_printf
#else
#define DEBUG_PRINTF(...)
#endif


/*
 * Global variables that define the fields
 */
#ifdef DEF_POLY
static const uint16_t def_poly_coeffs[VOX_C+1] = DEF_POLY;
nmod_poly_t def_poly;
#endif
nmod_t Fq_ctx;
fq_nmod_ctx_t Fqc_ctx;
nmod_mat_t base1[VOX_C+1];
nmod_mat_t base2[VOX_C+1];
int nb_limbs;

int retries = 0;


#if VOX_C == 1
unsigned char embed_table_base1[2][1][1] = {{{1}},{{1}}};
unsigned char embed_table_base2[2][1][1] = {{{1}},{{1}}};
#else
#define STRINGIZE_AUX(a) #a
#define STRINGIZE(a) STRINGIZE_AUX(a)
#define EMBED_TABLE_AUX(a, b) tables/embed_##a##_##b##_.h
#define EMBED_TABLE(a,b)      STRINGIZE(EMBED_TABLE_AUX(a,b))
#include EMBED_TABLE(VOX_Q,VOX_C)
#endif


void InitContext()
{
    int i, j, k;

    /* Initialize finite fields */
    nmod_init(&Fq_ctx, VOX_Q);
#ifdef DEF_POLY
    nmod_poly_init2(def_poly, VOX_Q, VOX_C+1);
    for (i = 0; i < VOX_C + 1; i++)
        nmod_poly_set_coeff_ui(def_poly, i, def_poly_coeffs[i]);
    fq_nmod_ctx_init_modulus(Fqc_ctx, def_poly, "a");
#else /* Use FLINTs default polynomial */
    fmpz_t prime;
    fmpz_init_set_si(prime, VOX_Q);
    fq_nmod_ctx_init(Fqc_ctx, prime, VOX_C, "a");
    fmpz_clear(prime);
#endif
    /* Initialize matrices for QR compression/decompression */
    for (k = 0; k < VOX_C + 1; k++) {
        nmod_mat_init(base1[k], VOX_C, VOX_C, VOX_Q);
        nmod_mat_init(base2[k], VOX_C, VOX_C, VOX_Q);
        for (i = 0; i < VOX_C; i++) {
            for (j = 0; j < VOX_C; j++) {
              nmod_mat_entry(base1[k], i, j) = embed_table_base1[k][i][j];
              nmod_mat_entry(base2[k], i, j) = embed_table_base2[k][i][j];
            }
        }
    }

    nb_limbs = _nmod_vec_dot_bound_limbs(VOX_N, Fq_ctx);
    DEBUG_PRINTF("\n InitContext nb_limbs %d \n", nb_limbs);
    return;
}


void ClearContext()
{
    int i;
    for (i = 0; i < VOX_C + 1; i++) {
        nmod_mat_clear(base1[i]);
        nmod_mat_clear(base2[i]);
    }

    fq_nmod_ctx_clear(Fqc_ctx);
#ifdef DEF_POLY
    nmod_poly_clear(def_poly);
#endif
    flint_cleanup();
    return;
}

/*
 * Fold the matrix of a quadratic equation into lower triangular form
 * (F_q^c version)
 */
void fq_nmod_mat_LowTri(fq_nmod_mat_t mat)
{
    int i, j;
    int r = fq_nmod_mat_nrows(mat, Fqc_ctx);
    for (i = 1; i < r; i++) {
        for (j = 0; j < i; j++) {
            fq_nmod_add(fq_nmod_mat_entry(mat, i, j),
                fq_nmod_mat_entry(mat, i, j),
                fq_nmod_mat_entry(mat, j, i), Fqc_ctx);
            fq_nmod_zero(fq_nmod_mat_entry(mat, j, i), Fqc_ctx);
        }
    }
    return;
}

/*
 * Fold the matrix of a quadratic equation into lower triangular form
 * (F_q version)
 */
void nmod_mat_LowTri(nmod_mat_t mat)
{
    int i, j;
    int r = nmod_mat_nrows(mat);
    for (i = 1; i < r; i++) {
        for (j = 0; j < i; j++) {
            nmod_mat_entry(mat, i, j) = nmod_add(
                nmod_mat_entry(mat, i, j),
                nmod_mat_entry(mat, j, i), 
                Fq_ctx
            );
            nmod_mat_entry(mat, j, i) = 0;
        }
    }
    return;
}


/*
 * Lifts an element of F_q into an element of F_q^c
 */
void Lift(fq_nmod_t e2, mp_limb_t e1)
{
    fq_nmod_set_si(e2, e1, Fqc_ctx);
    return;
}

/*
 * Extends an element of F_q^c (given as a vector over F_q) into a block in F_q,
 * using the given translation basis.
 */
static void ExtToBlock(nmod_mat_t block, nmod_mat_t col, nmod_mat_t* base)
{
    int i;
    nmod_mat_t col1;
    /* The i-th column of the destination is extended as base[i]*col */
    for (i = 0; i < VOX_C; i++) {
        nmod_mat_window_init(col1, block, 0, i, VOX_C, i + 1);
        nmod_mat_mul(col1, base[i], col);
        nmod_mat_window_clear(col1);
    }
    return;
}

/*
 * Converts a matrix over F_q^c into a matrix over F_q, using the
 * given translatrion basis.
 */
void ExpandMat(nmod_mat_t mat1, fq_nmod_mat_t mat2, nmod_mat_t* base)
{
    int i, j;
    nmod_mat_t block;
    nmod_mat_t col;

    nmod_mat_init(col, VOX_C, 1, VOX_Q);
    for (i = 0; i < VOX_NC; i++) {
        for (j = 0; j < VOX_NC; j++) {
            nmod_mat_window_init(
                block, 
                mat1,
                i * VOX_C,
                j * VOX_C,
                i * VOX_C + VOX_C,
                j * VOX_C + VOX_C
            );
            fq_nmod_get_nmod_mat(col, fq_nmod_mat_entry(mat2, i, j), Fqc_ctx);
            ExtToBlock(block, col, base);
            nmod_mat_window_clear(block);
        }
    }
    nmod_mat_clear(col);
    return;
}

/*
 * Converts the matrix of a quadratic equation over F_q^c into the matrix
 * of the corresponding quadratic equation over F_q.
 * Input and output are lower triangular.
 */
void ExpandMat_LowTri(nmod_mat_t mat1, fq_nmod_mat_t mat2, nmod_mat_t* base)
/* conversion d'une matrice a grands coeffs en une matrice bloc a petits coeff
 * en utilisant la base donnee, entrée et sortie sont triangulaires inférieure */
{
    int i, j;
    nmod_mat_t block;
    nmod_mat_t col;

    nmod_mat_init(col, VOX_C, 1, VOX_Q);
    for (i = 0; i < VOX_NC; i++) {
        for (j = 0; j <= i; j++) {
            nmod_mat_window_init(
                block,
                mat1,
                i * VOX_C,
                j * VOX_C,
                i * VOX_C + VOX_C,
                j * VOX_C + VOX_C
            );
            fq_nmod_get_nmod_mat(col, fq_nmod_mat_entry(mat2, i, j), Fqc_ctx);
            ExtToBlock(block, col, base);
            if (i == j)
                nmod_mat_LowTri(block);
            nmod_mat_window_clear(block);
        }
    }
    nmod_mat_clear(col);
    return;
}


/*
 * Compute C += B*A*tB
 * Result is folded into a lower triangular matrix
 */
void fq_nmod_mat_AoB(fq_nmod_mat_t C, fq_nmod_mat_t A, fq_nmod_mat_t B)
{
    int i, j;
    int r = fq_nmod_mat_nrows(B, Fqc_ctx);
    int c = fq_nmod_mat_ncols(B, Fqc_ctx);
    fq_nmod_mat_t w;
    fq_nmod_t x;

    fq_nmod_init(x, Fqc_ctx);
    fq_nmod_mat_init(w, r, c, Fqc_ctx);
    /* Compute w = B * A */
    fq_nmod_mat_mul(w, B, A, Fqc_ctx);
    /* Compute C += w * tB and fold the result */
    for (i = 0; i < r; i++) {
        for (j = 0; j < i; j++) {
            _fq_nmod_vec_dot(
                x,
                w->rows[i],
                B->rows[j],
                c,
                Fqc_ctx
            );
            fq_nmod_add(
                fq_nmod_mat_entry(C, i, j),
                fq_nmod_mat_entry(C, i, j),
                x,
                Fqc_ctx
            );
        }
        for (j = i; j < r; j++) {
            _fq_nmod_vec_dot(
                x,
                w->rows[i],
                B->rows[j],
                c,
                Fqc_ctx
            );
            fq_nmod_add(
                fq_nmod_mat_entry(C, j, i),
                fq_nmod_mat_entry(C, j, i),
                x,
                Fqc_ctx
            );
        }
    }
    fq_nmod_mat_clear(w, Fqc_ctx);
    fq_nmod_clear(x, Fqc_ctx);
    return;
}


/*
 * Compute the private key K1 = S o K2 o T from the public key K2 and the
 * secret operators S,T (S over F_q and T over F_q^c)
 */
void fq_nmod_mat_ComposeSTKey(fq_nmod_mat_t* K1, fq_nmod_mat_t* K2, nmod_mat_t S, fq_nmod_mat_t T)
{
    int i, j, k;
    fq_nmod_mat_t Tmp_mat[VOX_O];
    fq_nmod_mat_t SS; /* lift(S, F_q^c) */
    fq_nmod_mat_t tmp_vec;

    fq_nmod_mat_init(SS, VOX_O, VOX_O, Fqc_ctx);
    fq_nmod_mat_init(tmp_vec, 1, VOX_O, Fqc_ctx);

    /* Lift S as a matrix in F_q^c */
    for (i = 0; i < VOX_O; i++) {
        for (j = 0; j < VOX_O; j++) {
            Lift(fq_nmod_mat_entry(SS, i, j), nmod_mat_entry(S, i, j));
        }
    }

    /*
     * Use T to mix oil and vinegar variables :
     * Tmp = K2 o T = T * K2 * tT
     */
    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_init(Tmp_mat[i], VOX_NC, VOX_NC, Fqc_ctx);
        fq_nmod_mat_AoB(Tmp_mat[i], K2[i], T);
    }

    /*
     * Use S to mix random and constrained equations
     * K1 = S o Tmp
     * NOTE: only the lower triangular half is relevant
     */
    for (i = 0; i < VOX_NC; i++) {
        for (j = 0; j <= i; j++) {
            for (k = 0; k < VOX_O; k++) {
                fq_nmod_mat_entry_set(tmp_vec, 0, k, fq_nmod_mat_entry(Tmp_mat[k], i, j), Fqc_ctx);
            }
            fq_nmod_mat_mul(tmp_vec, tmp_vec, SS, Fqc_ctx);
            for (k = 0; k < VOX_O; k++) {
                fq_nmod_mat_entry_set(K1[k], i, j, fq_nmod_mat_entry(tmp_vec, 0, k), Fqc_ctx);
            }
        }
    }

    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_clear(Tmp_mat[i], Fqc_ctx);
    }
    fq_nmod_mat_clear(SS, Fqc_ctx);
    fq_nmod_mat_clear(tmp_vec, Fqc_ctx);


    return;
}

/*
 * Compute the constrained part of the public key using secret operators S,T
 * (S over F_q and T over F_q^c)
 */
static void ComputePetzoldRHS(fq_nmod_mat_t* K2, nmod_mat_t S, fq_nmod_mat_t T)
{
    int i, j, k;
    fq_nmod_mat_t Tmp_mat[VOX_O];
    fq_nmod_mat_t SS; /* lift(S, F_q^c) */
    fq_nmod_mat_t Tp, k1, k2, k3, tmp_vec1, tmp_vec2;

    fq_nmod_mat_init(SS, VOX_O, VOX_O-VOX_T, Fqc_ctx);
    fq_nmod_mat_init(tmp_vec1, 1, VOX_O, Fqc_ctx);
    fq_nmod_mat_init(tmp_vec2, 1, VOX_O-VOX_T, Fqc_ctx);

    /* Lift S as a matrix in F_q^c */
    for (i = 0; i < VOX_O; i++) {
        for (j = VOX_T; j < VOX_O; j++) {
            Lift(fq_nmod_mat_entry(SS, i, j-VOX_T), nmod_mat_entry(S, i, j));
        }
    }

    fq_nmod_mat_window_init(Tp, T, 0, VOX_OC, VOX_OC, VOX_NC, Fqc_ctx);

    /*
     * Use T to mix oil and vinegar variables :
     * Tmp = K2 o T = T * K2 * tT
     * Uses the fact that T = [ I_oc  T'  ] to simplify formulas
     *                        [   0  I_vc ]
     */
    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_init(Tmp_mat[i], VOX_OC, VOX_OC, Fqc_ctx);
        fq_nmod_mat_window_init(k1, K2[i], 0,      0,      VOX_OC, VOX_OC, Fqc_ctx);
        fq_nmod_mat_window_init(k2, K2[i], VOX_OC, 0,      VOX_NC, VOX_OC, Fqc_ctx);
        fq_nmod_mat_window_init(k3, K2[i], VOX_OC, VOX_OC, VOX_NC, VOX_NC, Fqc_ctx);

        fq_nmod_mat_mul(Tmp_mat[i], Tp, k2, Fqc_ctx);
        fq_nmod_mat_LowTri(Tmp_mat[i]);
        if (i < VOX_T) /* Otherwise k1 == 0 */
            fq_nmod_mat_add(Tmp_mat[i], Tmp_mat[i], k1, Fqc_ctx);
        fq_nmod_mat_AoB(Tmp_mat[i], k3, Tp);

        fq_nmod_mat_window_clear(k1, Fqc_ctx);
        fq_nmod_mat_window_clear(k2, Fqc_ctx);
        fq_nmod_mat_window_clear(k3, Fqc_ctx);

    }
    fq_nmod_mat_window_clear(Tp, Fqc_ctx);

    /*
     * Use S to mix random and constrained equations
     * K1 = S o Tmp
     * NOTE: only the lower triangular half is relevant
     */
    for (i = 0; i < VOX_OC; i++) {
        for (j = 0; j <= i; j++) {
            for (k = 0; k < VOX_O; k++) {
                fq_nmod_mat_entry_set(tmp_vec1, 0, k, fq_nmod_mat_entry(Tmp_mat[k], i, j), Fqc_ctx);
            }
            fq_nmod_mat_mul(tmp_vec2, tmp_vec1, SS, Fqc_ctx);
            for (k = 0; k < VOX_O - VOX_T; k++) {
                fq_nmod_neg(
                    fq_nmod_mat_entry(K2[k+VOX_T], i, j),
                    fq_nmod_mat_entry(tmp_vec2, 0, k),
                    Fqc_ctx
                );
            }
        }
    }

    fq_nmod_mat_clear(SS, Fqc_ctx);
    fq_nmod_mat_clear(tmp_vec1, Fqc_ctx);
    fq_nmod_mat_clear(tmp_vec2, Fqc_ctx);
    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_clear(Tmp_mat[i], Fqc_ctx);
    }

    return;
}


/*
 * Generates a QR-compressed public key compatible with secret transforms
 * S and T.
 * Non-constrained parts of the public key are sampled from seed_PK;
 * transforms S,T are sampled from seed_SK.
 */
void ComputePetzoldQR(fq_nmod_mat_t* Pub, nmod_mat_t S, fq_nmod_mat_t T,
                      const unsigned char *seed_PK, const unsigned char *seed_SK)
{
    GenQRKey(Pub, seed_PK);

    VOX_GenS(S, seed_SK);
    VOX_GenT(T, seed_SK);

    ComputePetzoldRHS(Pub, S, T);
}


/*
 * Generates a VOX key pair from seed_PK, seed_SK.
 * Result is fully extended as quadratic equations over F_q.
 */
void GenVoxKeys(nmod_mat_t* Pub, nmod_mat_t* Sec, nmod_mat_t S, nmod_mat_t T,
                const unsigned char *seed_PK, const unsigned char *seed_SK)
{
    int i;
    fq_nmod_mat_t T2, Pub2[VOX_O], Sec2[VOX_O];

    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_init(Pub2[i], VOX_NC, VOX_NC, Fqc_ctx);
        fq_nmod_mat_init(Sec2[i], VOX_NC, VOX_NC, Fqc_ctx);
    }
    fq_nmod_mat_init(T2, VOX_NC, VOX_NC, Fqc_ctx);
    ComputePetzoldQR(Pub2, S, T2, seed_PK, seed_SK);
    ExpandMat(T, T2, base1);
    fq_nmod_mat_ComposeSTKey(Sec2, Pub2, S, T2);
    for (i = 0; i < VOX_O; i++) {
        ExpandMat_LowTri(Pub[i], Pub2[i], base2);
        fq_nmod_mat_clear(Pub2[i], Fqc_ctx);
        ExpandMat_LowTri(Sec[i], Sec2[i], base2);
        fq_nmod_mat_clear(Sec2[i], Fqc_ctx);
    }
    fq_nmod_mat_clear(T2, Fqc_ctx);
}


/*
 * Sign the target vector using secret system Sec and transforms S,T.
 * Vinegar variables are sampled from seed_V
 */
int Sign(nmod_mat_t Signature,
         nmod_mat_t Message, nmod_mat_t* Sec, nmod_mat_t S, nmod_mat_t T,
         const unsigned char seed_V[VOX_SEED_BYTES])
{
    int i, j, k, l, res, nb_retries1, nb_retries2, ctr;
    const int t = VOX_T;
    const int t2 = ((t + 2) * (t + 1)) / 2;
    slong pivots[VOX_O+1], perm[VOX_O+1], *non_pivots;
    int nb_pivots;
    mp_limb_t hint; /* Hint to select the root in univariate solving */

    nmod_mat_t x, y1, y2, z, u, w, lin, lin2, tlin2, tmp1, tmp2, tmp3, tmp4, MQ_eqn;
    int xx;

    nmod_mat_init(x, 1, VOX_V, VOX_Q);
    nmod_mat_init(y1, 1, VOX_O, VOX_Q);
    nmod_mat_init(y2, 1, VOX_O, VOX_Q);
    nmod_mat_init(z, 1, VOX_V, VOX_Q);
    nmod_mat_init(u, 1, VOX_N, VOX_Q);
    nmod_mat_init(w, 1, t + 1, VOX_Q);
    nmod_mat_init(lin, VOX_O - t, VOX_O + 1, VOX_Q);
    nmod_mat_init(lin2, VOX_O, t + 1, VOX_Q);
    nmod_mat_init(tlin2, t + 1, VOX_O, VOX_Q);
    nmod_mat_init(tmp3, VOX_O, t + 1, VOX_Q);
    nmod_mat_init(tmp4, t + 1, t + 1, VOX_Q);
    nmod_mat_init(MQ_eqn, t, t2, VOX_Q);

    /* Mix equations using operator S */
    nmod_mat_mul(y1, Message, S);               /* (1,h) = (1,h) x (h, h) */

    ctr = 0;
    nb_retries1 = 0;
    res = 0;
    do {
        if (nb_retries1 >= MAXI_COUNT) {
            DEBUG_PRINTF("Sign : too many retries second pass !\n");
            res = -1;
            break;
            /*return 1;*/
        }
        nb_retries1 ++;
        nb_retries2 = 0;
        do {
            if ((ctr >= 256) || (nb_retries2 >= MAXI_COUNT)) {
                DEBUG_PRINTF("Sign : too many retries first pass  !\n");
                res = -1;
                break;
                /*return 1;*/
            }
            /* Sample vinegar variables + hint */
            VOX_GenVinegar(x, &hint, seed_V, (uint8_t) ctr);
            ctr ++;
            nb_retries2 ++;

            /* Get affine relations between oil variables */
            for (i = t; i < VOX_O; i++) {
                /* oil-vinegar part */
                nmod_mat_window_init(tmp1, lin, i - t, 0, i - t + 1, VOX_O);
                nmod_mat_window_init(tmp2, Sec[i], VOX_O, 0, VOX_N, VOX_O);
                nmod_mat_mul(tmp1, x, tmp2);   /* (1,o) = (1,o) x (v, o) */
                nmod_mat_window_clear(tmp1);
                nmod_mat_window_clear(tmp2);
                /* vinegar-vinegar parts */
                nmod_mat_window_init(tmp2, Sec[i], VOX_O, VOX_O, VOX_N, VOX_N);
                nmod_mat_mul(z, x, tmp2);      /* (1,v) = (1,v) x (v, v) */
                nmod_mat_window_clear(tmp2);
                nmod_mat_set_entry(lin, i - t, VOX_O,
                    _nmod_vec_dot(x->rows[0], z->rows[0], VOX_V, Fq_ctx, nb_limbs)
                );
                /* Inject target value to be signed */
                nmod_mat_set_entry(lin, i - t, VOX_O,
                    nmod_sub(
                        nmod_mat_entry(lin, i - t, VOX_O),
                        nmod_mat_entry(y1, 0, i),
                        Fq_ctx
                    )
                );
            }
            /* Solve the resulting affine system */
            nb_pivots = _nmod_mat_rref(lin, pivots, perm);
            non_pivots = pivots + nb_pivots;
            /* A solution is valid if there are t free variables,
             * and h-t non-free variables, so the rank must be h-t
             * and the last column must not be a pivot */
            if ((nb_pivots == VOX_O-t) && (pivots[nb_pivots-1] < VOX_O))
                res = 1;
            else
                res = 0;
        } while (res != 1);
        if (res != 1) {
            res = -1;
            break;
        }
        /* Dependency matrix between free and non-free variables */
        for (i=0; i<VOX_O-t; i++) {
          for (j=0; j<t+1; j++) {
            mp_limb_t coeff = nmod_neg(nmod_mat_entry(lin, i, non_pivots[j]),Fq_ctx);
            nmod_mat_set_entry(lin2, pivots[i], j, coeff);
          }
        }
        for (i=0; i<t; i++) {
          for (j=0; j<t+1; j++) {
            nmod_mat_set_entry(lin2, non_pivots[i], j, (j==i) ? 1 : 0);
          }
        }

        /* transpose lin2 */
        nmod_mat_transpose(tlin2, lin2);
 
        /* Building the VOX quadratic system (t equations in t variables) */
        for (i = 0; i < t; i++) {
            /* oil-oil part */
            nmod_mat_window_init(tmp2, Sec[i], 0, 0, VOX_O, VOX_O);
            nmod_mat_mul(tmp3, tmp2, lin2);        /* (o,t+1) = (o,o) x (o, t+1) */
            nmod_mat_window_clear(tmp2);
            nmod_mat_mul(tmp4, tlin2, tmp3);       /* (t+1,t+1) = (t+1,o) x (o, t+1) */

            /* vinegar-vinegar part  */
            nmod_mat_window_init(tmp2, Sec[i], VOX_O, VOX_O, VOX_N, VOX_N);
            nmod_mat_mul(z, x, tmp2);              /* (1,v) = (1,v) x (v, v) */
            nmod_mat_window_clear(tmp2);
            xx = _nmod_vec_dot(x->rows[0], z->rows[0], VOX_V, Fq_ctx, nb_limbs);
            /* Inject target value to be signed*/
            xx = nmod_sub(xx, nmod_mat_entry(y1, 0, i), Fq_ctx);
            nmod_mat_set_entry(tmp4, t, t,
                nmod_add(nmod_mat_entry(tmp4, t, t), xx, Fq_ctx)
            );
            /* oil-vinegar part */
            nmod_mat_window_init(tmp2, Sec[i], VOX_O, 0, VOX_N, VOX_O);
            nmod_mat_mul(y2, x, tmp2);              /* (1,o) = (1,v) x (v,o) */
            nmod_mat_window_clear(tmp2);
            nmod_mat_mul(w, y2, lin2);              /* (1,t+1) = (1,o) x (h, t+1) */
            for (j = 0; j < t + 1; j++) {
                nmod_mat_set_entry(tmp4, t, j,
                    nmod_add(
                        nmod_mat_entry(w, 0, j),
                        nmod_mat_entry(tmp4, t, j),
                        Fq_ctx
                    )
                );
            }
            /* Normalize equation in lower triangular form, and
             * copy monomials in Grevlex order */
            nmod_mat_LowTri(tmp4);
            l = 0;
            for (j = 0; j <= t; j++) {
                for (k = 0; k <= j; k++) {
                    nmod_mat_set_entry(
                        MQ_eqn, i, l,
                        nmod_mat_entry(tmp4, j, k)
                    );
                    l++;
                }
            }
        }
        /* Solve the small quadratic system */
        res = Solve_MQ(w, MQ_eqn, 2, hint);

    } while (res != 1);
#if VERBOSE>0
        count1[nb_retries1]++;
        count2[nb_retries2]++;
#endif /*  VERBOSE>0 */
    if (res == 1) {
        /* Signature is successful, recover oil variables from the solution */
        nmod_mat_set_entry(w, 0, t, 1);
        nmod_mat_window_init(tmp1, u, 0, 0, 1, VOX_O);
        nmod_mat_mul(tmp1, w, tlin2); /* (1,h) = (1,t+1) x (t+1,h) */
        nmod_mat_window_clear(tmp1);
        /* Copy vinegar variables */
        nmod_mat_window_init(tmp1, u, 0, VOX_O, 1, VOX_N);
        nmod_mat_set(tmp1, x);
        nmod_mat_window_clear(tmp1);
        /* Mix oil and vinegar variables using operator T */
        nmod_mat_mul(u, u, T); /* (1,n) = (1,n) x (n,n) */
        nmod_mat_set(Signature, u);
        res = 1;
    }
    else
    {
        /* Signature failed */
        res = -1;
    }

    /* Release memory and exit */
    nmod_mat_clear(x);
    nmod_mat_clear(y1);
    nmod_mat_clear(y2);
    nmod_mat_clear(z);
    nmod_mat_clear(u);
    nmod_mat_clear(w);
    nmod_mat_clear(lin);
    nmod_mat_clear(lin2);
    nmod_mat_clear(tlin2);
    nmod_mat_clear(tmp3);
    nmod_mat_clear(tmp4);
    nmod_mat_clear(MQ_eqn);
    return res;
}

int Verify(nmod_mat_t Signature, nmod_mat_t Message, nmod_mat_t* Pub)
{
    int i, res;
    int y;
    nmod_mat_t x;

    nmod_mat_init(x, 1, VOX_N, VOX_Q);
    res = 1;
    for (i = 0; i < VOX_O; i++) {
        nmod_mat_mul(x, Signature, Pub[i]);          /* (1,n) = (1,n) x (n,n) */
        y = _nmod_vec_dot(x->rows[0], Signature->rows[0], VOX_N, Fq_ctx, nb_limbs);
        res = res && (nmod_mat_entry(Message, 0, i) == y);
    }
    nmod_mat_clear(x);
    return res;
}
