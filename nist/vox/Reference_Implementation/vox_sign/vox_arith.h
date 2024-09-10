/******************************************************************************
 * Helper functions for linear algebra over F_q and F_q^c
 * Matrices S and T are actually of the form
 *   [ I_t    S'   ]     [ I_o  T' ]
 *   [  0  I_(o-t) ] and [  0  I_v ]
 * but to save space we only store the blocks S' and T'
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#ifndef VOX_ARITH_H
#define VOX_ARITH_H

#include <stddef.h>
#include <stdint.h>
#include "vox_params.h"


/* Number of terms in a quadratic system in VOX_T variables */
#define MQ_T_TERMS  (((VOX_T+1)*(VOX_T+2))/2)


/*
 * Compute the constrained part of the VOX public key, assuming the non-constrained part
 * is already set.
 * Aux is an array of VOX_O matrices of size VOX_O x VOX_O used as temporary space.
 * In the end, Aux[0:VOX_T] contain the oil-oil blocks of the secret key.
 * Used for Key generation.
 */
void VOX_CompletePetzoldPubkey(Fqc **Pub, Fqc **Aux,
                               const Fq  S[VOX_T*(VOX_O-VOX_T)],
                               const Fqc T[VOX_OC*VOX_VC]);


/*
 * From the public key and the private transforms S,T, compute the private key;
 * the result is written in Pub.
 * Assumes the oil-oil blocks for the first VOX_T equations are already computed
 * in Aux[0:T], as the secondary output of VOX_CompletePetzoldPubkey.
 * Used for key generation.
 */
void VOX_ComposeST(Fqc **Pub, Fqc **Aux,
                   const Fq  S[VOX_T*(VOX_O-VOX_T)],
                   const Fqc T[VOX_OC*VOX_VC]);


/*
 * Expand an element of F_q^c into a c*c block over Fq such that
 * for any X1,X2, we have X1 * A * tX2 = Tr(a*x1*x2).
 * dst points to the first element of the block, which is part of
 * a larger matrix with ncols columns.
 * Used to expand the secret key prior to signature.
 */
void Fqc_to_block(Fq dst[], const Fqc *a, size_t ncols);

/*
 * Same as above but fold the result as a lower triangular block.
 * For any X, we have X * A * tX = Tr(a*x*x)
 * Used to expand the secret key prior to signature.
 */
void Fqc_to_LowTriBlock(Fq dst[], const Fqc *a, size_t ncols);


/*
 * Mix coefficients in the target vector using the matrix S to match
 * the VOX transformation between purely random equations and UOV equations
 * Used for signature
 */
void Fq_InjectS(Fq target[VOX_O], const Fq S[VOX_T*(VOX_O-VOX_T)]);

/*
 * Mix coefficients in the signature vector to hide oil and vinegar variables,
 * as usual with UOV
 * Used as final step of the signature
 */
void Fq_InjectT(Fq signature[VOX_N], const Fqc T[VOX_OC*VOX_VC]);


/*
 * Inject given vinegar variables into the quadratic equation to obtain an affine
 * relation that must be verified by the oil variables.
 * Output coefficients are reduced modulo q, but packed in an uint32[] to compute the
 * reduced row echelon form later.
 * Used for signature
 */
void Fq_GetLinEqn(uint32_t dst[VOX_O+1], const Fq mat[VOX_V*VOX_N], const Fq V[VOX_V], const Fq target);


/*
 * Solve the given affine equations by computing the reduced row echelon form.
 * Return the indices of the pivot and non-pivot columns in the "pivots" array:
 *   - the index of the i-th pivot column is in pivots[i]
 *   - the index of the i-th non-pivot column is in pivots[VOX_O+1-i]
 * Input and output coefficients are reduced modulo q, but the working array should
 * be a uint32[] so we can store intermediate results without reallocation.
 * Used for signature
 */
int rref_with_pivots(uint32_t eqns[(VOX_O-VOX_T)*(VOX_O+1)], int pivots[VOX_O+1]);


/*
 * Compute the reduced row echelon form of block mat[i:i+k][j:j+l] that must be
 * of the form [ I_k M' ], assuming l >= k.
 *   - If the rref is of this form, returns k, and mat is updated accordingly.
 *   - Otherwise, returns a value <k, and mat is in an inconsistent state.
 * Input and output coefficients are reduced modulo q, but the working array should
 * be a uint32[] so we can store intermediate results without reallocation.
 * Subroutine of the F4 algorithm used in signature.
 */
int rref_block_strict(uint32_t mat[], int ncols, int i, int j, int k, int l);


/*
 * Compute a quadratic equation in VOX_T variables (grevlex order).
 * Injecting the vinegar variables gives an equation in VOX_O variables.
 * Assuming Ker defines the affine relations between the oil variables, we can
 * reduce the equation to VOX_T variables as expected.
 * Used for signature
 */
void Fq_GetMQEqn(Fq dst[MQ_T_TERMS], const Fq mat[VOX_N*VOX_N],
                 const Fq V[VOX_V], const Fq Ker[VOX_O*(VOX_T+1)], const Fq target);


/*
 * From a solution of a system in VOX_T variables, reconstruct the VOX_O oil variables:
 * dst = Ker * (X, 1).
 * Used for signature
 */
void Fq_GetOilVars(Fq dst[VOX_O], const Fq Ker[VOX_O*(VOX_T+1)], const Fq X[VOX_T]);


/*
 * Evaluate a multivariate quadratic system: Tr(eqn(Phi(vars)))
 * Used for verification.
 */
Fq Fqc_EvalMQSystem(const Fqc eqn[VOX_NC*VOX_NC], const Fq vars[VOX_N]);

#endif /* VOX_ARITH_H */
