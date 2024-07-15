#include "format.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "merkle_tree.h"
#include "params.h"

size_t serialize(uint8_t *sig, const size_t max_sig_len,
                 const uint64_t *prover_messages, const size_t prover_messages_len,
                 uint8_t **mt_roots, const size_t merkle_trees_count,
                 const size_t *query_responses_count,
                 uint64_t ***query_responses,
                 MerkleTreeMembershipProof **merkle_tree_membership_proofs,
                 const Parameters *params)
{
    // result.total_depth_without_pruning not serializing, probably won't be required by verifier
    // Easy part: fill in (explicit) prover messages and MT roots.
    size_t cur = 0;
    assert((cur + sizeof(size_t)) <= max_sig_len);
    memcpy(sig + cur, (uint8_t *)&prover_messages_len, sizeof(size_t));
    cur += sizeof(size_t);
    assert((cur + prover_messages_len * params->field_bytesize) <= max_sig_len);
    memcpy(sig + cur, (uint8_t *)prover_messages, prover_messages_len * params->field_bytesize);
    cur += prover_messages_len * params->field_bytesize;
    // result.proof_of_work_ = pow_answer;

    // MT_roots is in mt_roots
    assert((cur + sizeof(size_t)) <= max_sig_len);
    memcpy(sig + cur, (uint8_t *)&merkle_trees_count, sizeof(size_t));
    cur += sizeof(size_t);
    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        assert((cur + params->hash_bytesize) <= max_sig_len);
        memcpy(sig + cur, mt_roots[i], params->hash_bytesize);
        cur += params->hash_bytesize;
    }

    // query_responses
    // merkle_trees_count seems serializaing multiple times, can it be only serialized once?
    assert((cur + sizeof(size_t)) <= max_sig_len);
    memcpy(sig + cur, (uint8_t *)&merkle_trees_count, sizeof(size_t));
    cur += sizeof(size_t);
    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        assert((cur + sizeof(size_t)) <= max_sig_len);
        memcpy(sig + cur, (uint8_t *)&query_responses_count[i], sizeof(size_t));
        cur += sizeof(size_t);
        for (size_t j = 0; j < query_responses_count[i]; j++)
        {
            size_t query_responses_count_per_pos = 1;
            if (i == 0)
                query_responses_count_per_pos = 6;
            assert((cur + sizeof(size_t)) <= max_sig_len);
            memcpy(sig + cur, (uint8_t *)&query_responses_count_per_pos, sizeof(size_t));
            cur += sizeof(size_t);
            assert((cur + query_responses_count_per_pos * params->field_bytesize) <= max_sig_len);
            memcpy(sig + cur, (uint8_t *)query_responses[i][j], query_responses_count_per_pos * params->field_bytesize);
            cur += query_responses_count_per_pos * params->field_bytesize;
        }
    }

    // mt membership proofs
    assert((cur + sizeof(size_t)) <= max_sig_len);
    memcpy(sig + cur, (uint8_t *)&merkle_trees_count, sizeof(size_t));
    cur += sizeof(size_t);
    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        size_t merkle_tree_membership_proof_size = merkle_tree_membership_proof_serialize_size(merkle_tree_membership_proofs[i], params);
        assert((cur + merkle_tree_membership_proof_size) <= max_sig_len);
        merkle_tree_membership_proof_serialize(merkle_tree_membership_proofs[i], params, sig + cur);
        cur += merkle_tree_membership_proof_size;
    }

    return cur;
}

size_t deserialize(uint64_t *prover_messages, uint8_t **mt_roots,
                   size_t *query_responses_count, uint64_t ***query_responses,
                   MerkleTreeMembershipProof **merkle_tree_membership_proofs,
                   const uint8_t *sig, const size_t sig_len, const size_t prover_messages_len, const size_t merkle_trees_count,
                   const Parameters *params)
{
    size_t cur = 0;
    // size_t prover_messages_len = params.fri_interactive_repetitions * params.num_ldt_instances * params.fri_final_polynomial_degree_bound * field_words;
    size_t pml = 0;
    assert(cur + sizeof(size_t) <= sig_len);
    memcpy(&pml, sig + cur, sizeof(size_t));
    cur += sizeof(size_t);
    assert(pml == prover_messages_len);

    assert(cur + prover_messages_len * params->field_bytesize <= sig_len);
    memcpy(prover_messages, sig + cur, prover_messages_len * params->field_bytesize);
    cur += prover_messages_len * params->field_bytesize;

    // size_t merkle_trees_count = params.fri_num_reductions + 1;
    size_t mkc = 0;
    assert(cur + sizeof(size_t) <= sig_len);
    memcpy(&mkc, sig + cur, sizeof(size_t));
    cur += sizeof(size_t);
    assert(mkc == merkle_trees_count);

    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        assert(cur + params->hash_bytesize <= sig_len);
        memcpy(mt_roots[i], sig + cur, params->hash_bytesize);
        cur += params->hash_bytesize;
    }

    size_t query_responses_len;
    assert(cur + sizeof(size_t) <= sig_len);
    memcpy(&query_responses_len, sig + cur, sizeof(size_t));
    cur += sizeof(size_t);
    assert(query_responses_len == merkle_trees_count);
    for (size_t i = 0; i < query_responses_len; i++)
    {
        assert(cur + sizeof(size_t) <= sig_len);
        memcpy(&query_responses_count[i], sig + cur, sizeof(size_t));
        cur += sizeof(size_t);
        query_responses[i] = (uint64_t **)malloc(query_responses_count[i] * sizeof(uint64_t **));
        for (size_t j = 0; j < query_responses_count[i]; j++)
        {
            size_t query_responses_count_per_pos;
            assert(cur + sizeof(size_t) <= sig_len);
            memcpy(&query_responses_count_per_pos, sig + cur, sizeof(size_t));
            cur += sizeof(size_t);
            query_responses[i][j] = (uint64_t *)malloc(query_responses_count_per_pos * params->field_bytesize);
            assert(cur + query_responses_count_per_pos * params->field_bytesize <= sig_len);
            memcpy(query_responses[i][j], sig + cur, query_responses_count_per_pos * params->field_bytesize);
            cur += query_responses_count_per_pos * params->field_bytesize;
        }
    }

    size_t merkle_tree_membership_proofs_count;
    assert(cur + sizeof(size_t) <= sig_len);
    memcpy(&merkle_tree_membership_proofs_count, sig + cur, sizeof(size_t));
    cur += sizeof(size_t);
    if (merkle_tree_membership_proofs_count != merkle_trees_count)
        return 0;
    for (size_t i = 0; i < merkle_tree_membership_proofs_count; i++)
    {
        merkle_tree_membership_proof_deserialize(sig + cur, params, merkle_tree_membership_proofs[i]);
        size_t merkle_tree_membership_proof_size = merkle_tree_membership_proof_serialize_size(merkle_tree_membership_proofs[i], params);
        assert(cur + merkle_tree_membership_proof_size <= sig_len);
        cur += merkle_tree_membership_proof_size;
    }

    if (cur != sig_len)
    {
        return 0; // Not using entire signature error
    }

    return cur;
}