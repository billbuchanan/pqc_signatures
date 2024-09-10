#include "aurora_inner.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>

#include "field.h"
#include "merkle_tree.h"
#include "oracles.h"
#include "params.h"
#include "poly.h"
#include "util.h"

const Parameters *get_parameters(int sig_type)
{
    return &preon;
}

size_t aurora_proof_max_size(int sig_type)
{
    const Parameters *params = get_parameters(sig_type);
    size_t proof_max_size = 0;
    proof_max_size += sizeof(size_t);
    size_t prover_messages_len = params->fri_final_polynomial_degree_bound + 1;
    proof_max_size += prover_messages_len * params->field_bytesize;
    proof_max_size += sizeof(size_t);
    size_t merkle_trees_count = params->fri_num_reductions + 1;
    size_t coset_size = 1ull << params->fri_localization_parameters[0];
    proof_max_size += merkle_trees_count * params->hash_bytesize;
    proof_max_size += sizeof(size_t);
    proof_max_size += merkle_trees_count * (sizeof(size_t) + params->fri_query_repetitions * coset_size * (sizeof(size_t) + 6 * params->field_bytesize));
    size_t max_mt_leave_count = (size_t)(params->codeword_domain.size / coset_size);
    proof_max_size += sizeof(size_t);
    proof_max_size += merkle_trees_count * (1 /* version */ +
                                            8 /* total length */ +
                                            8 /* proof_length */ +
                                            8 /* randomness_hashes_length*/ +
                                            8 /* coset_serialization_size */ +
                                            8 /* num_leaves */ +
                                            (size_t)log2(max_mt_leave_count) * params->hash_bytesize /* proofs */ +
                                            params->fri_query_repetitions * coset_size * params->hash_zk_bytesize /* zk randomness */);
    return proof_max_size;
}

void signal_prover_round_done(uint8_t *hash_state, MerkleTree *mt, uint8_t *mt_root,
                              const uint64_t **mt_contents, size_t mt_contents_len,
                              uint64_t *prover_message, size_t prover_message_len,
                              const Parameters *params)
{
    if (mt && mt_root && mt_contents && mt_contents_len > 0)
    {
        assert(merkle_tree_compute(mt, mt_contents, mt_contents_len) == 0);
        assert(merkle_tree_get_root(mt_root, mt) == 0);
        SHA3s(hash_state, (const uint8_t *[]){hash_state, mt_root}, (size_t[]){params->hash_bytesize, params->hash_bytesize}, 2, params->hash_bitsize);
    }

    if (prover_message && prover_message_len > 0)
        SHA3s(hash_state,
              (const uint8_t *[]){hash_state, (uint8_t *)prover_message},
              (size_t[]){params->hash_bytesize, prover_message_len * params->field_bytesize},
              2, params->hash_bitsize);
}

void squeeze_field_elements(uint64_t *out, size_t elements_count, uint8_t round, const uint8_t *hash_state, const Parameters *params)
{
    for (uint8_t i = 1; i <= elements_count; i++)
    {
        uint8_t *output = (uint8_t *)malloc(params->hash_bytesize * sizeof(uint8_t));
        SHA3s(output,
              (const uint8_t *[]){hash_state, &round, &i},
              (size_t[]){params->hash_bytesize, sizeof(uint8_t), sizeof(uint8_t)}, 3, params->hash_bitsize);
        memcpy(&out[(i - 1) * params->field_words], output, params->field_bytesize);
        free(output);
    }
}

size_t squeeze_query_pos(size_t upper_bound, uint8_t pos_count, const uint8_t *hash_state, const Parameters *params)
{
    uint8_t round = params->fri_num_reductions + 3;
    uint8_t *output = (uint8_t *)malloc(params->hash_bytesize * sizeof(uint8_t));
    SHA3s(output,
          (const uint8_t *[]){hash_state, &round, &pos_count},
          (size_t[]){params->hash_bytesize, sizeof(uint8_t), sizeof(uint8_t)}, 3, params->hash_bitsize);
    size_t result;
    memcpy(&result, output, sizeof(result));
    free(output);
    return result % upper_bound;
}

void all_subset_sums(uint64_t *result, const uint64_t *basis_powers, size_t basis_power_len, const uint64_t *shift_power, const size_t field_words)
{
    const size_t result_size = 1ull << basis_power_len;
    memset(result, 0, result_size * field_words * sizeof(uint64_t));
    memcpy(&result[0], shift_power, field_words * sizeof(uint64_t));
    for (size_t i = 0; i < basis_power_len; ++i)
    {
        const size_t l = (1ull << i);
        for (size_t j = 0; j < l; ++j)
            field_add(&result[(l + j) * field_words], &result[j * field_words], &basis_powers[i * field_words], field_words);
    }
}

void subspace_to_power_of_two(uint64_t *result,
                              const uint64_t *subspace_basis, const size_t subspace_basis_len,
                              const uint64_t *subspace_shift,
                              const size_t power_of_two, const size_t field_words)
{

    // bump_factors_exponent is power of 2
    uint64_t *basis_powers = (uint64_t *)malloc(subspace_basis_len * field_words * sizeof(uint64_t));
    for (size_t i = 0; i < subspace_basis_len; i++)
    {
        field_pow(&basis_powers[i * field_words], &subspace_basis[i * field_words], power_of_two, field_words);
    }

    uint64_t *shift_power = (uint64_t *)malloc(field_words * sizeof(uint64_t));
    field_pow(shift_power, subspace_shift, power_of_two, field_words);
    all_subset_sums(result, basis_powers, subspace_basis_len, shift_power, field_words);
    free(shift_power);
    free(basis_powers);
}

size_t vanishing_polynomial_len(const Domain *domain)
{
    return domain->size + 1;
}

void vanishing_polynomial(uint64_t *result, const size_t result_len, const Domain *domain, const Parameters *params)
{
    const size_t field_words = params->field_words;
    size_t cur_result_size = 2;

    memset(result, 0, result_len * params->field_bytesize);
    memcpy(&result[1 * field_words], params->field_one, params->field_bytesize);

    for (size_t i = 0; i < domain->basis_len; i++)
    {
        uint64_t *poly_c = (uint64_t *)malloc(params->field_bytesize);
        poly_eval(poly_c, result, cur_result_size, &domain->basis[i * field_words], field_words);

        size_t next_result_size = 2 * cur_result_size - 1;
        uint64_t *squared = (uint64_t *)malloc(next_result_size * params->field_bytesize);
        memset(squared, 0, next_result_size * params->field_bytesize);
        poly_mul(squared, result, cur_result_size, result, cur_result_size, field_words);
        memcpy(&squared[0 * field_words], params->field_zero, params->field_bytesize);

        poly_scalarMulEqual(result, cur_result_size, poly_c, field_words);
        poly_addEqual(result, next_result_size, squared, next_result_size, field_words);
        cur_result_size = next_result_size;
        free(squared);
        free(poly_c);
    }
    uint64_t *shift_eval = (uint64_t *)malloc(params->field_bytesize);
    poly_eval(shift_eval, result, result_len, domain->shift, field_words);
    field_addEqual(&result[0 * field_words], shift_eval, field_words);
    free(shift_eval);
}

void get_ith_field_element(uint64_t *result, const Domain *domain, const size_t i, const Parameters *params)
{
    memcpy(result, domain->shift, params->field_bytesize);
    for (size_t j = 0; j < domain->basis_len; j++)
    {
        if (i & (1ull << j))
            field_addEqual(result, &domain->basis[j * params->field_words], params->field_words);
    }
}

// This is actually not a cache, just computing required lagrange coefficients
void lagrange_cache(uint64_t *result, const uint64_t *interpolation_point, const Domain *domain, const Parameters *params)
{
    size_t field_words = params->field_words;
    uint64_t c[field_words];
    size_t vp_len = domain->size + 1;
    uint64_t *vp = (uint64_t *)malloc(vp_len * params->field_bytesize);
    vanishing_polynomial(vp, vp_len, domain, params);
    field_inv(c, &vp[1 * field_words]);

    uint64_t k[field_words];
    poly_eval(k, vp, vp_len, interpolation_point, field_words);
    field_mulEqual(k, c, field_words);

    uint64_t V_shift[field_words];
    memcpy(V_shift, interpolation_point, params->field_bytesize);
    field_addEqual(V_shift, domain->shift, field_words);
    size_t result_len = domain->size;
    uint64_t *V = (uint64_t *)malloc(result_len * params->field_bytesize);
    all_subset_sums(V, domain->basis, domain->basis_len, V_shift, field_words);
    // Handle check if interpolation point is in domain
    // In our case, is_interpolation_domain_intersects_domain always == false
    // if (is_interpolation_domain_intersects_domain && is_zero(k, field_words))
    // {
    //     memset(result, 0, result_len * params->field_bytesize);
    //     for (size_t i = 0; i < result_len; i++)
    //     {
    //         if (is_zero(V[i * field_words], field_words))
    //         {
    //             memcpy(&result[i * field_words], params->field_one, params->field_bytesize);
    //             break;
    //         }
    //     }
    // }
    field_batch_inverse_and_mul(result, V, result_len, k, field_words);
    free(V);
    free(vp);
}

static inline __attribute__((always_inline)) void swap_values(size_t i, size_t j, uint64_t **values, size_t value_len, size_t value_words, uint64_t *tmp_buf)
{
    memcpy(tmp_buf, values[i], value_len * value_words * 8);
    memcpy(values[i], values[j], value_len * value_words * 8);
    memcpy(values[j], tmp_buf, value_len * value_words * 8);
}

static int qs_partition(size_t *arr, int low, int high, uint64_t **values, size_t value_len, size_t value_words, uint64_t *tmp_buf)
{
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;
            size_t tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            if (values != NULL)
            {
                swap_values(i, j, values, value_len, value_words, tmp_buf);
            }
        }
    }

    size_t tmp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = tmp;
    if (values != NULL)
    {
        swap_values(i + 1, high, values, value_len, value_words, tmp_buf);
    }
    return (i + 1);
}

static void qs(size_t *arr, int low, int high, uint64_t **values, size_t values_len, size_t value_words, uint64_t *tmp_buf)
{
    if (low < high)
    {
        int pi = qs_partition(arr, low, high, values, values_len, value_words, tmp_buf);
        qs(arr, low, pi - 1, values, values_len, value_words, tmp_buf);
        qs(arr, pi + 1, high, values, values_len, value_words, tmp_buf);
    }
}

size_t sort_and_unique_values(size_t *key, size_t key_len, uint64_t **values, size_t value_len, size_t value_words)
{
    for (size_t i = 1; i < key_len; ++i)
    {
        if (key[i - 1] <= key[i])
        {
            // sorted, no need to sort again
            continue;
        }
        uint64_t *buf = NULL;
        if (values != NULL)
        {
            buf = (uint64_t *)malloc(value_len * value_words * sizeof(uint64_t));
        }
        // Shuffle the key-value pairs to ensure that the worst-case time complexity of qsort is avoided
        srand(time(NULL));
        for (size_t i = 0; i < key_len; i++)
        {
            size_t j = rand() % (key_len - i) + i;
            size_t temp_key = key[i];
            key[i] = key[j];
            key[j] = temp_key;
            if (values != NULL)
            {
                swap_values(i, j, values, value_len, value_words, buf);
            }
        }

        // Sort the key-value pairs using quick sort
        qs(key, 0, key_len - 1, values, value_len, value_words, buf);

        free(buf);
        break;
    }

    // Remove duplicates by shifting unique elements to the front of the arrays
    size_t unique_len = 1;
    for (size_t i = 1; i < key_len; i++)
    {
        if (key[i] != key[i - 1])
        {
            key[unique_len] = key[i];
            if (values != NULL)
            {
                memcpy(values[unique_len], values[i], value_len * value_words * 8);
            }
            unique_len++;
        }
    }

    return unique_len;
}

void get_all_merkle_tree_membership_proofs(MerkleTreeMembershipProof **merkle_tree_membership_proofs, size_t merkle_trees_count,
                                           uint64_t ***query_responses,
                                           size_t *query_responses_count,
                                           size_t **query_pos,
                                           Oracle **f_i_oracles,
                                           MerkleTree **merkle_trees,
                                           const Parameters *params)
{
    for (size_t mt = 0; mt < merkle_trees_count; mt++)
    {
        size_t unique_query_pos_len;
        size_t *merkle_tree_pos;
        if (mt == 0)
        {
            size_t cur_coset_size = 1ull << params->fri_localization_parameters[0];
            unique_query_pos_len = sort_and_unique_values(query_pos[0], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0);
            assert(unique_query_pos_len);
            query_responses_count[mt] = unique_query_pos_len;
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            query_responses[mt] = (uint64_t **)malloc(unique_query_pos_len * sizeof(uint64_t *));
            Oracle merkle_tree_contents_oracles[6] = {fw_oracle, fAz_oracle, fBz_oracle, fCz_oracle, sumcheck_masking_poly_oracle, ldt_blinding_vector_oracle};
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                query_responses[mt][i] = (uint64_t *)malloc(6 * params->field_bytesize);
                for (size_t j = 0; j < 6; j++)
                    memcpy(&query_responses[mt][i][j * params->field_words],
                           &merkle_tree_contents_oracles[j].content[query_pos[0][i] * params->field_words],
                           params->field_bytesize);
                merkle_tree_pos[i] = query_pos[0][i] / cur_coset_size;
            }
        }
        else if (mt == 1)
        {
            size_t cur_coset_size = 1ull << params->fri_localization_parameters[0];
            unique_query_pos_len = sort_and_unique_values(query_pos[0], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0); // This is actually sorted when mt = 0
            assert(unique_query_pos_len);
            query_responses_count[mt] = unique_query_pos_len;
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            query_responses[mt] = (uint64_t **)malloc(unique_query_pos_len * sizeof(uint64_t *));
            Oracle merkle_tree_contents_oracles[1] = {sumcheck_h_oracle};
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                query_responses[mt][i] = (uint64_t *)malloc(params->field_bytesize);
                for (size_t j = 0; j < 1; j++)
                    memcpy(&query_responses[mt][i][j * params->field_words],
                           &merkle_tree_contents_oracles[j].content[query_pos[0][i] * params->field_words],
                           params->field_bytesize);
                merkle_tree_pos[i] = query_pos[0][i] / cur_coset_size;
            }
        }
        else
        {
            size_t cur_coset_size = 1ull << params->fri_localization_parameters[mt - 1];
            unique_query_pos_len = sort_and_unique_values(query_pos[mt - 1], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0);
            assert(unique_query_pos_len);
            query_responses_count[mt] = unique_query_pos_len;
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            query_responses[mt] = (uint64_t **)malloc(unique_query_pos_len * sizeof(uint64_t *));
            Oracle merkle_tree_contents_oracles[1] = {*f_i_oracles[mt - 1]};
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                query_responses[mt][i] = (uint64_t *)malloc(params->field_bytesize);
                assert(query_pos[mt - 1][i] < merkle_tree_contents_oracles[0].content_len);
                for (size_t j = 0; j < 1; j++)
                    memcpy(&query_responses[mt][i][j * params->field_words],
                           &merkle_tree_contents_oracles[j].content[query_pos[mt - 1][i] * params->field_words],
                           params->field_bytesize);
                merkle_tree_pos[i] = query_pos[mt - 1][i] / cur_coset_size;
            }
        }
        merkle_tree_membership_proofs[mt] = merkle_tree_membership_proof_init();
        merkle_tree_get_membership_proof(merkle_tree_membership_proofs[mt], merkle_trees[mt], merkle_tree_pos, unique_query_pos_len);
        free(merkle_tree_pos);
    }
}

int verify_all_merkle_tree_membership_proofs(size_t **unique_query_pos,
                                             MerkleTreeMembershipProof **merkle_tree_membership_proofs, uint8_t **mt_roots, size_t merkle_trees_count,
                                             uint64_t ***query_responses,
                                             size_t *query_responses_count,
                                             size_t **query_pos,
                                             const Parameters *params)
{
    int proof_is_valid = 1;
    for (size_t mt = 0; mt < merkle_trees_count; ++mt)
    {
        // For now each round has only one domain
        // Step 1) serialize query responses into leafs
        // Step 2) validate proof
        size_t unique_query_pos_len;
        size_t *cur_unique_query_pos;
        size_t *merkle_tree_pos;
        size_t merkle_tree_pos_len = 0;
        size_t cur_coset_size;
        size_t query_responses_len = 1;
        if (mt == 0)
        {
            query_responses_len = 6;
            cur_coset_size = 1ull << params->fri_localization_parameters[0];
            unique_query_pos[0] = (size_t *)malloc(cur_coset_size * params->fri_query_repetitions * sizeof(size_t));
            memcpy(unique_query_pos[0], query_pos[0], cur_coset_size * params->fri_query_repetitions * sizeof(size_t));
            unique_query_pos_len = sort_and_unique_values(unique_query_pos[0], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0);
            if (unique_query_pos_len == 0)
                return 0;
            cur_unique_query_pos = unique_query_pos[0];
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            memset(merkle_tree_pos, 0, unique_query_pos_len * sizeof(size_t));
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                size_t new_pos = unique_query_pos[0][i] / cur_coset_size;
                int found_duplicate = 0;
                for (size_t j = 0; j < merkle_tree_pos_len; j++)
                {
                    if (merkle_tree_pos[j] == new_pos)
                    {
                        found_duplicate = 1;
                        break;
                    }
                }
                if (!found_duplicate)
                {
                    merkle_tree_pos[merkle_tree_pos_len] = new_pos;
                    merkle_tree_pos_len++;
                }
            }
        }
        else if (mt == 1)
        {
            // This mt uses the same query pos as mt = 0
            // Need unique_query_pos_len, so calling sort_and_unique_values on unique_query_pos[0] again
            cur_coset_size = 1ull << params->fri_localization_parameters[0];
            unique_query_pos_len = sort_and_unique_values(unique_query_pos[0], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0);
            cur_unique_query_pos = unique_query_pos[0];
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            memset(merkle_tree_pos, 0, unique_query_pos_len * sizeof(size_t));
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                size_t new_pos = unique_query_pos[0][i] / cur_coset_size;
                int found_duplicate = 0;
                for (size_t j = 0; j < merkle_tree_pos_len; j++)
                {
                    if (merkle_tree_pos[j] == new_pos)
                    {
                        found_duplicate = 1;
                        break;
                    }
                }
                if (!found_duplicate)
                {
                    merkle_tree_pos[merkle_tree_pos_len] = new_pos;
                    merkle_tree_pos_len++;
                }
            }
        }
        else
        {
            cur_coset_size = 1ull << params->fri_localization_parameters[mt - 1];
            unique_query_pos[mt - 1] = (size_t *)malloc(cur_coset_size * params->fri_query_repetitions * sizeof(size_t));
            memcpy(unique_query_pos[mt - 1], query_pos[mt - 1], cur_coset_size * params->fri_query_repetitions * sizeof(size_t));
            unique_query_pos_len = sort_and_unique_values(unique_query_pos[mt - 1], cur_coset_size * params->fri_query_repetitions, NULL, 0, 0);
            if (unique_query_pos_len == 0)
                return 0;
            cur_unique_query_pos = unique_query_pos[mt - 1];
            merkle_tree_pos = (size_t *)malloc(unique_query_pos_len * sizeof(size_t));
            memset(merkle_tree_pos, 0, unique_query_pos_len * sizeof(size_t));
            for (size_t i = 0; i < unique_query_pos_len; i++)
            {
                size_t new_pos = unique_query_pos[mt - 1][i] / cur_coset_size;
                int found_duplicate = 0;
                for (size_t j = 0; j < merkle_tree_pos_len; j++)
                {
                    if (merkle_tree_pos[j] == new_pos)
                    {
                        found_duplicate = 1;
                        break;
                    }
                }
                if (!found_duplicate)
                {
                    merkle_tree_pos[merkle_tree_pos_len] = new_pos;
                    merkle_tree_pos_len++;
                }
            }
        }
        uint64_t **merkle_tree_leaf_columns = merkle_tree_leaf_responses_init(unique_query_pos_len, query_responses_len, cur_coset_size, params->field_bytesize);
        size_t merkle_tree_leaf_columns_len = merkle_tree_leaf_responses_len(unique_query_pos_len, cur_coset_size);
        size_t merkle_tree_leaf_column_len = merkle_tree_leaf_response_len(query_responses_len, cur_coset_size);

        query_responses_to_merkle_tree_leaf_responses(merkle_tree_leaf_columns,
                                                      cur_unique_query_pos, unique_query_pos_len,
                                                      query_responses[mt], query_responses_len, cur_coset_size,
                                                      params->field_bytesize, params->field_words);

        int result = merkle_tree_validate_membership_proof(merkle_tree_membership_proofs[mt], params, mt_roots[mt],
                                                           merkle_tree_pos, merkle_tree_pos_len,
                                                           merkle_tree_leaf_columns, merkle_tree_leaf_columns_len, merkle_tree_leaf_column_len);

        free(merkle_tree_pos);
        merkle_tree_leaf_responses_free(merkle_tree_leaf_columns, merkle_tree_leaf_columns_len);

        proof_is_valid = proof_is_valid & result;
        if (!proof_is_valid)
            return 0; // Invalid proof, return false
    }

    return proof_is_valid;
}

size_t merkle_tree_leaf_responses_len(size_t query_pos_len, size_t coset_serialization_size)
{
    return query_pos_len / coset_serialization_size;
}

size_t merkle_tree_leaf_response_len(size_t query_reponses_len, size_t coset_serialization_size)
{
    return query_reponses_len * coset_serialization_size;
}

uint64_t **merkle_tree_leaf_responses_init(size_t query_pos_len, size_t query_reponses_len, size_t coset_serialization_size, size_t field_bytesize)
{
    size_t len = merkle_tree_leaf_responses_len(query_pos_len, coset_serialization_size);
    uint64_t **merkle_tree_leaf_responses = (uint64_t **)malloc(len * sizeof(uint64_t *));
    for (size_t i = 0; i < len; i++)
    {
        merkle_tree_leaf_responses[i] = (uint64_t *)malloc(merkle_tree_leaf_response_len(query_reponses_len, coset_serialization_size) * field_bytesize);
    }
    return merkle_tree_leaf_responses;
}

void merkle_tree_leaf_responses_free(uint64_t **merkle_tree_leaf_responses, size_t merkle_tree_leaf_responses_len)
{
    for (size_t i = 0; i < merkle_tree_leaf_responses_len; i++)
    {
        free(merkle_tree_leaf_responses[i]);
    }
    free(merkle_tree_leaf_responses);
}

void query_responses_to_merkle_tree_leaf_responses(uint64_t **merkle_tree_leaf_responses,
                                                   size_t *query_pos, size_t query_pos_len,
                                                   uint64_t **query_responses, size_t query_responses_len,
                                                   size_t coset_serialization_size, size_t field_bytesize, size_t field_words)
{
    /** Elements within a given coset appear in order,
     * so we simply store the index for the next element of the coset,
     * and increment as we see new positions belonging to this coset. */
    size_t len = merkle_tree_leaf_responses_len(query_pos_len, coset_serialization_size);
    size_t *intra_coset_index = (size_t *)malloc(len * sizeof(size_t));
    memset(intra_coset_index, 0, len * sizeof(size_t));
    size_t paired_MT_leaf_index_count = 0;
    size_t *paired_MT_leaf_index = (size_t *)malloc(query_pos_len * sizeof(size_t));
    memset(paired_MT_leaf_index, 0, query_pos_len * sizeof(size_t));
    size_t *paired_MT_response_index = (size_t *)malloc(query_pos_len * sizeof(size_t));
    memset(paired_MT_response_index, 0, query_pos_len * sizeof(size_t));
    size_t next_response_index = 0;
    for (size_t i = 0; i < query_pos_len; i++)
    {
        const size_t query_position = query_pos[i];
        const size_t MT_leaf_index = query_position / coset_serialization_size;
        /* For supported domain types, new MT leaf positions appear in order of query positions.
         * If we don't yet know the index of this leaf within the queried for leaves,
         * we can find it by simply incrementing the prior leaf's index. */
        int is_paired = 0;
        size_t paired_MT_leaf_index_id;
        for (size_t j = 0; j < paired_MT_leaf_index_count; j++)
        {
            if (paired_MT_leaf_index[j] == MT_leaf_index)
            {
                is_paired = 1;
                paired_MT_leaf_index_id = j;
                break;
            }
        }
        if (!is_paired)
        {
            paired_MT_leaf_index_id = paired_MT_leaf_index_count;
            paired_MT_response_index[paired_MT_leaf_index_id] = next_response_index;
            next_response_index++;
            paired_MT_leaf_index[paired_MT_leaf_index_id] = MT_leaf_index;
            paired_MT_leaf_index_count++;
        }

        const size_t MT_response_index = paired_MT_response_index[paired_MT_leaf_index_id];
        const size_t index_in_coset = intra_coset_index[MT_response_index];
        intra_coset_index[MT_response_index]++;
        for (size_t j = 0; j < query_responses_len; j++)
        {
            const size_t oracle_index = j * coset_serialization_size;
            memcpy(&merkle_tree_leaf_responses[MT_response_index][(oracle_index + index_in_coset) * field_words], &query_responses[i][j * field_words], field_bytesize);
        }
    }
    free(intra_coset_index);
    free(paired_MT_leaf_index);
    free(paired_MT_response_index);
}
