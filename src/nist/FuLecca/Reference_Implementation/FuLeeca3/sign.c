#include "sign.h"
#include "poly.h"
#include "rng.h"
#include "fips202.h"
#include "utils.h"
#include "params.h"
#include "encode.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


/*************************************************
* Name:        crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
**************************************************/
int crypto_sign_keypair(uint8_t *pk,
                        uint8_t *sk)
{
  //Declare varibles
  poly_n_2 a = {0}, b = {0}, verif_inverse = {0}, a_inv = {0}, a_inv_b = {0};
  uint8_t seed[SEEDBYTES];

  bool inverse_exists = 0, inv_check = 0;

  //Get random seed and initialize shake256 with it
  randombytes(seed, sizeof(seed));
  keccak_state state;
  
  shake256_init(&state);
  shake256_absorb(&state, seed, SEEDBYTES);
  shake256_finalize(&state);

  //Sample b, as be does not need to be invertible we only have to do this once
  poly_sample_from_typical_set(&b, &state);

  //Repeat until an invertible a is found
  while((inverse_exists == 0) || (inv_check != 0))
  {
    //Set candidate a and inversion check to zero 
    inv_check = 0;
    memset(&(a.coeffs), 0, sizeof(a.coeffs));
    //Sample a
    poly_sample_from_typical_set(&a, &state);
  
    //Compute inverse of a
    extended_euclidean_algorithm(&a_inv, &a);

    //Multiply a with its inverse
    poly_mul_modp(&verif_inverse, &a_inv, &a);

    //Check that the output is one, i.e. that a_inv is indeed the inverse of a
    inverse_exists = (verif_inverse.coeffs[0] == 1);
    for(int i = 1; i < N/2; i++)
    {
      inv_check |= verif_inverse.coeffs[i];
    }
  }
  
  //Calculate the public key by multiplying a_inv with b
  poly_mul_modp(&a_inv_b, &a_inv, &b);

  //Copy sk and ok into output buffer
  memcpy(sk, &(a.coeffs), sizeof(a.coeffs));
  memcpy(sk + sizeof(a.coeffs), &(b.coeffs), sizeof(b.coeffs));
  memcpy(pk, &(a_inv_b.coeffs), sizeof(a_inv_b.coeffs));

  return 0;
}

/*************************************************
* Name:        crypto_sign_signature
*
* Description: Computes signature.
*
* Arguments:   - uint8_t *sig:   pointer to output signature (of length CRYPTO_BYTES)
*              - size_t *siglen: pointer to output length of signature
*              - uint8_t *m:     pointer to message to be signed
*              - size_t mlen:    length of message
*              - uint8_t *sk:    pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
int crypto_sign_signature(uint8_t *sig,
                          size_t *siglen,
                          const uint8_t *m,
                          size_t mlen,
                          const uint8_t *sk) {
  //Variable declaration
  poly_n_2 a, b;
  poly_s_n gsec[N/2];
  uint8_t exp_chall[N];
  poly_s_n_2 a_signed, b_signed;
  poly_n v_unsig = {0};
  poly_s_d_n v = {0}, v_bar = {0};
  int score_per_row[N/2];
  bool signature_valid = 0;
  size_t i;
  keccak_state state, salt_state;
  uint8_t seed[SEEDBYTES], salt[SALT_BYTES];
  uint8_t mhash[N/8 + 1];
  uint8_t hash[SHA3_LENGTH];
  size_t lee_weight;
  uint8_t allowed_indexes[N];
  uint8_t loopfree = 1;
  int32_t cand_lmp, cand_lmp_bar = 0;
  size_t opt_steps = 0;
  
  //Copy sk to polys
  memcpy(&(a.coeffs), sk, sizeof(a.coeffs));
  memcpy(&(b.coeffs), sk + sizeof(a.coeffs), sizeof(b.coeffs));
  //Convert polys to signed representation for efficient signing
  poly_n_2_to_poly_s_n_2(&a_signed, &a);
  poly_n_2_to_poly_s_n_2(&b_signed, &b);

  //Precalculate key matrix incl. shifts w/o sign switch
  expand_key(gsec, &a_signed, &b_signed);

  randombytes(seed, sizeof(seed));

  //Initialize Keccak for salt squeezing
  shake256_init(&salt_state);
  shake256_absorb(&salt_state, seed, SEEDBYTES);
  shake256_finalize(&salt_state);

  //Hash message
#if FULEECA_MODE == 1
  sha3_256(hash, m, mlen);
#elif FULEECA_MODE == 3
  sha3_384(hash, m, mlen);
#elif FULEECA_MODE == 5
  sha3_512(hash, m, mlen);
#endif

  //Repeat until a valid signature is found
  while(signature_valid == 0) {
    //Nullify any previous signature attempts
    memset(&(v_unsig.coeffs), 0, sizeof(v_unsig.coeffs));
    memset(&(v.coeffs), 0, sizeof(v.coeffs));
    memset(&(v_bar.coeffs), 0, sizeof(v_bar.coeffs));
    //Reset allowed_indexes as all rows may be added/subtracted for new candidate
    memset(&(allowed_indexes), 1, sizeof(allowed_indexes));
    //Don't allow previous adds/subs to be undone
    loopfree = 0;
    //Init the opt_steps variable, i.e.: number of concentrating runs allowed
    opt_steps = 0;

    //Squeeze a new salt
    shake256_squeeze(salt, SALT_BYTES, &salt_state);
  
    //Init CSPRNG to generate challenge
    shake256_init(&state);
    //Absorb perhashed message
    shake256_absorb(&state, hash, SHA3_LENGTH);
    //Absorb the new salt
    shake256_absorb(&state, salt, SALT_BYTES);
    shake256_finalize(&state);

    //Squeeze the new message challenge
    shake256_squeeze(mhash, N/8+1, &state);
    
    //Expand challenge bits to avoid costly shifts in loop
    expand_chall(exp_chall, mhash);

    //Start simple sign, determine how often which row is added/subtracted
    simple_sign_score(score_per_row, &a, &b, mhash);

    //Generate the signature candidate from simple sign
    gen_signature_candidate(&v_unsig, &a, &b, score_per_row);
    //Convert to signed signature to allow lazy reduction for candidates
    poly_n_to_poly_s_d_n(&v, &v_unsig);
    
    //Calculate the lee weight of this initial candidate
    lee_weight = 0;
    for(i = 0; i < N; i++)
    {
      lee_weight += s_d_coeff_lee_weight(v.coeffs[i]);
    }

    //Concentrating starts, i.e. do a fixed number of iterations to concentrate the candidate
    while(opt_steps < OPTS)
    {
      //Increment the counter
      opt_steps++;

      //Store previous result (we might need it if the new one has a too large lee weight)
      memcpy(&(v_bar.coeffs), &(v.coeffs), sizeof(v_bar.coeffs));
      cand_lmp_bar = cand_lmp;

      //Calculate improvements for one added/subtracted row
      find_improvement(&v, allowed_indexes, gsec, exp_chall, &cand_lmp, loopfree);

      //Calculate the lee weight of the new candidate
      lee_weight = 0;
      for(i = 0; i < N; i++)
      {
        lee_weight += s_d_coeff_lee_weight(v.coeffs[i]);
      }
      //Allow previous instructions to be undone in case the lee weight of the candidate is the LOOP_THRESHOLD
      loopfree = (lee_weight > LOOP_THRESHOLD);
    }
    //Lazy reduction of v_bar, needs recomputation of lmp and lee_weight afterwards in case we actually were using a not reduced value beforehand
    for( i = 0; i < N; i++)
    {
      v_unsig.coeffs[i] = coeff_red_modp((d_coeff)(v_bar.coeffs[i] + 260*P));
    }
    //Recalculate lee weight and LMP
    lee_weight = 0;
    calc_matches_and_rel_weight_verify(&cand_lmp_bar, &lee_weight, &v_unsig, mhash);
    //Determine whether the generated candidate can be published as signature, restart otherwise
    if(cand_lmp_bar >= LAMBDA + 64 && lee_weight < TARGET_WEIGHT && lee_weight >= TARGET_WEIGHT - 2*KEYWEIGHT)
    {
      signature_valid = 1;
    }
  }
  // Signature compression
  s_coeff sig_sign[N/2];
  uint8_t sig_comp[CRYPTO_BYTES] = {0};
  size_t sig_comp_len = 0;
  poly_to_signed(sig_sign, &v_unsig, N/2);
  encode_sig(sig_comp, &sig_comp_len, sig_sign);

  //Copy the salt and the signature to the output buffer
  memcpy(sig, salt, SALT_BYTES);
  memcpy(sig+SALT_BYTES, &sig_comp, CRYPTO_BYTES-SALT_BYTES);
  //Set siglen to the correct value
  *siglen = CRYPTO_BYTES;

  //Always return 0, i.e. sign can not fail
  return 0;
}

/*************************************************
* Name:        crypto_sign
*
* Description: Compute signed message.
*
* Arguments:   - uint8_t *sm: pointer to output signed message (allocated
*                             array with CRYPTO_BYTES + mlen bytes),
*                             can be equal to m
*              - size_t *smlen: pointer to output length of signed
*                               message
*              - const uint8_t *m: pointer to message to be signed
*              - size_t mlen: length of message
*              - const uint8_t *sk: pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
int crypto_sign(uint8_t *sm,
                size_t *smlen,
                const uint8_t *m,
                size_t mlen,
                const uint8_t *sk)
{
  size_t i;
  for(i = 0; i < mlen; i++)
  {
    sm[CRYPTO_BYTES + mlen - 1 -i] = m[mlen - 1 -i];
  }
  crypto_sign_signature(sm, smlen, sm + CRYPTO_BYTES, mlen, sk);
  *smlen += mlen;
  return 0;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_verify(const uint8_t *sig,
                       size_t siglen,
                       const uint8_t *m,
                       size_t mlen,
                       const uint8_t *pk)
{
  //Init variables
  size_t i;
  poly_n_2 pk_poly, sig_poly, sys_sig;
  poly_n full_sig;
  keccak_state state;
  uint8_t mhash[N/8 + 1];
  uint8_t hash[SHA3_LENGTH];
  uint8_t salt[SALT_BYTES];

  //Extract salt
  memcpy(salt, sig, SALT_BYTES);

  //Calculate message hash
#if FULEECA_MODE == 1
  sha3_256(hash, m, mlen);
#elif FULEECA_MODE == 3
  sha3_384(hash, m, mlen);
#elif FULEECA_MODE == 5
  sha3_512(hash, m, mlen);
#endif

  //Calculate challenge to verify sig. against from message hash and salt
  shake256_init(&state);
  shake256_absorb(&state, hash, SHA3_LENGTH);
  shake256_absorb(&state, salt, SALT_BYTES);
  shake256_finalize(&state);
  shake256_squeeze(mhash, N/8 + 1, &state);

  //Extract pk and signature from corresponding arrays
  memcpy(&(pk_poly.coeffs), pk, sizeof(pk_poly.coeffs));

  // Decompress signature
  s_coeff sig_signed[N/2] = {0};
  decode_sig(sig_signed, &sig[sizeof(salt)]);
  signed_to_poly(&sig_poly, sig_signed, N/2);

  //Reconstruct second part of signature from key and first part, i.e.: (sig * pk) % phi
  poly_mul_modp(&sys_sig, &sig_poly, &pk_poly);

  //Merge signature parts
  for(i = 0; i < N/2; i++) {
    full_sig.coeffs[i] = sig_poly.coeffs[i];
    full_sig.coeffs[N/2 + i] = sys_sig.coeffs[i];
  }

  //Check for lmp and calculate lee weight
  size_t lee_weight;
  int32_t lmp;
  calc_matches_and_rel_weight_verify(&lmp, &lee_weight, &full_sig, mhash);

  //Valid signature if lmp and lee weight match expectations
  if(lee_weight <= TARGET_WEIGHT && lmp >= LAMBDA + 64) {
    return 0;
  }

  //Otherwise 
  return -1;
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m: pointer to output message (allocated
*                            array with smlen bytes), can be equal to sm
*              - size_t *mlen: pointer to output length of message
*              - const uint8_t *sm: pointer to signed message
*              - size_t smlen: length of signed message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signed message could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_open(uint8_t *m,
                     size_t *mlen,
                     const uint8_t *sm,
                     size_t smlen,
                     const uint8_t *pk)
{
  size_t i;

  if(smlen < CRYPTO_BYTES) {
    *mlen = -1;
    for(i = 0; i < smlen; i++)
    {
      m[i] = 0;
    }
    return -1;
  }
  *mlen = smlen - CRYPTO_BYTES;
  if(crypto_sign_verify(sm, CRYPTO_BYTES, sm + CRYPTO_BYTES, *mlen, pk))
  {
    *mlen = -1;
    for(i = 0; i < smlen - CRYPTO_BYTES; i++)
    {
      m[i] = 0;
    }
    return -1;  
  }
  for(i = 0; i < *mlen; i++)
  {
    m[i] = sm[CRYPTO_BYTES + i];
  }
  return 0;
}
