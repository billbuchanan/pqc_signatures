/**
 * \file parameters.h
 * \brief Parameters of the SIGN_MIRA scheme
 */

#ifndef SIGN_MIRA_192_PARAMETER_H
#define SIGN_MIRA_192_PARAMETER_H

#define SIGN_MIRA_192_SECRET_KEY_BYTES 48           /**< Secret key size */
#define SIGN_MIRA_192_PUBLIC_KEY_BYTES 121          /**< Public key size */
#define SIGN_MIRA_192_SIGNATURE_BYTES 15560         /**< Signature size */

#define SIGN_MIRA_192_SECURITY 192                  /**< Expected security level */
#define SIGN_MIRA_192_SECURITY_BYTES 24             /**< Expected security level (in bytes) */

#define SIGN_MIRA_192_PARAM_Q 16                    /**< Parameter q of the scheme (finite field GF(q^m)) */
#define SIGN_MIRA_192_PARAM_M 19                    /**< Parameter m of the scheme (finite field GF(q^m)) */
#define SIGN_MIRA_192_PARAM_K 168                   /**< Parameter k of the scheme (code dimension) */
#define SIGN_MIRA_192_PARAM_N 19                    /**< Parameter n of the scheme (code length) */
#define SIGN_MIRA_192_PARAM_R 6                     /**< Parameter r of the scheme (rank of vectors) */

#define SIGN_MIRA_192_PARAM_N_MPC 32                /**< Parameter N of the scheme (number of parties) */
#define SIGN_MIRA_192_PARAM_N_MPC_LOG2 5            /**< Log2 of parameter N */
#define SIGN_MIRA_192_PARAM_D  5                    /**< Parameter D of the scheme (number of main parties) */
#define SIGN_MIRA_192_PARAM_TAU 41                  /**< Parameter tau of the scheme (number of iterations) */

#define SIGN_MIRA_192_PARAM_M_BYTES 10              /**< Number of bytes required to store an Fqm element */
#define SIGN_MIRA_192_VEC_K_BYTES 84                /**< Number of bytes required to store k Fq elements */
#define SIGN_MIRA_192_VEC_N_BYTES 181               /**< Number of bytes required to store a vector of size n*/
#define SIGN_MIRA_192_VEC_N_PLUS_ONE_BYTES 190      /**< Number of bytes required to store a vector of size n + 1*/
#define SIGN_MIRA_192_VEC_R_BYTES 57                /**< Number of bytes required to store r Fqm elements */

#define SIGN_MIRA_192_PARAM_TREE_LEAF_BYTES 768     /**< Number of bytes required to store the leaf of the seed tree */
#define SIGN_MIRA_192_PARAM_TREE_PATH_BYTES 120     /**< Number of bytes required to store the authentication path of the seed tree */
#define SIGN_MIRA_192_PARAM_COMMITMENT_BYTES 1536   /**< Number of bytes required to store the commitments */
#define SIGN_MIRA_192_PARAM_STATE_BYTES 151         /**< Number of bytes required to store the state of party N */
#define SIGN_MIRA_192_PARAM_RESPONSE_BYTES 376      /**< Number of bytes required to store the response of a given iteration */

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2
#define DOMAIN_SEPARATOR_TREE 3
#define DOMAIN_SEPARATOR_COMMITMENT 4

#endif
