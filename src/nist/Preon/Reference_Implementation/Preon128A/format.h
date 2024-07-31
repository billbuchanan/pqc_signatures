#ifndef _FORMAT_H__
#define _FORMAT_H__

#include <stddef.h>
#include <stdint.h>

#include "merkle_tree.h"
#include "params.h"

size_t serialize(uint8_t *sig, const size_t max_sig_len,
                 const uint64_t *prover_messages, const size_t prover_messages_len,
                 uint8_t **mt_roots, const size_t merkle_trees_count,
                 const size_t *query_responses_count,
                 uint64_t ***query_responses,
                 MerkleTreeMembershipProof **merkle_tree_membership_proofs,
                 const Parameters *params);

size_t deserialize(uint64_t *prover_messages, uint8_t **mt_roots,
                   size_t *query_responses_count, uint64_t ***query_responses,
                   MerkleTreeMembershipProof **merkle_tree_membership_proofs,
                   const uint8_t *sig, const size_t sig_len, const size_t prover_messages_len, const size_t merkle_trees_count,
                   const Parameters *params);

#endif
