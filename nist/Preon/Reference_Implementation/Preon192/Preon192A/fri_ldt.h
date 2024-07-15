#ifndef _FRI_LDT_H__
#define _FRI_LDT_H__

#include "merkle_tree.h"
#include "oracles.h"
#include "params.h"
#include "query.h"

void submit_fri_ldt_masking_poly(const Parameters *params);
void fri_ldt(uint8_t *hash_state, MerkleTree **merkle_trees, uint8_t **mt_roots,
             size_t prover_messages_len, uint64_t *prover_messages,
             Oracle **f_i_oracles, uint64_t *const multi_f_i_evaluations_by_interaction, const Parameters *params);
int fri_verify_predicate(Oracle *combined_ldt_virtual_oracle,
                         QuerySet *query_sets, const size_t query_sets_len,
                         const uint64_t *prover_messages, const size_t prover_messages_len,
                         const uint64_t *verifier_messages,
                         size_t **query_pos, size_t **unique_query_pos,
                         uint64_t ***query_responses, const size_t *query_responses_count,
                         const Parameters *params);
#endif
