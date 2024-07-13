#ifndef _AURORA_INNER_H__
#define _AURORA_INNER_H__

#include <stddef.h>
#include <stdint.h>
#include <openssl/evp.h>

#include "field.h"
#include "merkle_tree.h"
#include "oracles.h"
#include "params.h"

const Parameters *get_parameters(int sig_type);

size_t aurora_proof_max_size(int sig_type);

void signal_prover_round_done(uint8_t *hash_state, MerkleTree *mt, uint8_t *mt_root,
                              const uint64_t **mt_contents, size_t mt_contents_len,
                              uint64_t *prover_message, size_t prover_message_len,
                              const Parameters *params);

void squeeze_field_elements(uint64_t *out, size_t elements_count, uint8_t round, const uint8_t *hash_state, const Parameters *params);

size_t squeeze_query_pos(size_t upper_bound, uint8_t pos_count, const uint8_t *hash_state, const Parameters *params);

void all_subset_sums(uint64_t *result, const uint64_t *basis_powers, size_t basis_power_len, const uint64_t *shift_power, const size_t field_words);

void subspace_to_power_of_two(uint64_t *result,
                              const uint64_t *subspace_basis, const size_t subspace_basis_len,
                              const uint64_t *subspace_shift,
                              const size_t power_of_two, const size_t field_words);

size_t vanishing_polynomial_len(const Domain *domain);
void vanishing_polynomial(uint64_t *result, const size_t result_len, const Domain *domain, const Parameters *params);

void get_ith_field_element(uint64_t *result, const Domain *domain, const size_t i, const Parameters *params);

void lagrange_cache(uint64_t *result, const uint64_t *interpolation_point, const Domain *domain, const Parameters *params);

size_t sort_and_unique_values(size_t *key, size_t key_len, uint64_t **values, size_t value_len, size_t value_words);

void get_all_merkle_tree_membership_proofs(MerkleTreeMembershipProof **merkle_tree_membership_proofs, size_t merkle_trees_count,
                                           uint64_t ***query_responses,
                                           size_t *query_responses_count,
                                           size_t **query_pos,
                                           Oracle **f_i_oracles,
                                           MerkleTree **merkle_trees,
                                           const Parameters *params);
int verify_all_merkle_tree_membership_proofs(size_t **unique_query_pos,
                                             MerkleTreeMembershipProof **merkle_tree_membership_proofs, uint8_t **mt_roots, size_t merkle_trees_count,
                                             uint64_t ***query_responses,
                                             size_t *query_responses_count,
                                             size_t **query_pos,
                                             const Parameters *params);

size_t merkle_tree_leaf_response_len(size_t query_reponses_len, size_t coset_serialization_size);
size_t merkle_tree_leaf_responses_len(size_t query_pos_len, size_t coset_serialization_size);
uint64_t **merkle_tree_leaf_responses_init(size_t query_pos_len, size_t query_reponses_len, size_t coset_serialization_size, size_t field_bytesize);
void merkle_tree_leaf_responses_free(uint64_t **merkle_tree_leaf_responses, size_t merkle_tree_leaf_responses_len);

void query_responses_to_merkle_tree_leaf_responses(uint64_t **merkle_tree_leaf_responses, size_t *query_pos, size_t query_pos_len, uint64_t **query_responses, size_t query_responses_len, size_t coset_serialization_size, size_t field_bytesize, size_t field_words);
#endif
