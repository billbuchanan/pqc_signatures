#ifndef GEN_H
#define GEN_H

#include "flint.h"
#include <flint/fmpz_vec.h>
#include <flint/fmpz_mod_mat.h>

extern fmpz_t p;
extern flint_rand_t state;

void KroneckerProductVec(const fmpz *v1, fmpz *tens_v1_v1_v1);
void FaceSplitProd(const fmpz_mod_mat_t M1, fmpz_mod_mat_t prod);
void GeneratePublicKey(fmpz_mod_mat_t pub_key, fmpz_mod_mat_t inv_T, fmpz_mod_mat_t inv_S);
void AdjustPublicKey(const fmpz_mod_mat_t pub_key, fmpz_mod_mat_t short_pk);

#endif