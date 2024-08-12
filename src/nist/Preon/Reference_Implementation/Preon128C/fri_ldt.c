#include "fri_ldt.h"

#include <stdlib.h>
#include <assert.h>

#include "aurora_inner.h"
#include "fft.h"
#include "merkle_tree.h"
#include "oracles.h"
#include "params.h"
#include "poly.h"
#include "rand.h"
#include "query.h"
#include "util.h"
#include "virtual_oracles.h"

void submit_fri_ldt_masking_poly(const Parameters *params)
{
    // LDT_reducer_submit_masking_polynomial();
    uint64_t *ldt_blinding_vector = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    assert(params->num_ldt_instances == 1); // We have only 1 ldt instance for now
    for (size_t i = 0; i < params->num_ldt_instances; ++i)
    {
        uint64_t *random_p = (uint64_t *)malloc(params->max_ldt_tested_degree_bound * params->field_bytesize);
        random_bytes(random_p, params->max_ldt_tested_degree_bound * params->field_bytesize);
        fft(ldt_blinding_vector, random_p, params->max_ldt_tested_degree_bound, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);
        free(random_p);
    }
    ldt_blinding_vector_oracle.content = ldt_blinding_vector;
    ldt_blinding_vector_oracle.content_len = params->codeword_domain.size;
}

void fri_ldt(uint8_t *hash_state, MerkleTree **merkle_trees, uint8_t **mt_roots,
             size_t prover_messages_len, uint64_t *prover_messages,
             Oracle **f_i_oracles, uint64_t *const multi_f_i_evaluations_by_interaction, const Parameters *params)
{
    assert(params->fri_interactive_repetitions == 1);
    assert(params->num_ldt_instances == 1);
    uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);
    size_t total_localizations = 0;
    uint64_t *cur_f_i = multi_f_i_evaluations_by_interaction;
    size_t cur_f_i_len = 0;

    uint64_t *x_i = (uint64_t *)malloc(params->fri_num_reductions * params->field_bytesize);
    for (size_t i = 0; i < params->fri_num_reductions; ++i)
    {
        total_localizations += params->fri_localization_parameters[i];
        const size_t coset_size = 1ull << params->fri_localization_parameters[i];

        /** Send over the oracle produced for each interaction.
         *  When i is 0, it has already been submitted. */
        if (i > 0)
        {
            assert(cur_f_i_len);
            if (f_i_oracles[i]->content != multi_f_i_evaluations_by_interaction)
            {
                free(f_i_oracles[i]->content);
            }
            f_i_oracles[i]->content = cur_f_i;
            f_i_oracles[i]->content_len = cur_f_i_len;
            f_i_oracles[i]->content_deg = params->max_ldt_tested_degree_bound >> total_localizations;
            f_i_oracles[i]->domain = &params->fri_domains[i];
            f_i_oracles[i]->is_virtual = 0;
            cur_f_i_len = 0;

            // IOP_signal_prover_round_done();
            // This code runs when i >= 1, first merkle tree here has index 2 = i+1 in merkle_trees
            const uint64_t *merkle_tree_contents[1] = {f_i_oracles[i]->content};
            assert(merkle_tree_compute(merkle_trees[i + 1], merkle_tree_contents, 1) == 0);
            assert(merkle_tree_get_root(mt_roots[i + 1], merkle_trees[i + 1]) == 0);

            SHA3s(hash_state,
                  (const uint8_t *[]){(uint8_t *)&x_i[(i - 1) * params->field_words], mt_roots[i + 1]},
                  (size_t[]){params->field_bytesize, params->hash_bytesize}, 2, params->hash_bitsize);
        }

        /* For each interaction, receive the verifier challenge and create f_{i + 1} */
        squeeze_field_elements(&x_i[i * params->field_words], 1, i + 3, hash_state, params);

        // (this->poly_handles_.size() = params->num_ldt_instances) == 1 for now
        // evaluate_next_f_i_over_entire_domain
        const size_t num_cosets = params->fri_domains[i].size / coset_size;
        // TODO: leaks: next_f_i
        uint64_t *next_f_i = (uint64_t *)malloc(num_cosets * params->field_bytesize);
        assert(next_f_i);
        size_t next_f_i_len = num_cosets;

        /** Lagrange coefficient for coset element k is: vp_coset(x) / vp_coset[1] * (x - v[k])
         *
         *  The only change in vp_coset between cosets of the same domain is the constant term.
         *  As a consequence, we only need to calculate the vp_coset(x) and vp_coset[1] once.
         *  We then just adjust the vp_coset(x) by the constant term when processing each coset.
         *  TODO: Investigate if we can lower the number of inversions by taking advantage of
         *        v[k] = unshifted v[k % coset_size] + shift
         */

        // const uint64_t *coset_basis = f_i_domain_get_basis_subset_of_order(coset_size);
        // coset_basis actually getting first coset_size basis
        // const uint64_t *coset_basis = params->fri_domains[i].basis;
        Domain unshifted_coset = {
            .size = coset_size,
            .basis_len = params->fri_localization_parameters[i],
            .basis = params->fri_domains[i].basis,
            .shift = params->field_zero};
        const size_t unshifted_vp_len = vanishing_polynomial_len(&unshifted_coset);
        uint64_t *unshifted_vp = (uint64_t *)malloc(unshifted_vp_len * params->field_bytesize);
        vanishing_polynomial(unshifted_vp, unshifted_vp_len, &unshifted_coset, params);
        uint64_t *unshifted_vp_x = (uint64_t *)malloc(params->field_bytesize);
        poly_eval(unshifted_vp_x, unshifted_vp, unshifted_vp_len, &x_i[i * params->field_words], params->field_words);
        uint64_t *inv_vp_linear_term = (uint64_t *)malloc(params->field_bytesize);
        field_inv(inv_vp_linear_term, &unshifted_vp[1 * params->field_words]);

        /** We process cosets one at a time.
         *  We should batch process them for fewer inversions,
         *  but at too large of a batch size we will lose out on cache efficiency.    */
        /* x - V[k] vector, defined outside the loop to avoid re-allocations. */
        uint64_t shifted_coset_elements[coset_size * params->field_words];
        for (size_t j = 0; j < num_cosets; j++)
        {
            /** By definition of cosets,
             *  shifted vp = unshifted vp - unshifted_vp(shift) */
            uint64_t *coset_shift = (uint64_t *)malloc(params->field_bytesize);
            get_ith_field_element(coset_shift, &params->fri_domains[i], coset_size * j, params);
            // shifted_vp_x = unshifted_vp_x - evaluation_at_point(unshifted_vp, coset_shift)
            // = unshifted_vp_x + evaluation_at_point(unshifted_vp, coset_shift)
            uint64_t *shifted_vp_x = (uint64_t *)malloc(params->field_bytesize);
            poly_eval(shifted_vp_x, unshifted_vp, unshifted_vp_len, coset_shift, params->field_words);
            free(coset_shift);
            field_addEqual(shifted_vp_x, unshifted_vp_x, params->field_words);

            int x_in_domain = is_zero(shifted_vp_x, params->field_words);

            uint64_t *interpolation = (uint64_t *)malloc(params->field_bytesize);
            memset(interpolation, 0, params->field_bytesize);
            for (size_t k = 0; k < coset_size; k++)
            {
                /** If x in coset, set the interpolation accordingly. */

                get_ith_field_element(tmp, &params->fri_domains[i], j * coset_size + k, params);
                if ((x_in_domain) && field_equal(&x_i[i * params->field_words], tmp, params->field_words))
                {
                    memcpy(interpolation, &cur_f_i[(j * coset_size + k) * params->field_words], params->field_bytesize);
                    break;
                }
                /* If it's not in the coset, set this element to x - V[k] */
                memcpy(&shifted_coset_elements[k * params->field_words], &x_i[i * params->field_words], params->field_bytesize);
                field_subEqual(&shifted_coset_elements[k * params->field_words], tmp, params->field_words);
            }
            if (!x_in_domain)
            {
                uint64_t k[params->field_words];
                field_mul(k, inv_vp_linear_term, shifted_vp_x);
                uint64_t lagrange_coefficients[coset_size * params->field_words];
                field_batch_inverse_and_mul(lagrange_coefficients, shifted_coset_elements, coset_size, k, params->field_words);
                for (size_t k = 0; k < coset_size; k++)
                {
                    uint64_t tmp[params->field_words];
                    field_mul(tmp, &cur_f_i[(j * coset_size + k) * params->field_words], &lagrange_coefficients[k * params->field_words]);
                    field_addEqual(interpolation, tmp, params->field_words);
                }
            }
            memcpy(&next_f_i[j * params->field_words], interpolation, params->field_bytesize);
            free(interpolation);
            free(shifted_vp_x);
        }
        cur_f_i = next_f_i;
        cur_f_i_len = next_f_i_len;
        free(unshifted_vp);
        free(unshifted_vp_x);
        free(inv_vp_linear_term);
    }
    free(tmp);

    /* Finally, recover the coefficients of final polynomial using
       polynomial interpolation and directly send them. */
    uint64_t *temp = (uint64_t *)malloc(params->fri_domains[params->fri_num_reductions].size * params->field_bytesize);
    memset(temp, 0, params->fri_domains[params->fri_num_reductions].size * params->field_bytesize);
    // domain shift here should apply corresponding shift of fri_domains_size[params->fri_num_reductions]
    ifft(temp, cur_f_i, params->field_words, params->fri_domains[params->fri_num_reductions].size, params->fri_domains[params->fri_num_reductions].shift);
    memcpy(prover_messages, temp, prover_messages_len * params->field_bytesize);
    free(temp);
    if (cur_f_i != multi_f_i_evaluations_by_interaction)
    {
        free(cur_f_i);
    }

    // IOP_signal_prover_round_done();
    // Last round has no merkle tree, instead absorbing actual prover messages
    // No verifier squeezing in this round
    SHA3s(hash_state,
          (const uint8_t *[]){(uint8_t *)&x_i[(params->fri_num_reductions - 1) * params->field_words], (uint8_t *)prover_messages},
          (size_t[]){params->field_bytesize, prover_messages_len * params->field_bytesize}, 2, params->hash_bitsize);
    free(x_i);
}

int fri_verify_predicate(Oracle *combined_ldt_virtual_oracle,
                         QuerySet *query_sets, const size_t query_sets_len,
                         const uint64_t *prover_messages, const size_t prover_messages_len,
                         const uint64_t *verifier_messages,
                         size_t **query_pos, size_t **unique_query_pos,
                         uint64_t ***query_responses, const size_t *query_responses_count,
                         const Parameters *params)
{
    assert(combined_ldt_virtual_oracle->_id == combined_ldt_oracle._id);
    Oracle *sumcheck_g_virtual_oracle = combined_ldt_virtual_oracle->constituent_oracles[2];
    Oracle *combined_f_virtual_oracle = sumcheck_g_virtual_oracle->constituent_oracles[0];
    Oracle *multi_lincheck_virtual_oracle = combined_f_virtual_oracle->constituent_oracles[1];
    Oracle *fz_virtual_oracle = multi_lincheck_virtual_oracle->constituent_oracles[0];
    Oracle *rowcheck_virtual_oracle = combined_ldt_virtual_oracle->constituent_oracles[7];

    int decision = 1;
    uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);

    for (size_t q = 0; q < query_sets_len; q++)
    {
        // predicate for each query_set
        /* Verifier work */
        const size_t s0_idx = query_sets[q].s0_position;
        uint64_t *s0 = (uint64_t *)malloc(params->field_bytesize);
        get_ith_field_element(s0, &params->fri_domains[0], s0_idx, params);

        uint64_t *si = (uint64_t *)malloc(params->field_bytesize);
        memcpy(si, s0, params->field_bytesize);
        free(s0);
        size_t si_idx = s0_idx;
        uint64_t *last_interpolation = (uint64_t *)malloc(params->field_bytesize);

        int reduction_decision = 1;
        for (size_t i = 0; i < params->fri_num_reductions; ++i)
        {
            uint64_t x_i[params->field_words];
            memcpy(x_i, &verifier_messages[(22 + i) * params->field_words], params->field_bytesize);
            // get computed (hashed) verifier message instead of squeeze from hashchain

            size_t unique_query_pos_len = query_responses_count[i + 1];
            if (i == 0)
            {
                unique_query_pos_len = query_responses_count[0];
            }

            const size_t current_coset_size = 1ull << params->fri_localization_parameters[i];
            size_t si_j = si_idx / current_coset_size;
            size_t si_k = si_idx % current_coset_size;
            si_idx = si_j; /* next si position is the same as the coset index for si */

            /* Query the entire si coset */
            uint64_t *fi_on_si_coset = (uint64_t *)malloc(current_coset_size * params->field_bytesize);
            for (size_t k = 0; k < current_coset_size; ++k)
            {
                // Using the fact that params->fri_localization_parameters[i] == params->fri_localization_parameters[i - 1]
                size_t cur_query_pos = query_pos[i][q * current_coset_size + k];
                size_t query_responses_index = -1;
                for (size_t j = 0; j < unique_query_pos_len; j++)
                {
                    if (unique_query_pos[i][j] == cur_query_pos)
                    {
                        query_responses_index = j;
                        break;
                    }
                }

                assert(query_responses_index <= unique_query_pos_len);

                if (i != 0)
                {
                    // Querying non-virtual oracles
                    memcpy(&fi_on_si_coset[k * params->field_words], query_responses[i + 1][query_responses_index], params->field_bytesize);
                    continue;
                }
                // Getting query response of combined_ldt_virtual_oracle using query responses from script
                /*
                    Oracle *combined_ldt_virtual_oracle_constituent_oracles[9] = {&sumcheck_masking_poly_oracle,
                                                                                  &sumcheck_h_oracle,
                                                                                  sumcheck_g_virtual_oracle,
                                                                                  &fw_oracle,
                                                                                  &fAz_oracle,
                                                                                  &fBz_oracle,
                                                                                  &fCz_oracle,
                                                                                  rowcheck_virtual_oracle,
                                                                                  &ldt_blinding_vector_oracle};
                    query_responses[0][query_responses_index] = responses of {fw_oracle,
                                                                              fAz_oracle,
                                                                              fBz_oracle,
                                                                              fCz_oracle,
                                                                              sumcheck_masking_poly_oracle,
                                                                              ldt_blinding_vector_oracle}
                    query_responses[1][query_responses_index] = responses of {sumcheck_h_oracle}
                */
                // combined_ldt_virtual_oracle->constituent_oracles_len = 9
                uint64_t *combined_ldt_constituent_oracle_evaluations = (uint64_t *)malloc(combined_ldt_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[0 * params->field_words], &query_responses[0][query_responses_index][4 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[1 * params->field_words], &query_responses[1][query_responses_index][0 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[3 * params->field_words], &query_responses[0][query_responses_index][0 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[4 * params->field_words], &query_responses[0][query_responses_index][1 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[5 * params->field_words], &query_responses[0][query_responses_index][2 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[6 * params->field_words], &query_responses[0][query_responses_index][3 * params->field_words], params->field_bytesize);
                memcpy(&combined_ldt_constituent_oracle_evaluations[8 * params->field_words], &query_responses[0][query_responses_index][5 * params->field_words], params->field_bytesize);
                // Get query_responses of sumcheck_g_virtual_oracle
                uint64_t *sumcheck_g_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(sumcheck_g_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *sumcheck_g_constituent_oracle_evaluations = (uint64_t *)malloc(sumcheck_g_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&sumcheck_g_constituent_oracle_evaluations[1 * params->field_words],
                       &query_responses[1][query_responses_index][0 * params->field_words],
                       params->field_bytesize);
                // Get query_responses of combined_f_virtual_oracle, first element of sumcheck_g_constituent_oracle_evaluations
                uint64_t *combined_f_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(combined_f_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *combined_f_constituent_oracle_evaluations = (uint64_t *)malloc(combined_f_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&combined_f_constituent_oracle_evaluations[0 * params->field_words],
                       &query_responses[0][query_responses_index][4 * params->field_words],
                       params->field_bytesize);
                // Get query_responses of multi_lincheck_virtual_oracle, second element of combined_f_constituent_oracle_evaluations
                uint64_t *multi_lincheck_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(multi_lincheck_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *multi_lincheck_constituent_oracle_evaluations = (uint64_t *)malloc(multi_lincheck_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&multi_lincheck_constituent_oracle_evaluations[1 * params->field_words],
                       &query_responses[0][query_responses_index][1 * params->field_words],
                       params->field_bytesize);
                memcpy(&multi_lincheck_constituent_oracle_evaluations[2 * params->field_words],
                       &query_responses[0][query_responses_index][2 * params->field_words],
                       params->field_bytesize);
                memcpy(&multi_lincheck_constituent_oracle_evaluations[3 * params->field_words],
                       &query_responses[0][query_responses_index][3 * params->field_words],
                       params->field_bytesize);
                // Get query_responses of fz_virtual_oracle, first element of multi_lincheck_constituent_oracle_evaluations
                uint64_t *fz_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(fz_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *fz_constituent_oracle_evaluations = (uint64_t *)malloc(fz_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&fz_constituent_oracle_evaluations[0 * params->field_words],
                       &query_responses[0][query_responses_index][0 * params->field_words],
                       params->field_bytesize);
                assert(fz_virtual_oracle->evaluation_at_point(&multi_lincheck_constituent_oracle_evaluations[0 * params->field_words],
                                                              fz_virtual_oracle,
                                                              params,
                                                              0,
                                                              fz_evaluation_point,
                                                              fz_constituent_oracle_evaluations,
                                                              fz_virtual_oracle->constituent_oracles_len) == 0);
                free(fz_evaluation_point);
                free(fz_constituent_oracle_evaluations);

                assert(multi_lincheck_virtual_oracle->evaluation_at_point(&combined_f_constituent_oracle_evaluations[1 * params->field_words],
                                                                          multi_lincheck_virtual_oracle,
                                                                          params,
                                                                          0,
                                                                          multi_lincheck_evaluation_point,
                                                                          multi_lincheck_constituent_oracle_evaluations,
                                                                          multi_lincheck_virtual_oracle->constituent_oracles_len) == 0);
                free(multi_lincheck_evaluation_point);
                free(multi_lincheck_constituent_oracle_evaluations);
                assert(combined_f_virtual_oracle->evaluation_at_point(&sumcheck_g_constituent_oracle_evaluations[0 * params->field_words],
                                                                      combined_f_virtual_oracle,
                                                                      params,
                                                                      0,
                                                                      combined_f_evaluation_point,
                                                                      combined_f_constituent_oracle_evaluations,
                                                                      combined_f_virtual_oracle->constituent_oracles_len) == 0);
                free(combined_f_evaluation_point);
                free(combined_f_constituent_oracle_evaluations);
                assert(sumcheck_g_virtual_oracle->evaluation_at_point(&combined_ldt_constituent_oracle_evaluations[2 * params->field_words],
                                                                      sumcheck_g_virtual_oracle,
                                                                      params,
                                                                      0,
                                                                      sumcheck_g_evaluation_point,
                                                                      sumcheck_g_constituent_oracle_evaluations,
                                                                      sumcheck_g_virtual_oracle->constituent_oracles_len) == 0);
                free(sumcheck_g_evaluation_point);
                free(sumcheck_g_constituent_oracle_evaluations);
                // Get query_responses of rowcheck_virtual_oracle
                uint64_t *rowcheck_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(rowcheck_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *rowcheck_constituent_oracle_evaluations = (uint64_t *)malloc(rowcheck_virtual_oracle->constituent_oracles_len * params->field_bytesize);
                memcpy(&rowcheck_constituent_oracle_evaluations[0 * params->field_words],
                       &query_responses[0][query_responses_index][1 * params->field_words],
                       params->field_bytesize);
                memcpy(&rowcheck_constituent_oracle_evaluations[1 * params->field_words],
                       &query_responses[0][query_responses_index][2 * params->field_words],
                       params->field_bytesize);
                memcpy(&rowcheck_constituent_oracle_evaluations[2 * params->field_words],
                       &query_responses[0][query_responses_index][3 * params->field_words],
                       params->field_bytesize);
                assert(rowcheck_virtual_oracle->evaluation_at_point(&combined_ldt_constituent_oracle_evaluations[7 * params->field_words],
                                                                    rowcheck_virtual_oracle,
                                                                    params,
                                                                    0,
                                                                    rowcheck_evaluation_point,
                                                                    rowcheck_constituent_oracle_evaluations,
                                                                    rowcheck_virtual_oracle->constituent_oracles_len) == 0);
                free(rowcheck_evaluation_point);
                free(rowcheck_constituent_oracle_evaluations);

                uint64_t *combined_ldt_evaluation_point = (uint64_t *)malloc(params->field_bytesize);
                get_ith_field_element(combined_ldt_evaluation_point, &params->codeword_domain, cur_query_pos, params);
                uint64_t *combined_ldt_evaluation = new_oracle_evaulation_result(params);
                assert(combined_ldt_virtual_oracle->evaluation_at_point(combined_ldt_evaluation,
                                                                        combined_ldt_virtual_oracle,
                                                                        params,
                                                                        0,
                                                                        combined_ldt_evaluation_point,
                                                                        combined_ldt_constituent_oracle_evaluations,
                                                                        combined_ldt_virtual_oracle->constituent_oracles_len) == 0);

                free(combined_ldt_evaluation_point);
                free(combined_ldt_constituent_oracle_evaluations);
                // TODO: should the evaluation directly store into fi_on_si_coset?
                memcpy(&fi_on_si_coset[k * params->field_words], combined_ldt_evaluation, params->field_bytesize);
                free_oracle_evaulation_result(combined_ldt_evaluation);
            }

            /* Check that the coset matches interpolation computed in the
            previous round. */
            if (i > 0)
            {
                if (!field_equal(last_interpolation, &fi_on_si_coset[si_k * params->field_words], params->field_words))
                {
                    reduction_decision = 0;
                    break;
                }
            }

            /* Now compute interpolant of f_i|S_i evaluated at x_i
               (for use in next round). */
            const size_t shift_position = si_j * current_coset_size;
            uint64_t shift[params->field_words];
            get_ith_field_element(shift, &params->fri_domains[i], shift_position, params);
            uint64_t interpolation[params->field_words];
            // FieldT interpolation = evaluate_next_f_i_at_coset(
            //     fi_on_si_coset,
            //     this->localizer_domains_[i],
            //     shift,
            //     this->localizer_polynomials_[i],
            //     x_i);
            /** Other than vanishing polynomial calculation, this function is the same as
             *  the subspace lagrange coefficient generation, but with the interpolation being returned. */
            /* TODO: Cache unshifted_vp(x_i) and c */
            size_t localizer_poly_len = vanishing_polynomial_len(&params->fri_localizer_domains[i]);
            uint64_t *localizer_poly = (uint64_t *)malloc(localizer_poly_len * params->field_bytesize);
            vanishing_polynomial(localizer_poly, localizer_poly_len, &params->fri_localizer_domains[i], params);
            uint64_t localizer_poly_at_xi[params->field_words];
            uint64_t localizer_poly_at_shift[params->field_words];
            poly_eval(localizer_poly_at_xi, localizer_poly, localizer_poly_len, x_i, params->field_words);
            poly_eval(localizer_poly_at_shift, localizer_poly, localizer_poly_len, shift, params->field_words);

            uint64_t vp_x[params->field_words];
            field_sub(vp_x, localizer_poly_at_xi, localizer_poly_at_shift, params->field_words);
            uint64_t c[params->field_words];
            field_inv(c, &localizer_poly[1 * params->field_words]);
            /* In binary fields addition and subtraction are the same operation */
            uint64_t coset_shift[params->field_words];
            field_add(coset_shift, x_i, shift, params->field_words);
            uint64_t coset_elems[params->fri_localizer_domains[i].size * params->field_words];
            all_subset_sums(coset_elems, params->fri_localizer_domains[i].basis, params->fri_localizer_domains[i].basis_len, coset_shift, params->field_words);
            int interpolation_done = 0;
            if (is_zero(vp_x, params->field_words))
            {
                for (size_t k = 0; k < current_coset_size; k++)
                {
                    if (is_zero(&coset_elems[k * params->field_words], params->field_words))
                    {
                        memcpy(interpolation, &fi_on_si_coset[k * params->field_words], params->field_bytesize);
                        interpolation_done = 1;
                        break;
                    }
                }
            }
            if (!interpolation_done)
            {
                uint64_t lagrange_coefficients_mul[params->field_words];
                field_mul(lagrange_coefficients_mul, vp_x, c);
                uint64_t lagrange_coefficients[params->fri_localizer_domains[i].size * params->field_words];
                field_batch_inverse_and_mul(lagrange_coefficients, coset_elems, params->fri_localizer_domains[i].size, lagrange_coefficients_mul, params->field_words);
                memcpy(interpolation, params->field_zero, params->field_bytesize);
                for (size_t k = 0; k < params->fri_localizer_domains[i].size; k++)
                {
                    field_mul(tmp, &lagrange_coefficients[k * params->field_words], &fi_on_si_coset[k * params->field_words]);
                    field_addEqual(interpolation, tmp, params->field_words);
                }
            }

            memcpy(last_interpolation, interpolation, params->field_bytesize);
            poly_eval(tmp, localizer_poly, localizer_poly_len, si, params->field_words);
            memcpy(si, tmp, params->field_bytesize);
            free(localizer_poly);
            free(fi_on_si_coset);
        }

        decision = reduction_decision;

        /* Finally, check that the LAST round polynomial, evaluated at si,
           matches the final interpolation from the loop. */
        // const polynomial<FieldT> last_poly(
        // this->IOP_.receive_prover_message(this->final_polynomial_handles_[Q.interaction_index_][Q.LDT_index_]));
        // const FieldT last_poly_at_si = last_poly.evaluation_at_point(si); (last_poly = prover_messages)
        uint64_t *last_poly_at_si = (uint64_t *)malloc(params->field_bytesize);
        poly_eval(last_poly_at_si, prover_messages, prover_messages_len, si, params->field_words);

        if (!field_equal(last_poly_at_si, last_interpolation, params->field_words))
        {
            decision = 0;
            break;
        }
        free(last_interpolation);
        free(si);
        free(last_poly_at_si);
    }
    free(tmp);
    return decision;
}
