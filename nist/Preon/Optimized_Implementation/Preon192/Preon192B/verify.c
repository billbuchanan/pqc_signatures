#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "aurora_inner.h"
#include "field.h"
#include "fft.h"
#include "format.h"
#include "fri_ldt.h"
#include "merkle_tree.h"
#include "poly.h"
#include "params.h"
#include "query.h"
#include "r1cs.h"
#include "virtual_oracles.h"
#include "util.h"

int aurora_verifier(const uint8_t *sig, size_t sig_len, int sig_type, const uint8_t *msg, size_t msg_len,
                    const R1CS *r1cs, const uint64_t *input, size_t input_len)
{
    const Parameters *params = get_parameters(sig_type);
    const size_t field_bytesize = params->field_bytesize;
    const size_t field_words = params->field_words;
    const size_t hash_bytesize = params->hash_bytesize;

    size_t merkle_trees_count = params->fri_num_reductions + 1;
    size_t prover_messages_len = params->fri_final_polynomial_degree_bound;

    // Deserialization
    uint64_t *prover_messages = (uint64_t *)malloc(prover_messages_len * params->field_bytesize);
    uint8_t **mt_roots = (uint8_t **)malloc(merkle_trees_count * sizeof(uint8_t *));
    uint64_t ***query_responses = (uint64_t ***)malloc(merkle_trees_count * sizeof(uint64_t **));
    size_t *query_responses_count = (size_t *)malloc(merkle_trees_count * sizeof(size_t));
    MerkleTreeMembershipProof **merkle_tree_membership_proofs = (MerkleTreeMembershipProof **)malloc(merkle_trees_count * sizeof(MerkleTreeMembershipProof *));

    memset(prover_messages, 0, prover_messages_len * params->field_bytesize);
    memset(query_responses, 0, merkle_trees_count * sizeof(uint64_t **));
    memset(query_responses_count, 0, merkle_trees_count * sizeof(size_t));
    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        mt_roots[i] = (uint8_t *)malloc(hash_bytesize);
        merkle_tree_membership_proofs[i] = merkle_tree_membership_proof_init();
    }

    assert(deserialize(prover_messages, mt_roots,
                       query_responses_count, query_responses,
                       merkle_tree_membership_proofs,
                       sig, sig_len, prover_messages_len, merkle_trees_count,
                       params) == sig_len);
    // transcript_is_valid(sig, sig_len);
    // IOP.seal_interaction_registrations();
    // 1. For each round, check auth path and MT_roots
    // 2. Check proof of work

    size_t total_verifier_messages_count = 0;
    for (size_t i = 0; i < params->num_interaction_rounds; i++)
    {
        total_verifier_messages_count += params->verifier_messages_count[i];
    }

    uint64_t *verifier_messages = (uint64_t *)malloc(total_verifier_messages_count * field_bytesize);
    size_t cur_verifier_messages_count = 0;

    uint8_t *hash_state = (uint8_t *)malloc(hash_bytesize);
    SHA3(hash_state, msg, msg_len, params->hash_bitsize);

    for (size_t round = 0; round < params->num_interaction_rounds; ++round)
    {
        /* Update the pseudorandom state for the oracle messages. */

        /* Absorb MT roots into hashchain. */
        // Each domain has one Merkle tree containing all the oracles.
        // Last round has no merkle tree
        if (round == 0)
        {
            SHA3s(hash_state, (const uint8_t *[]){hash_state, mt_roots[round]}, (size_t[]){hash_bytesize, hash_bytesize}, 2, params->hash_bitsize);
            squeeze_field_elements(&verifier_messages[cur_verifier_messages_count * field_words], params->verifier_messages_count[0], 1, hash_state, params);
            cur_verifier_messages_count += params->verifier_messages_count[round];
        }
        else if (round == 1)
        {
            SHA3s(hash_state,
                  (const uint8_t *[]){(uint8_t *)&verifier_messages[0 * field_words], mt_roots[round]},
                  (size_t[]){params->verifier_messages_count[0] * params->field_bytesize, hash_bytesize}, 2, params->hash_bitsize);
            squeeze_field_elements(&verifier_messages[cur_verifier_messages_count * field_words], params->verifier_messages_count[1] - 1, 2, hash_state, params);
            cur_verifier_messages_count += (params->verifier_messages_count[round] - 1);
            squeeze_field_elements(&verifier_messages[cur_verifier_messages_count * field_words], 1, 3, hash_state, params);
            cur_verifier_messages_count++;
        }
        else
        {
            SHA3s(hash_state,
                  (const uint8_t *[]){(uint8_t *)&verifier_messages[(cur_verifier_messages_count - 1) * field_words], mt_roots[round]},
                  (size_t[]){params->field_bytesize, hash_bytesize}, 2, params->hash_bitsize);
            squeeze_field_elements(&verifier_messages[cur_verifier_messages_count * field_words], params->verifier_messages_count[round], round + 2, hash_state, params);
            cur_verifier_messages_count += params->verifier_messages_count[round];
        }
    }
    SHA3s(hash_state,
          (const uint8_t *[]){(uint8_t *)&verifier_messages[(cur_verifier_messages_count - 1) * field_words], (uint8_t *)prover_messages},
          (size_t[]){params->field_bytesize, prover_messages_len * params->field_bytesize},
          2, params->hash_bitsize);

    size_t query_sets_len = all_query_sets_len(params);
    QuerySet *query_sets = (QuerySet *)malloc(query_sets_len * sizeof(QuerySet));
    get_all_query_sets(query_sets, hash_state, params->fri_domains, params);

    size_t **query_pos = (size_t **)malloc(params->fri_num_reductions * sizeof(size_t *));
    memset(query_pos, 0, params->fri_num_reductions * sizeof(size_t *));
    get_all_query_positions_from_all_query_sets(query_pos, query_sets, params->fri_domains, params);
    size_t **unique_query_pos = (size_t **)malloc(params->fri_num_reductions * sizeof(size_t *));

    /* Validate all MT queries relative to the transcript. */
    int proof_is_valid = verify_all_merkle_tree_membership_proofs(unique_query_pos,
                                                                  merkle_tree_membership_proofs, mt_roots, merkle_trees_count,
                                                                  query_responses,
                                                                  query_responses_count,
                                                                  query_pos,
                                                                  params);
    if (!proof_is_valid)
    {
        return 0;
    }

    /* Check proof of work */
    // hash_digest_type pow_challenge = this->hashchain_->squeeze_root_type();
    // int valid_pow = this_pow_verify_pow(this->parameters_.compression_hasher, pow_challenge, this->transcript_.proof_of_work_);
    // if (valid_pow != 1)
    //     return valid_pow;

    /* Finally populate things for obtaining query responses */
    // parse_query_responses_from_transcript(); // parsed a {(oracle_id, position): field_value} dictionary

    // LDT_reducer_verifier_predicate();
    init_oracles(params);
    Oracle **virtual_oracles = init_virtual_oracles(r1cs, params);
    for (size_t i = 0; i < 6; i++)
    {
        if (!virtual_oracles[i])
            return 0;
    }
    Oracle *fz_virtual_oracle = virtual_oracles[0];
    // Oracle *rowcheck_virtual_oracle = virtual_oracles[1]; // unused in verifier
    Oracle *multi_lincheck_virtual_oracle = virtual_oracles[2];
    Oracle *combined_f_virtual_oracle = virtual_oracles[3];
    Oracle *sumcheck_g_virtual_oracle = virtual_oracles[4];
    Oracle *combined_ldt_virtual_oracle = virtual_oracles[5];

    if (fz_oracle_set_primary_input(fz_virtual_oracle, input, input_len) != 0)
        return 0;

    uint64_t alpha[field_words];
    memcpy(alpha, &verifier_messages[0 * field_words], field_bytesize);
    size_t r_Mz_count = 3;
    uint64_t *r_Mz = (uint64_t *)malloc(r_Mz_count * field_bytesize);
    memcpy(r_Mz, &verifier_messages[1 * field_words], r_Mz_count * field_bytesize);
    if (multi_lincheck_oracle_set_challenge(multi_lincheck_virtual_oracle, alpha, r_Mz, r_Mz_count) != 0)
        return 0;

    uint64_t *combined_f_random_coefficients = (uint64_t *)malloc(2 * field_bytesize);
    memcpy(&combined_f_random_coefficients[0 * params->field_words], params->field_one, params->field_bytesize);
    memcpy(&combined_f_random_coefficients[1 * params->field_words], params->field_one, params->field_bytesize);
    if (combined_f_oracle_set_random_coefficients(combined_f_virtual_oracle, combined_f_random_coefficients, 2) != 0)
        return 0;

    if (sumcheck_g_oracle_set_claimed_sum(sumcheck_g_virtual_oracle, params->field_zero) != 0)
        return 0;

    assert(params->num_ldt_instances == 1);
    // params->verifier_messages_count[1] contains both random coeff for combined_ldt_oracle and challenge for first fri_ldt
    size_t ldt_challenges_count = params->verifier_messages_count[1] - 1;
    uint64_t *ldt_challenges = (uint64_t *)malloc(ldt_challenges_count * field_bytesize);
    memcpy(ldt_challenges, &verifier_messages[4 * field_words], ldt_challenges_count * field_bytesize);
    if (combined_ldt_oracle_set_random_coefficients(combined_ldt_virtual_oracle,
                                                    ldt_challenges,
                                                    2 * combined_ldt_oracle.constituent_oracles_len) != 0)
        return 0;
    free(ldt_challenges); // ldt_challenges memcpyed into combined_ldt_virtual_oracle->data->_coefficients

    int decision = fri_verify_predicate(combined_ldt_virtual_oracle,
                                        query_sets,
                                        query_sets_len,
                                        prover_messages, prover_messages_len,
                                        verifier_messages,
                                        query_pos, unique_query_pos,
                                        query_responses, query_responses_count, params);

    free(prover_messages);
    free(r_Mz);
    free(query_sets);

    free(combined_f_random_coefficients);

    free(verifier_messages);
    for (size_t i = 0; i < params->fri_num_reductions; i++)
    {
        free(query_pos[i]);
        free(unique_query_pos[i]);
    }
    free(query_pos);
    free(unique_query_pos);

    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        for (size_t j = 0; j < query_responses_count[i]; j++)
        {
            free(query_responses[i][j]);
        }
        free(query_responses[i]);
        merkle_tree_membership_proof_free(merkle_tree_membership_proofs[i]);
        free(mt_roots[i]);
    }
    free(query_responses_count);
    free(merkle_tree_membership_proofs);
    free(query_responses);
    free(mt_roots);

    deinit_virtual_oracles(virtual_oracles);
    deinit_oracles();

    return proof_is_valid && decision;
}