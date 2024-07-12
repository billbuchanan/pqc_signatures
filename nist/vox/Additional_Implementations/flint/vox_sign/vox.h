/******************************************************************************
 * Header file for VOX internal functions
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Gilles Macario-Rat
 *****************************************************************************/
#ifndef _VOX_H_
#define _VOX_H_

#include <stdbool.h>
#include <stdint.h>
#include "flint/flint.h"
#include "flint/fmpz_vec.h"
#include "flint/nmod_vec.h"
#include "flint/nmod_poly.h"
#include "flint/nmod_mat.h"
#include "flint/fq_nmod.h"
#include "flint/fq_nmod_poly.h"
#include "flint/fq_nmod_vec.h"
#include "flint/fq_nmod_mat.h"

#include "vox_params.h"


/* Global variables that define the fields */
#ifdef DEF_POLY
extern nmod_poly_t def_poly;   /* FLINT equivalent to DEF_POLY */
#endif
extern nmod_t Fq_ctx;             /* F_q */
extern fq_nmod_ctx_t Fqc_ctx;     /* F_q^c */
extern nmod_mat_t base1[VOX_C+1]; /* Translation basis to extend operator T from F_q^c to F_q */
extern nmod_mat_t base2[VOX_C+1]; /* Translation basis to extend quadratic equations from F_q^c to F_q */
extern int nb_limbs;              /* size of intermediate results in dot products */


int verify_mq(nmod_mat_t A, nmod_mat_t B);
int Solve_MQ(nmod_mat_t sol, nmod_mat_t System, const int full, mp_limb_t hint);
void GenQRKey(fq_nmod_mat_t* Key, const unsigned char *seed_Pub);
void VOX_GenS(nmod_mat_t S, const unsigned char seed_S[VOX_SEED_BYTES]);
void VOX_GenT(fq_nmod_mat_t T, const unsigned char seed_T[VOX_SEED_BYTES]);
void VOX_GenM(nmod_mat_t M, const unsigned char hpk[VOX_HPK_BYTES], const unsigned char *msg, size_t mlen);
void VOX_GenVinegar(nmod_mat_t V, mp_limb_t *hint, const unsigned char seed_V[VOX_SEED_BYTES], uint8_t ctr);
void InitContext();
void ClearContext();
void fq_nmod_mat_LowTri(fq_nmod_mat_t mat);
void nmod_mat_LowTri(nmod_mat_t mat);
void Lift(fq_nmod_t e2, mp_limb_t e1);
void ExpandMat(nmod_mat_t mat1, fq_nmod_mat_t mat2, nmod_mat_t* base);
void ExpandMat_LowTri(nmod_mat_t mat1, fq_nmod_mat_t mat2, nmod_mat_t* base);
void fq_nmod_mat_ComposeSTKey(fq_nmod_mat_t* K1, fq_nmod_mat_t* K2, nmod_mat_t S, fq_nmod_mat_t T);
void ComputePetzoldQR(fq_nmod_mat_t* Pub, nmod_mat_t S, fq_nmod_mat_t T,
                      const unsigned char *seed_PK, const unsigned char *seed_SK);
void GenVoxKeys(nmod_mat_t* Pub, nmod_mat_t* Sec, nmod_mat_t S, nmod_mat_t T,
                const unsigned char *seed_PK, const unsigned char *seed_SK);
int Sign(nmod_mat_t Signature,
         nmod_mat_t Message, nmod_mat_t* Sec, nmod_mat_t S, nmod_mat_t T,
         const unsigned char seed_V[VOX_SEED_BYTES]);
int Verify(nmod_mat_t Signature, nmod_mat_t Message, nmod_mat_t* Pub);

void encode_PK(unsigned char *pk, fq_nmod_mat_t* Pub, const unsigned char *seed_PK);
void decode_PK_expand(nmod_mat_t* Pub, const unsigned char *pk);
void encode_SK(unsigned char *sk, fq_nmod_mat_t* Sec);
void decode_SK_expand(nmod_mat_t* Sec, const unsigned char *sk);
void encode_sig(unsigned char *sig, nmod_mat_t Signature);
void decode_sig(nmod_mat_t Signature, const unsigned char *sig);

#if VERBOSE>0
void display_counters();
#endif

#endif /* _VOX_H_ */
