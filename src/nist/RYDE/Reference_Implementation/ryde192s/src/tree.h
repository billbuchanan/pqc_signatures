/**
 * @file ryde_192s_tree.h
 * @brief Header file for ryde_192s_tree.c
 */

#ifndef RYDE_192S_TREE_H
#define RYDE_192S_TREE_H

#include <stdint.h>
#include "hash_fips202.h"
#include "parameters.h"


typedef uint8_t ryde_192s_seed_tree_node_t[RYDE_192S_SECURITY_BYTES];
typedef ryde_192s_seed_tree_node_t ryde_192s_seed_tree_t[2 * RYDE_192S_PARAM_N_MPC - 1];

void ryde_192s_tree_expand(ryde_192s_seed_tree_t theta_tree, const uint8_t *salt, uint8_t e);
void ryde_192s_tree_expand_partial(ryde_192s_seed_tree_t partial_theta_tree, const ryde_192s_seed_tree_node_t partial_tree_seeds[RYDE_192S_PARAM_N_MPC_LOG2], const uint8_t *salt, uint8_t e, uint16_t alpha);
void ryde_192s_tree_compute_partial(ryde_192s_seed_tree_node_t partial_tree_seeds[RYDE_192S_PARAM_N_MPC_LOG2], const ryde_192s_seed_tree_t theta_tree, uint16_t alpha);

#endif
