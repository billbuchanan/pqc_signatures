#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aurora.h"
#include "aurora_inner.h"
#include "fft.h"
#include "field.h"
#include "format.h"
#include "fri_ldt.h"
#include "lincheck.h"
#include "merkle_tree.h"
#include "oracles.h"
#include "poly.h"
#include "params.h"
#include "query.h"
#include "r1cs.h"
#include "r1cs_rs_iop.h"
#include "rand.h"
#include "sumcheck.h"
#include "virtual_oracles.h"
#include "util.h"

size_t aurora_prover(uint8_t *sig, size_t sig_len, int sig_type, const uint8_t *msg, size_t msg_len,
                     const R1CS *r1cs, const uint64_t *input, size_t primary_input_count, size_t auxiliary_input_count)
{
    const Parameters *params = get_parameters(sig_type);
    if (primary_input_count != (params->input_variable_domain.size - 1) ||
        (primary_input_count + auxiliary_input_count) != (params->variable_domain.size - 1))
        return 0;

    size_t merkle_trees_count = params->fri_num_reductions + 1;
    MerkleTree **merkle_trees = (MerkleTree **)malloc(merkle_trees_count * sizeof(MerkleTree *));

    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        size_t coset_size = 1;
        size_t leaf_content_len = 0;
        if (i < 2)
        {
            coset_size = 1ull << params->fri_localization_parameters[0];
            leaf_content_len = params->codeword_domain.size;
        }
        else
        {
            coset_size = 1ull << params->fri_localization_parameters[i - 1];
            leaf_content_len = params->fri_domains[i - 1].size;
        }
        merkle_trees[i] = merkle_tree_init(leaf_content_len, coset_size, 1, params);
    }
    uint8_t **mt_roots = (uint8_t **)malloc(merkle_trees_count * sizeof(uint8_t *));
    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        mt_roots[i] = (uint8_t *)malloc(params->hash_bytesize);
    }

    uint8_t *hash_state = (uint8_t *)malloc(params->hash_bytesize);
    SHA3(hash_state, msg, msg_len, params->hash_bitsize);

    init_oracles(params);

    // submit_witness_oracles
    uint64_t *f1v_coeff = (uint64_t *)malloc(params->input_variable_domain.size * params->field_bytesize);
    r1cs_rs_iop(f1v_coeff, r1cs, input, primary_input_count, auxiliary_input_count, params);
    submit_sumcheck_masking_poly(params);
    submit_fri_ldt_masking_poly(params);

    // IOP_signal_prover_round_done();
    const uint64_t *merkle_tree_0_contents[6] = {fw_oracle.content,
                                                 fAz_oracle.content,
                                                 fBz_oracle.content,
                                                 fCz_oracle.content,
                                                 sumcheck_masking_poly_oracle.content,
                                                 ldt_blinding_vector_oracle.content};

    assert(merkle_tree_compute(merkle_trees[0], merkle_tree_0_contents, 6) == 0);
    assert(merkle_tree_get_root(mt_roots[0], merkle_trees[0]) == 0);
    SHA3s(hash_state, (const uint8_t *[]){hash_state, mt_roots[0]}, (size_t[]){params->hash_bytesize, params->hash_bytesize}, 2, params->hash_bitsize);

    Oracle **virtual_oracles = init_virtual_oracles(r1cs, params);
    for (size_t i = 0; i < 6; i++)
    {
        if (!virtual_oracles[i])
            return 0;
    }
    Oracle *fz_virtual_oracle = virtual_oracles[0];
    Oracle *rowcheck_virtual_oracle = virtual_oracles[1];
    Oracle *multi_lincheck_virtual_oracle = virtual_oracles[2];
    Oracle *combined_f_virtual_oracle = virtual_oracles[3];
    Oracle *sumcheck_g_virtual_oracle = virtual_oracles[4];
    Oracle *combined_ldt_virtual_oracle = virtual_oracles[5];

    if (fz_oracle_set_primary_input(fz_virtual_oracle, input, primary_input_count) != 0)
        return 0;
    fz_oracle_evaluate_content(fz_virtual_oracle, f1v_coeff, params->input_variable_domain.size, params);
    free(f1v_coeff);
    multi_lincheck_calculate_and_submit_proof(multi_lincheck_virtual_oracle, hash_state, primary_input_count, auxiliary_input_count, params);
    sumcheck_calculate_and_submit_proof(combined_f_virtual_oracle, params);

    // IOP_signal_prover_round_done();
    const uint64_t *merkle_tree_1_contents[1] = {sumcheck_h_oracle.content};
    assert(merkle_tree_compute(merkle_trees[1], merkle_tree_1_contents, 1) == 0);
    assert(merkle_tree_get_root(mt_roots[1], merkle_trees[1]) == 0);

    MultilinCheckOracleData *multi_lincheck_data = (MultilinCheckOracleData *)multi_lincheck_virtual_oracle->data;
    SHA3s(hash_state,
          (const uint8_t *[]){(uint8_t *)multi_lincheck_data->_alpha, (uint8_t *)multi_lincheck_data->_r_Mz, mt_roots[1]},
          (size_t[]){params->field_bytesize, params->field_bytesize * multi_lincheck_data->_r_Mz_len, params->hash_bytesize}, 3, params->hash_bitsize);

    /* LDT will send a challenge and signal done internally */
    // LDT_reducer_calculate_and_submit_proof();

    // multi_LDT_calculate_and_submit_proof();
    // Compute content of sumcheck_g_oracle before usage
    sumcheck_g_oracle_set_claimed_sum(sumcheck_g_virtual_oracle, params->field_zero);
    sumcheck_g_oracle_evaluate_content(sumcheck_g_virtual_oracle, params);

    // Compute content of rowcheck_oracle before usage
    rowcheck_oracle_evaluate_content(rowcheck_virtual_oracle, params);

    // Compute multi_f_i_evaluations
    /* First set of codewords: the original purported codewords we're testing. */
    assert(params->num_ldt_instances == 1);
    // size_t num_input_oracles = 8;
    // size_t ldt_challenges_count = 2 * num_input_oracles + 2 = 18;
    uint64_t *ldt_challenges = (uint64_t *)malloc(18 * params->field_bytesize);
    squeeze_field_elements(ldt_challenges, 18, 2, hash_state, params);
    if (combined_ldt_oracle_set_random_coefficients(combined_ldt_virtual_oracle, ldt_challenges, 2 * combined_ldt_oracle.constituent_oracles_len) != 0)
        return 0;
    free(ldt_challenges); // ldt_challenges memcpyed into combined_ldt_virtual_oracle->data->_coefficients
    combined_ldt_oracle_evaluate_content(combined_ldt_virtual_oracle, params);

    /* indexed by interaction, then LDT instance index */
    // this may not be required if params->fri_interactive_repetitions == params->num_ldt_instances == 1 in all conditions
    assert(params->fri_interactive_repetitions == 1);
    assert(params->num_ldt_instances == 1);
    uint64_t *multi_f_i_evaluations_by_interaction = combined_ldt_virtual_oracle->content;
    Oracle **f_i_oracles = (Oracle **)malloc(params->fri_num_reductions * sizeof(Oracle *));
    for (size_t i = 0; i < params->fri_num_reductions; ++i)
    {
        f_i_oracles[i] = (Oracle *)malloc(sizeof(Oracle));
        memset(f_i_oracles[i], 0, sizeof(Oracle));
    }
    f_i_oracles[0]->content = combined_ldt_virtual_oracle->content;
    f_i_oracles[0]->content_len = combined_ldt_virtual_oracle->content_len;

    size_t prover_messages_len = params->fri_final_polynomial_degree_bound;
    uint64_t *prover_messages = (uint64_t *)malloc(prover_messages_len * params->field_bytesize);
    fri_ldt(hash_state, merkle_trees, mt_roots,
            prover_messages_len, prover_messages,
            f_i_oracles, multi_f_i_evaluations_by_interaction, params);

    // IOP_get_transcript(sig);
    // Only deal with query_pos, query_responses, and getting all MTs authentication paths (as membership proofs)
    // each query_pos[i] has 1ull << params->fri_localization_parameters[i]
    // This, for now, not considering fri_interactive_repetitions and num_ldt_instances
    // Though both of them are 1 in our case
    size_t **query_pos = (size_t **)malloc(params->fri_num_reductions * sizeof(size_t *));
    memset(query_pos, 0, params->fri_num_reductions * sizeof(size_t *));
    get_all_query_positions(query_pos, hash_state, params->fri_domains, params);
    uint64_t ***query_responses = (uint64_t ***)malloc(merkle_trees_count * sizeof(uint64_t **));
    size_t *query_responses_count = (size_t *)malloc(merkle_trees_count * sizeof(size_t));
    /*
      Go over all MTs and prepare multi membership proofs. (Now that we
      know both what the oracle messages are and where they'll be queried,
      we can find authentication paths.)
    */
    MerkleTreeMembershipProof **merkle_tree_membership_proofs = (MerkleTreeMembershipProof **)malloc(merkle_trees_count * sizeof(MerkleTreeMembershipProof *));
    get_all_merkle_tree_membership_proofs(merkle_tree_membership_proofs, merkle_trees_count,
                                          query_responses,
                                          query_responses_count,
                                          query_pos,
                                          f_i_oracles,
                                          merkle_trees,
                                          params);

    // Serialization, not having transcript probably directly output to sig
    size_t result_sig_len = serialize(sig, sig_len,
                                      prover_messages, prover_messages_len,
                                      mt_roots, merkle_trees_count,
                                      query_responses_count,
                                      query_responses,
                                      merkle_tree_membership_proofs,
                                      params);
    // Free malloc memory
    free(prover_messages);

    // oracles contents
    free(fw_oracle.content);
    free(fAz_oracle.content);
    free(fBz_oracle.content);
    free(fCz_oracle.content);
    free(sumcheck_masking_poly_oracle.content);
    free(ldt_blinding_vector_oracle.content);
    free(sumcheck_h_oracle.content);
    for (size_t i = 0; i < params->fri_num_reductions; i++)
    {
        if (f_i_oracles[i]->content != combined_ldt_virtual_oracle->content)
        {
            free(f_i_oracles[i]->content);
        }
        free(f_i_oracles[i]);
    }
    free(f_i_oracles);

    // others
    for (size_t i = 0; i < params->fri_num_reductions; i++)
    {
        free(query_pos[i]);
    }

    for (size_t i = 0; i < merkle_trees_count; i++)
    {
        for (size_t j = 0; j < query_responses_count[i]; j++)
        {
            free(query_responses[i][j]);
        }
        free(query_responses[i]);
        merkle_tree_free(merkle_trees[i]);
        merkle_tree_membership_proof_free(merkle_tree_membership_proofs[i]);
        free(mt_roots[i]);
    }
    free(query_responses_count);
    free(merkle_tree_membership_proofs);
    free(query_responses);
    free(merkle_trees);
    free(mt_roots);
    free(query_pos);

    deinit_virtual_oracles(virtual_oracles);
    deinit_oracles();

    return result_sig_len;
}