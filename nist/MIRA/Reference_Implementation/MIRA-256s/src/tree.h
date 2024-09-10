/**
 * @file sign_mira_256_tree.h
 * @brief Header file for sign_mira_256_tree.c
 */

#ifndef SIGN_MIRA_256_TREE_H
#define SIGN_MIRA_256_TREE_H

#include <stdint.h>
#include "hash_fips202.h"
#include "parameters.h"

#define DOMAIN_SEPARATOR_TREE       3

typedef uint8_t sign_mira_256_seed_tree_node_t[SIGN_MIRA_256_SECURITY_BYTES];
typedef sign_mira_256_seed_tree_node_t sign_mira_256_seed_tree_t[2 * SIGN_MIRA_256_PARAM_N_MPC - 1];

void sign_mira_256_tree_expand(sign_mira_256_seed_tree_t theta_tree, const uint8_t *salt);

void sign_mira_256_tree_expand_partial(sign_mira_256_seed_tree_t partial_theta_tree, const sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
                                        const uint8_t *salt, uint16_t alpha);

void sign_mira_256_tree_compute_partial(sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2],
                                           const sign_mira_256_seed_tree_t theta_tree, uint16_t alpha);

#endif
