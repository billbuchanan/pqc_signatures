#include "query.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#include "aurora_inner.h"
#include "oracles.h"
#include "params.h"

size_t compute_first_query_pos(size_t initial_query, size_t i, size_t coset_size)
{
    const size_t coset_index = (size_t)(initial_query / coset_size);
    size_t query_pos = coset_index * coset_size + i;
    return query_pos;
}

void get_next_query_positions(size_t *result, size_t cur_query_pos_root, size_t cur_coset_size, size_t next_coset_size)
{
    // size_t *result = (size_t *)malloc(next_coset_size * sizeof(size_t));

    /** The previous query position gets localized to its coset index within the previous domain,
     *  for cosets of size (1ull << localization_parameter). Call this the localized position.
     *  Then we find the coset it is in, and query every position witin it. */
    for (size_t i = 0; i < next_coset_size; i++)
    {
        const size_t localized_position = (size_t)(cur_query_pos_root / cur_coset_size);
        const size_t localized_coset_index = (size_t)(localized_position / next_coset_size);
        result[i] = localized_coset_index * next_coset_size + i;
    }
}

size_t all_query_sets_len(const Parameters *params)
{
    return params->fri_query_repetitions * params->fri_interactive_repetitions * params->num_ldt_instances;
}

void get_all_query_sets(QuerySet *result, const uint8_t *hash_state, const Domain *fri_domains, const Parameters *params)
{
    // QuerySet *query_sets = (size_t *)malloc(params->fri_query_repetitions * params->fri_interactive_repetitions * params->num_ldt_instances * sizeof(QuerySet));
    // However params->fri_interactive_repetitions == params->num_ldt_instances == 1 in our cases
    size_t query_sets_count = 0;
    for (size_t i = 0; i < params->fri_query_repetitions; i++)
    {
        size_t initial_pos_upper_bound = fri_domains[0].size;
        for (size_t j = 0; j < params->fri_interactive_repetitions; j++)
        {
            for (size_t k = 0; k < params->num_ldt_instances; k++)
            {
                result[query_sets_count].interaction_index = j;
                result[query_sets_count].LDT_index = k;
                result[query_sets_count].s0_position = squeeze_query_pos(initial_pos_upper_bound, query_sets_count + 1, hash_state, params);
                query_sets_count++;
            }
        }
    }
}

void get_all_query_positions_from_all_query_sets(size_t **result, const QuerySet *query_sets, const Domain *fri_domains, const Parameters *params)
{
    memset(result, 0, params->fri_num_reductions * sizeof(size_t *));
    size_t result_count[params->fri_num_reductions];
    memset(result_count, 0, params->fri_num_reductions * sizeof(size_t));
    for (size_t i = 0; i < params->fri_query_repetitions; i++)
    {
        size_t initial_coset_size = 1ull << params->fri_localization_parameters[0];
        size_t initial_query_pos = query_sets[i].s0_position;

        size_t *s0_positions = (size_t *)malloc(initial_coset_size * sizeof(size_t));
        for (size_t j = 0; j < initial_coset_size; j++)
        {
            s0_positions[j] = compute_first_query_pos(initial_query_pos, j, initial_coset_size);
        }
        for (size_t j = 0; j < params->fri_interactive_repetitions; j++)
        {
            for (size_t k = 0; k < params->num_ldt_instances; k++)
            {
                result[0] = (size_t *)realloc(result[0], (result_count[0] + initial_coset_size) * sizeof(size_t));
                memcpy(&result[0][result_count[0]], s0_positions, initial_coset_size * sizeof(size_t));
                result_count[0] += initial_coset_size;
                for (size_t l = 1; l < params->fri_num_reductions; l++)
                {
                    size_t cur_coset_size = 1ull << params->fri_localization_parameters[l - 1];
                    size_t next_coset_size = 1ull << params->fri_localization_parameters[l];
                    result[l] = (size_t *)realloc(result[l], (result_count[l] + next_coset_size) * sizeof(size_t));
                    get_next_query_positions(&result[l][result_count[l]], result[l - 1][result_count[l]], cur_coset_size, next_coset_size);
                    result_count[l] += next_coset_size;
                }
            }
        }
        free(s0_positions);
    }
}

void get_all_query_positions(size_t **result, const uint8_t *hash_state, const Domain *fri_domains, const Parameters *params)
{
    memset(result, 0, params->fri_num_reductions * sizeof(size_t *));
    size_t result_count[params->fri_num_reductions];
    memset(result_count, 0, params->fri_num_reductions * sizeof(size_t));
    for (size_t i = 0; i < params->fri_query_repetitions; i++)
    {
        size_t initial_pos_upper_bound = fri_domains[0].size;
        size_t initial_coset_size = 1ull << params->fri_localization_parameters[0];
        size_t initial_query_pos = squeeze_query_pos(initial_pos_upper_bound, i + 1, hash_state, params);

        size_t *s0_positions = (size_t *)malloc(initial_coset_size * sizeof(size_t));
        for (size_t j = 0; j < initial_coset_size; j++)
        {
            s0_positions[j] = compute_first_query_pos(initial_query_pos, j, initial_coset_size);
        }
        for (size_t j = 0; j < params->fri_interactive_repetitions; j++)
        {
            for (size_t k = 0; k < params->num_ldt_instances; k++)
            {
                result[0] = (size_t *)realloc(result[0], (result_count[0] + initial_coset_size) * sizeof(size_t));
                memcpy(&result[0][result_count[0]], s0_positions, initial_coset_size * sizeof(size_t));
                result_count[0] += initial_coset_size;
                for (size_t l = 1; l < params->fri_num_reductions; l++)
                {
                    size_t cur_coset_size = 1ull << params->fri_localization_parameters[l - 1];
                    size_t next_coset_size = 1ull << params->fri_localization_parameters[l];
                    result[l] = (size_t *)realloc(result[l], (result_count[l] + next_coset_size) * sizeof(size_t));
                    get_next_query_positions(&result[l][result_count[l]], result[l - 1][result_count[l]], cur_coset_size, next_coset_size);
                    result_count[l] += next_coset_size;
                }
            }
        }
        free(s0_positions);
    }
}

// TODO: decide whether rewrite this or remove this
void get_query_response(uint64_t *result, const Oracle *query_oracle, size_t query_pos, const Parameters *params)
{
    if (!query_oracle->is_virtual)
    {
        memcpy(result, &query_oracle->content[query_pos * params->field_words], params->field_bytesize);
    }
    else
    {
        assert(0 && "not implemented for now");
        // uint64_t evaluation_point[params->field_words];
        // get_ith_field_element(evaluation_point, query_oracle->domain, query_pos, params);
        // size_t constituent_evaluations_len = query_oracle->constituent_oracles_len;
        // uint64_t constituent_evaluations[constituent_evaluations_len * params->field_words];
        // for (size_t i = 0; i < constituent_evaluations_len; i++)
        //     get_query_response(&constituent_evaluations[i * params->field_words], query_oracle->constituent_oracles[i], query_pos, params);

        // query_oracle->evaluation_at_point(result, query_oracle, params, query_pos,
        //                                   evaluation_point,
        //                                   constituent_evaluations, constituent_evaluations_len);
    }
}
