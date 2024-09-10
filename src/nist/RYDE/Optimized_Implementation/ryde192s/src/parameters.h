/**
 * \file parameters.h
 * \brief Parameters of the RYDE scheme
 */

#ifndef RYDE_192S_PARAMETER_H
#define RYDE_192S_PARAMETER_H

#define RYDE_192S_SECRET_KEY_BYTES 48            /**< Secret key size */
#define RYDE_192S_PUBLIC_KEY_BYTES 131           /**< Public key size */
#define RYDE_192S_SIGNATURE_BYTES 12933          /**< Signature size */

#define RYDE_192S_SECURITY 192                   /**< Expected security level */
#define RYDE_192S_SECURITY_BYTES 24              /**< Expected security level (in bytes) */

#define RYDE_192S_PARAM_Q 2                      /**< Parameter q of the scheme (finite field GF(q^m)) */
#define RYDE_192S_PARAM_M 37                     /**< Parameter m of the scheme (finite field GF(q^m)) */
#define RYDE_192S_PARAM_K 18                     /**< Parameter k of the scheme (code dimension) */
#define RYDE_192S_PARAM_N 41                     /**< Parameter n of the scheme (code length) */
#define RYDE_192S_PARAM_W 13                     /**< Parameter omega of the scheme (rank of vectors) */

#define RYDE_192S_PARAM_N_MPC 256                /**< Parameter N of the scheme (number of parties) */
#define RYDE_192S_PARAM_N_MPC_LOG2 8             /**< Log2 of parameter N */
#define RYDE_192S_PARAM_D 8                      /**< Parameter D of the scheme (number of main parties) */
#define RYDE_192S_PARAM_TAU 29                   /**< Parameter tau of the scheme (number of iterations) */

#define RYDE_192S_PARAM_M_MASK 0x1f              /**< Mask related to the representation of finite field elements */
#define RYDE_192S_PARAM_M_BYTES 5                /**< Number of bytes required to store a GF(2^m) element */
#define RYDE_192S_VEC_K_BYTES 84                 /**< Number of bytes required to store a vector of size k */
#define RYDE_192S_VEC_N_BYTES 190                /**< Number of bytes required to store a vector of size n */
#define RYDE_192S_VEC_W_BYTES 61                 /**< Number of bytes required to store a vector of size w */
#define RYDE_192S_VEC_W_MINUS_ONE_BYTES 56       /**< Number of bytes required to store a vector of size (w - 1) */

#define RYDE_192S_PARAM_TREE_LEAF_BYTES 6144     /**< Number of bytes required to store the leaf of the seed tree */
#define RYDE_192S_PARAM_TREE_PATH_BYTES 192      /**< Number of bytes required to store the authentication path of the seed tree */
#define RYDE_192S_PARAM_COMMITMENT_BYTES 12288   /**< Number of bytes required to store the commitments */
#define RYDE_192S_PARAM_STATE_BYTES 145          /**< Number of bytes required to store the state of party N */
#define RYDE_192S_PARAM_RESPONSE_BYTES 441       /**< Number of bytes required to store the response of a given iteration */

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2
#define DOMAIN_SEPARATOR_TREE 3
#define DOMAIN_SEPARATOR_COMMITMENT 4

#endif
