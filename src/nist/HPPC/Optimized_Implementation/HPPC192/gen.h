#ifndef GEN_H
#define GEN_H

#include <m4ri/m4ri.h>
#include <flint/flint.h>
#include <flint/fq.h>
#include <flint/fq_vec.h>
#include <flint/fq_poly.h>
#include <flint/fq_poly_factor.h>
#include <flint/fmpz_vec.h>
#include <flint/fmpz_mod_poly.h>
#include <flint/fmpz_mod_mat.h>

extern fmpz_t p;
extern fq_ctx_t ctx;
extern flint_rand_t state;

void GenRndMat(mzd_t* mat);
void GenVandermonde(fq_mat_t mat);
void Ident(mzd_t *mat);
void GenRndLinPermPolyMat(mzd_t *mat, fq_poly_t private_polynomial, fq_mat_t vander);
void GenerateBasis(mzd_t *mat);
void AdjustPublicKey(mzd_t *pub_key, mzd_t *short_pk);
void KroneckerProductMat(const mzd_t *M1, const mzd_t *M2, mzd_t *tens_M1_M2);
void KroneckerProductVec(const mzd_t *v1, const mzd_t *v2, mzd_t *tens_v1_v2);
void GenPublicKey(mzd_t *pub_key, const mzd_t *M, const mzd_t *T, const mzd_t *S, const mzd_t *L1, const mzd_t *L2);

#endif