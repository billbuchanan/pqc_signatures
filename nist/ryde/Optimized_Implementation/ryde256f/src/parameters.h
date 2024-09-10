/**
 * \file parameters.h
 * \brief Parameters of the RYDE scheme
 */

#ifndef RYDE_256F_PARAMETER_H
#define RYDE_256F_PARAMETER_H

#define RYDE_256F_SECRET_KEY_BYTES 64            /**< Secret key size */
#define RYDE_256F_PUBLIC_KEY_BYTES 188           /**< Public key size */
#define RYDE_256F_SIGNATURE_BYTES 29134          /**< Signature size */

#define RYDE_256F_SECURITY 256                   /**< Expected security level */
#define RYDE_256F_SECURITY_BYTES 32              /**< Expected security level (in bytes) */

#define RYDE_256F_PARAM_Q 2                      /**< Parameter q of the scheme (finite field GF(q^m)) */
#define RYDE_256F_PARAM_M 43                     /**< Parameter m of the scheme (finite field GF(q^m)) */
#define RYDE_256F_PARAM_K 18                     /**< Parameter k of the scheme (code dimension) */
#define RYDE_256F_PARAM_N 47                     /**< Parameter n of the scheme (code length) */
#define RYDE_256F_PARAM_W 17                     /**< Parameter omega of the scheme (rank of vectors) */

#define RYDE_256F_PARAM_N_MPC 32                 /**< Parameter N of the scheme (number of parties) */
#define RYDE_256F_PARAM_N_MPC_LOG2 5             /**< Log2 of parameter N */
#define RYDE_256F_PARAM_D 5                      /**< Parameter D of the scheme (number of main parties) */
#define RYDE_256F_PARAM_TAU 58                   /**< Parameter tau of the scheme (number of iterations) */

#define RYDE_256F_PARAM_M_MASK 0x7               /**< Mask related to the representation of finite field elements */
#define RYDE_256F_PARAM_M_BYTES 6                /**< Number of bytes required to store a GF(2^m) element */
#define RYDE_256F_VEC_K_BYTES 97                 /**< Number of bytes required to store a vector of size k */
#define RYDE_256F_VEC_N_BYTES 253                /**< Number of bytes required to store a vector of size n */
#define RYDE_256F_VEC_W_BYTES 92                 /**< Number of bytes required to store a vector of size w */
#define RYDE_256F_VEC_W_MINUS_ONE_BYTES 86       /**< Number of bytes required to store a vector of size (w - 1) */

#define RYDE_256F_PARAM_TREE_LEAF_BYTES 1024     /**< Number of bytes required to store the leaf of the seed tree */
#define RYDE_256F_PARAM_TREE_PATH_BYTES 160      /**< Number of bytes required to store the authentication path of the seed tree */
#define RYDE_256F_PARAM_COMMITMENT_BYTES 2048    /**< Number of bytes required to store the commitments */
#define RYDE_256F_PARAM_STATE_BYTES 189          /**< Number of bytes required to store the state of party N */
#define RYDE_256F_PARAM_RESPONSE_BYTES 499       /**< Number of bytes required to store the response of a given iteration */

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2
#define DOMAIN_SEPARATOR_TREE 3
#define DOMAIN_SEPARATOR_COMMITMENT 4

#endif

