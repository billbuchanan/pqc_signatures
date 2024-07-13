/**
 * \file parameters.h
 * \brief Parameters of the RYDE scheme
 */

#ifndef RYDE_128S_PARAMETER_H
#define RYDE_128S_PARAMETER_H

#define RYDE_128S_SECRET_KEY_BYTES 32           /**< Secret key size */
#define RYDE_128S_PUBLIC_KEY_BYTES 86           /**< Public key size */
#define RYDE_128S_SIGNATURE_BYTES 5956          /**< Signature size */

#define RYDE_128S_SECURITY 128                  /**< Expected security level */
#define RYDE_128S_SECURITY_BYTES 16             /**< Expected security level (in bytes) */

#define RYDE_128S_PARAM_Q 2                     /**< Parameter q of the scheme (finite field GF(q^m)) */
#define RYDE_128S_PARAM_M 31                    /**< Parameter m of the scheme (finite field GF(q^m)) */
#define RYDE_128S_PARAM_K 15                    /**< Parameter k of the scheme (code dimension) */
#define RYDE_128S_PARAM_N 33                    /**< Parameter n of the scheme (code length) */
#define RYDE_128S_PARAM_W 10                    /**< Parameter omega of the scheme (rank of vectors) */

#define RYDE_128S_PARAM_N_MPC 256               /**< Parameter N of the scheme (number of parties) */
#define RYDE_128S_PARAM_N_MPC_LOG2 8            /**< Log2 of parameter N */
#define RYDE_128S_PARAM_D 8                     /**< Parameter D of the scheme (number of main parties) */
#define RYDE_128S_PARAM_TAU 20                  /**< Parameter tau of the scheme (number of iterations) */

#define RYDE_128S_PARAM_M_MASK 0x7f             /**< Mask related to the representation of finite field elements */
#define RYDE_128S_PARAM_M_BYTES 4               /**< Number of bytes required to store a GF(2^m) element */
#define RYDE_128S_VEC_K_BYTES 59                /**< Number of bytes required to store a vector of size k */
#define RYDE_128S_VEC_N_BYTES 128               /**< Number of bytes required to store a vector of size n */
#define RYDE_128S_VEC_W_BYTES 39                /**< Number of bytes required to store a vector of size w */
#define RYDE_128S_VEC_W_MINUS_ONE_BYTES 35      /**< Number of bytes required to store a vector of size (w - 1) */

#define RYDE_128S_PARAM_TREE_LEAF_BYTES 4096    /**< Number of bytes required to store the leaf of the seed tree */
#define RYDE_128S_PARAM_TREE_PATH_BYTES 128     /**< Number of bytes required to store the authentication path of the seed tree */
#define RYDE_128S_PARAM_COMMITMENT_BYTES 8192   /**< Number of bytes required to store the commitments */
#define RYDE_128S_PARAM_STATE_BYTES 98          /**< Number of bytes required to store the state of party N */
#define RYDE_128S_PARAM_RESPONSE_BYTES 293      /**< Number of bytes required to store the response of a given iteration */

#define DOMAIN_SEPARATOR_MESSAGE 0
#define DOMAIN_SEPARATOR_HASH1 1
#define DOMAIN_SEPARATOR_HASH2 2
#define DOMAIN_SEPARATOR_TREE 3
#define DOMAIN_SEPARATOR_COMMITMENT 4

#endif
