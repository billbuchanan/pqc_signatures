#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "flint.h"
#include <flint/fmpz_vec.h>
#include <flint/fmpz_mod_mat.h>

void EncodePublicKey(const fmpz_mod_mat_t pub_key, unsigned char *pk);
void DecodePublicKey(fmpz_mod_mat_t pub_key, const unsigned char *pk);
void EncodePrivateKey(const fmpz_mod_mat_t inv_T, const fmpz_mod_mat_t inv_S, unsigned char *sk);
void DecodePrivateKey(fmpz_mod_mat_t inv_T, fmpz_mod_mat_t inv_S, const unsigned char *sk);

#endif