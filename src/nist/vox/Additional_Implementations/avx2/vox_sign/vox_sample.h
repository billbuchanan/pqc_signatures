/******************************************************************************
 * Pseudo-random sampling of VOX objects by extension from a seed
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#ifndef VOX_SAMPLE_H
#define VOX_SAMPLE_H

#include <stddef.h>
#include "vox_params.h"

/*
 * Generate the oil-oil block of Pub_i (assumes i < VOX_T)
 */
void VOX_GenPK_oil(Fqc Pub_i[VOX_N*VOX_N], const unsigned char seed[VOX_SEED_BYTES], int i);

/*
 * Same as above, but samples 4 matrices in parallel using SIMD instructions.
 * Safe to use even if i+3 >= VOX_T
 */
void VOX_GenPK_oil_x4(Fqc *Pub_i[4], const unsigned char seed[VOX_SEED_BYTES], int i);

/*
 * Generate the vinegar-oil and vinegar-vinegar blocks of Pub_i
 */
void VOX_GenPK_vin(Fqc Pub_i[VOX_N*VOX_N], const unsigned char seed[VOX_SEED_BYTES], int i);

/*
 * Same as above, but samples 4 matrices in parallel using SIMD instructions.
 * Safe to use even if i+3 >= VOX_O
 */
void VOX_GenPK_vin_x4(Fqc *Pub_i[4], const unsigned char seed[VOX_SEED_BYTES], int i);

/*
 * Generate the non-constrained parts of the public key, that is
 *   - the oil-oil block for Pub_i (i < VOX_T)
 *   - the vinegar-oil and vinegar-vinegar blocks of Pub_i (i < VOX_O)
 */
void VOX_GenPK_template(Fqc **Pub, const unsigned char seed[VOX_SEED_BYTES]);

/*
 * Generate the matrix S used to mix the equations. S is of the form
 *   [ I_t     S'   ]
 *   [  0   I_(h-t) ]
 * and only S' is stored to save space.
 */
void VOX_GenS(Fq  S[VOX_T*(VOX_O-VOX_T)], const unsigned char seed[VOX_SEED_BYTES]);

/*
 * Generate the matrix T used to mix the variables. T is of the form
 *   [ I_oc   T' ]
 *   [  0    I_vc ]
 * and only T' is stored to save space.
 */
void VOX_GenT(Fqc T[VOX_OC*VOX_VC], const unsigned char seed[VOX_SEED_BYTES]);

/*
 * Generate the target vector corresponding to the input message
 * Includes the hash of the public key for exclusive ownership property.
 */
void VOX_GenM(Fq M[VOX_O],
              const unsigned char hpk[VOX_HPK_BYTES],
              const unsigned char *msg, size_t mlen);

/*
 * Generate the vinegar variables, plus a hint to select the root in univariate resolution.
 * A counter is used to sample new variables for each signature attempt.
 */
void VOX_GenVinegar(Fq V[VOX_V], Fq *hint, const unsigned char seed[VOX_SEED_BYTES], uint8_t ctr);


#endif /* VOX_SAMPLE_H */
