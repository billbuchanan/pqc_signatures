/******************************************************************************
 * Description of a VOX parameter set
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#ifndef VOX_PARAMS_H
#define VOX_PARAMS_H

/*
 * Deterministic signing ?
 * If enabled (this is the default), the vinegar variables are derived from
 * the message and the private key.
 *
 * Comment this line to enable randomized signing where the vinegar variables
 * are sampled at random. This saves a hash of the message (especially useful
 * for large inputs), but this means an attacker sending the same message
 * multiple times would get different signatures. The security implications
 * of such an attack were not studied yet.
 */
#define DETERMINISTIC_SIGNING

/* Size of seeds (common to all parameter sets) */
#define VOX_SEED_BYTES 32

/* Size of the PK hash for exclusive ownership property */
#define VOX_HPK_BYTES  64

/*
 * Main parameters
 *   VOX_Q   Characteristic of the base field (prime)
 *   VOX_C   Degree of the extension for the QR compression
 *   VOX_T   Number of entirely random equations
 *   VOX_OC  Number of oil variables in F_q^c
 *   VOX_VC  Number of vinegar variables in F_q^c
 *   DEF_POLY polynomial to use in FLINT to represent F_q^c (F_q^c = Fq[X] / DEF_POLY)
 */
#ifdef PARAM_SET_VOX128
#define VOX_Q      251
#define VOX_Q_BITS   8
#define VOX_C        6
#define VOX_T        6
#define VOX_OC       8
#define VOX_VC       9
#define DEF_POLY { 1, 1, 0, 0, 0, 0, 1 } /* X^6 + X + 1 */
#endif

#ifdef PARAM_SET_VOX192
#define VOX_Q      1021
#define VOX_Q_BITS   10
#define VOX_C         7
#define VOX_T         7
#define VOX_OC       10
#define VOX_VC       11
#define DEF_POLY { 5, 1, 0, 0, 0, 0, 0, 1 }  /* X^7 + X + 5 */
#endif

#ifdef PARAM_SET_VOX256
#define VOX_Q      4093
#define VOX_Q_BITS   12
#define VOX_C         8
#define VOX_T         8
#define VOX_OC       12
#define VOX_VC       13
#define DEF_POLY { 2, 0, 0, 0, 0, 0, 0, 0, 1 } /* X^8 + 2 */
#endif


#ifdef PARAM_SET_FULLVOX128
#define VOX_Q       251
#define VOX_Q_BITS    8
#define VOX_C         1
#define VOX_OC       48
#define VOX_VC       72
#define VOX_T         8
#endif

#ifdef PARAM_SET_FULLVOX192
#define VOX_Q      4093
#define VOX_Q_BITS   12
#define VOX_C         1
#define VOX_OC       68
#define VOX_VC      102
#define VOX_T         8
#endif

#ifdef PARAM_SET_FULLVOX256
#define VOX_Q     65521
#define VOX_Q_BITS   16
#define VOX_C         1
#define VOX_OC       91
#define VOX_VC      140
#define VOX_T         8
#endif


/*
 * Derived parameters
 *   VOX_NC  Total number of variables in F_q^c
 *   VOX_O   Number of oil variables in F_q
 *   VOX_V   Number of vinegar variables in F_q
 *   VOX_N   Total number of variables in F_q
 */
#define VOX_NC  (VOX_OC + VOX_VC)
#define VOX_O   (VOX_OC * VOX_C)
#define VOX_V   (VOX_VC * VOX_C)
#define VOX_N   (VOX_NC * VOX_C)

/*
 * Key sizes
 *
 * For the public key, we need to store (O-T) triangular blocks of size (OC*OC);
 * the rest is sampled pseudo-randomly from a seed.
 *
 * For the private key, we need to store T triangular blocks of size (NC*NC) plus
 * (O-T) triangular blocks of size (VC*NC) (the Oil-Oil part of those equations is zero)
 * In addition, we need to store the seed that generates the translation matrices.
 *
 * Although it is not strictly necessary for the signature, we store the public seed
 * in the secret key to allow recovering the public key from the private key.
 */
#define VOX_PUB_COEFFS   ( (VOX_O-VOX_T) * (VOX_OC * (VOX_OC+1)) / 2 )
#define VOX_SEC_COEFFS   ( VOX_O * (VOX_NC * (VOX_NC+1)) / 2  - VOX_PUB_COEFFS )
#define VOX_PUB_BITS     ( VOX_C * VOX_PUB_COEFFS * VOX_Q_BITS )
#define VOX_SEC_BITS     ( VOX_C * VOX_SEC_COEFFS * VOX_Q_BITS )
#define VOX_SIG_BITS     ( VOX_N * VOX_Q_BITS )

#define VOX_PUBLICKEYBYTES   ( VOX_SEED_BYTES + (VOX_PUB_BITS+7)/8 )
#define VOX_SECRETKEYBYTES   ( 2*VOX_SEED_BYTES + VOX_HPK_BYTES + (VOX_SEC_BITS+7)/8 )
#define VOX_SIG_BYTES        ( (VOX_SIG_BITS+7)/8 )


#endif /* VOX_PARAMS_H */
