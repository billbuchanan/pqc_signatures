/**
 * @file ryde_192f_tree.h
 * @brief Header file for ryde_192f_tree.c
 */

#ifndef RYDE_192F_TREE_H
#define RYDE_192F_TREE_H

#include <stdint.h>
#include "hash_fips202.h"
#include "parameters.h"


typedef uint8_t ryde_192f_seed_tree_node_t[RYDE_192F_SECURITY_BYTES];
typedef ryde_192f_seed_tree_node_t ryde_192f_seed_tree_t[2 * RYDE_192F_PARAM_N_MPC - 1];

void ryde_192f_tree_expand(ryde_192f_seed_tree_t theta_tree, const uint8_t *salt, uint8_t e);
void ryde_192f_tree_expand_partial(ryde_192f_seed_tree_t partial_theta_tree, const ryde_192f_seed_tree_node_t partial_tree_seeds[RYDE_192F_PARAM_N_MPC_LOG2], const uint8_t *salt, uint8_t e, uint16_t alpha);
void ryde_192f_tree_compute_partial(ryde_192f_seed_tree_node_t partial_tree_seeds[RYDE_192F_PARAM_N_MPC_LOG2], const ryde_192f_seed_tree_t theta_tree, uint16_t alpha);

#endif
