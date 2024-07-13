/**
 * \file parameters.h
 * \brief Parameters of the SIGN_MIRA scheme
 */

#ifndef SIGN_MIRA_128_PARAMETER_H
#define SIGN_MIRA_128_PARAMETER_H

#define SIGN_MIRA_128_SECRET_KEY_BYTES 32           /**< Secret key size */
#define SIGN_MIRA_128_PUBLIC_KEY_BYTES 84           /**< Public key size */
#define SIGN_MIRA_128_SIGNATURE_BYTES 5640          /**< Signature size */

#define SIGN_MIRA_128_SECURITY 128                  /**< Expected security level */
#define SIGN_MIRA_128_SECURITY_BYTES 16             /**< Expected security level (in bytes) */

#define SIGN_MIRA_128_PARAM_Q 16                    /**< Parameter q of the scheme (finite field GF(q^m)) */
#define SIGN_MIRA_128_PARAM_M 16                    /**< Parameter m of the scheme (finite field GF(q^m)) */
#define SIGN_MIRA_128_PARAM_K 120                   /**< Parameter k of the scheme (code dimension) */
#define SIGN_MIRA_128_PARAM_N 16                    /**< Parameter n of the scheme (code length) */
#define SIGN_MIRA_128_PARAM_R 5                     /**< Parameter r of the scheme (rank of vectors) */

#define SIGN_MIRA_128_PARAM_N_MPC 256               /**< Parameter N of the scheme (number of parties) */
#define SIGN_MIRA_128_PARAM_N_MPC_LOG2 8            /**< Log2 of parameter N */
#define SIGN_MIRA_128_PARAM_D 8                     /**< Parameter D of the scheme (number of main parties) */
#define SIGN_MIRA_128_PARAM_TAU 18                  /**< Parameter tau of the scheme (number of iterations) */

#define SIGN_MIRA_128_PARAM_M_BYTES 8               /**< Number of bytes required to store an Fqm element */
#define SIGN_MIRA_128_VEC_K_BYTES 60                /**< Number of bytes required to store k Fq elements */
#define SIGN_MIRA_128_VEC_N_BYTES 128               /**< Number of bytes required to store a vector of size n */
#define SIGN_MIRA_128_VEC_N_PLUS_ONE_BYTES 136      /**< Number of bytes required to store a vector of size n + 1*/
#define SIGN_MIRA_128_VEC_R_BYTES 40                /**< Number of bytes required to store r Fqm elements */

#define SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES 4096    /**< Number of bytes required to store the leaf of the seed tree */
#define SIGN_MIRA_128_PARAM_TREE_PATH_BYTES 128     /**< Number of bytes required to store the authentication path of the seed tree */
#define SIGN_MIRA_128_PARAM_COMMITMENT_BYTES 8192   /**< Number of bytes required to store the commitments */
#define SIGN_MIRA_128_PARAM_STATE_BYTES 108         /**< Number of bytes required to store the state of party N */
#define SIGN_MIRA_128_PARAM_RESPONSE_BYTES 308      /**< Number of bytes required to store the response of a given iteration */

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2
#define DOMAIN_SEPARATOR_TREE 3
#define DOMAIN_SEPARATOR_COMMITMENT 4

#endif
