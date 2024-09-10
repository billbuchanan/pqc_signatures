#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "virtual_oracles.h"
#include "aurora_inner.h"
#include "field.h"
#include "fft.h"
#include "oracles.h"
#include "poly.h"
#include "r1cs.h"

Oracle **init_virtual_oracles(const R1CS *r1cs, const Parameters *params)
{
    Oracle **oracles = (Oracle **)malloc(6 * sizeof(Oracle *));
    FzOracleData fz_data;
    memset(&fz_data, 0, sizeof(fz_data));
    fz_data.primary_input_len = params->input_variable_domain.size - 1;

    // oracles[0] = fz_virtual_oracle;
    Oracle *fz_constituent[1] = {&fw_oracle};
    oracles[0] = new_fz_oracle(&fz_data,
                               params->codeword_domain.size,
                               params->variable_domain.size + params->query_bound,
                               &params->codeword_domain,
                               fz_constituent, fz_oracle.constituent_oracles_len);
    assert(oracles[0]);

    RowcheckOracleData rowcheck_data;
    memset(&rowcheck_data, 0, sizeof(rowcheck_data));
    rowcheck_data.params = params;

    // oracles[1] = rowcheck_virtual_oracle;
    Oracle *rowcheck_constituent[3] = {&fAz_oracle, &fBz_oracle, &fCz_oracle};
    oracles[1] = new_rowcheck_oracle(&rowcheck_data,
                                     params->codeword_domain.size,
                                     params->constraint_domain.size + 2 * params->query_bound - 1,
                                     &params->codeword_domain,
                                     rowcheck_constituent,
                                     rowcheck_oracle.constituent_oracles_len);
    assert(oracles[1]);

    size_t multi_lincheck_content_deg = 0;
    if (oracles[0]->content_deg > fAz_oracle.content_deg)
        multi_lincheck_content_deg = params->summation_domain.size + oracles[0]->content_deg - 1;
    else
        multi_lincheck_content_deg = params->summation_domain.size + fAz_oracle.content_deg - 1;

    MultilinCheckOracleData multi_lincheck_data;
    memset(&multi_lincheck_data, 0, sizeof(multi_lincheck_data));
    multi_lincheck_data.params = params;
    multi_lincheck_data.r1cs = r1cs;

    // oracles[2] = multi_lincheck_virtual_oracle;
    Oracle *multi_lincheck_constituent[4] = {oracles[0], &fAz_oracle, &fBz_oracle, &fCz_oracle};
    oracles[2] = new_multi_lincheck_oracle(&multi_lincheck_data,
                                           params->codeword_domain.size,
                                           multi_lincheck_content_deg,
                                           &params->codeword_domain,
                                           multi_lincheck_constituent,
                                           multi_lincheck_oracle.constituent_oracles_len);
    assert(oracles[2]);

    CombinedFOracleData combined_f_data;
    memset(&combined_f_data, 0, sizeof(combined_f_data));
    combined_f_data.params = params;
    // oracles[3] = combined_f_virtual_oracle;
    Oracle *combined_f_constituent[2] = {&sumcheck_masking_poly_oracle, oracles[2]};
    oracles[3] = new_combined_f_oracle(&combined_f_data,
                                       params->codeword_domain.size,
                                       oracles[2]->content_deg,
                                       &params->codeword_domain,
                                       combined_f_constituent,
                                       combined_f_oracle.constituent_oracles_len);
    assert(oracles[3]);

    // Compute content of sumcheck_g_oracle before usage
    SumcheckGOracleData sumcheck_g_data;

    memset(&sumcheck_g_data, 0, sizeof(sumcheck_g_data));
    sumcheck_g_data.params = params;
    // oracles[4] = sumcheck_g_virtual_oracle;
    Oracle *sumcheck_g_constituent[2] = {oracles[3], &sumcheck_h_oracle};
    oracles[4] = new_sumcheck_g_oracle(&sumcheck_g_data,
                                       params->codeword_domain.size,
                                       params->summation_domain.size - 1,
                                       &params->codeword_domain,
                                       sumcheck_g_constituent,
                                       sumcheck_g_oracle.constituent_oracles_len);
    assert(oracles[4]);

    /* First set of codewords: the original purported codewords we're testing. */
    Oracle *combined_ldt_virtual_oracle_constituent_oracles[9] = {&sumcheck_masking_poly_oracle,
                                                                  &sumcheck_h_oracle,
                                                                  oracles[4],
                                                                  &fw_oracle,
                                                                  &fAz_oracle,
                                                                  &fBz_oracle,
                                                                  &fCz_oracle,
                                                                  oracles[1],
                                                                  &ldt_blinding_vector_oracle};
    size_t *combined_ldt_virtual_oracle_input_oracle_degrees = (size_t *)malloc(9 * sizeof(size_t));
    if (oracles[0]->content_deg > fAz_oracle.content_deg)
    {
        combined_ldt_virtual_oracle_input_oracle_degrees[0] = params->summation_domain.size + oracles[0]->content_deg - 1;
        combined_ldt_virtual_oracle_input_oracle_degrees[1] = oracles[0]->content_deg - 1;
    }
    else
    {
        combined_ldt_virtual_oracle_input_oracle_degrees[0] = params->summation_domain.size + fAz_oracle.content_deg - 1;
        combined_ldt_virtual_oracle_input_oracle_degrees[1] = fAz_oracle.content_deg - 1;
    }
    combined_ldt_virtual_oracle_input_oracle_degrees[2] = params->summation_domain.size - 1;
    combined_ldt_virtual_oracle_input_oracle_degrees[3] = params->variable_domain.size - params->input_variable_domain.size + params->query_bound;
    combined_ldt_virtual_oracle_input_oracle_degrees[4] = params->constraint_domain.size + params->query_bound;
    combined_ldt_virtual_oracle_input_oracle_degrees[5] = params->constraint_domain.size + params->query_bound;
    combined_ldt_virtual_oracle_input_oracle_degrees[6] = params->constraint_domain.size + params->query_bound;
    combined_ldt_virtual_oracle_input_oracle_degrees[7] = params->constraint_domain.size + 2 * params->query_bound - 1;
    combined_ldt_virtual_oracle_input_oracle_degrees[8] = params->max_ldt_tested_degree_bound;

    CombinedLDTOracleData combined_ldt_data;
    memset(&combined_ldt_data, 0, sizeof(combined_ldt_data));
    combined_ldt_data.params = params;
    combined_ldt_data.input_oracle_degrees_len = 9;
    combined_ldt_data.input_oracle_degrees = combined_ldt_virtual_oracle_input_oracle_degrees;
    // oracles[5] = combined_ldt_virtual_oracle;
    oracles[5] = new_combined_ldt_oracle(&combined_ldt_data,
                                         params->codeword_domain.size,
                                         params->max_ldt_tested_degree_bound,
                                         &params->codeword_domain,
                                         combined_ldt_virtual_oracle_constituent_oracles,
                                         combined_ldt_oracle.constituent_oracles_len);
    assert(oracles[5]);
    return oracles;
}

void deinit_virtual_oracles(Oracle **oracles)
{
    free((size_t *)((CombinedLDTOracleData *)oracles[5]->data)->input_oracle_degrees);

    free_fz_oracle(oracles[0]);
    free_rowcheck_oracle(oracles[1]);
    free_multi_lincheck_oracle(oracles[2]);
    free_combined_f_oracle(oracles[3]);
    free_sumcheck_g_oracle(oracles[4]);
    free_combined_ldt_oracle(oracles[5]);

    free(oracles);
}

uint64_t *new_oracle_evaulation_result(const Parameters *params)
{
    return (uint64_t *)malloc(params->field_bytesize);
}

void free_oracle_evaulation_result(uint64_t *result)
{
    free(result);
}

static inline __attribute__((always_inline)) void var_set_zero(uint64_t *var, const Parameters *params)
{
    memset(var, 0, params->field_bytesize);
}

static inline __attribute__((always_inline)) uint64_t *new_tmp_var(const Parameters *params)
{
    return new_oracle_evaulation_result(params);
}

int fz_oracle_set_primary_input(Oracle *oracle, const uint64_t *primary_input, size_t primary_input_len)
{
    FzOracleData *data = (FzOracleData *)oracle->data;
    if (data->primary_input_len != primary_input_len)
    {
        fprintf(stderr, "Primary input size does not match the previously declared size.");
        return 1;
    }
    data->_primary_input = primary_input;
    return 0;
}

int fz_oracle_evaluate_content(Oracle *oracle, const uint64_t *f1v_coeff, const size_t f1v_coeff_len, const Parameters *params)
{

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);

    size_t input_vp_len = vanishing_polynomial_len(&params->input_variable_domain);
    uint64_t *input_vp = (uint64_t *)malloc(input_vp_len * params->field_bytesize);
    vanishing_polynomial(input_vp, input_vp_len, &params->input_variable_domain, params);
    /* TODO (low priority): Use that Z_{1,v} | L is a |1,v| to 1 map */
    uint64_t *input_vp_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    poly_eval_over_domain(input_vp_over_codeword_domain, input_vp, input_vp_len, &params->codeword_domain, params->field_words, params->field_bytesize);
    free(input_vp);

    uint64_t *f_1v_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(f_1v_over_codeword_domain, f1v_coeff, f1v_coeff_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);

    memcpy(oracle->content, f_1v_over_codeword_domain, params->codeword_domain.size * params->field_bytesize);
    for (size_t i = 0; i < params->codeword_domain.size; ++i)
    {
        uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);
        // fw_over_codeword_domain = oracle->constituent_oracles[0]->content
        memcpy(tmp, &oracle->constituent_oracles[0]->content[i * params->field_words], params->field_bytesize);
        field_mulEqual(tmp, &input_vp_over_codeword_domain[i * params->field_words], params->field_words);
        field_addEqual(&oracle->content[i * params->field_words], tmp, params->field_words);
        free(tmp);
    }

    free(input_vp_over_codeword_domain);
    free(f_1v_over_codeword_domain);
    return 0;
}

static int fz_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{
    if (constituent_evaluations_len != 1)
    {
        fprintf(stderr, "fz_virtual_oracle has one constituent oracle.");
        return 1;
    }

    const size_t field_words = params->field_words;
    const FzOracleData *data = (FzOracleData *)oracle->data;
    assert(data->_primary_input);

    const uint64_t *fw_X = &constituent_evaluations[0];
    size_t result_len = params->input_variable_domain.size;
    uint64_t *L_X_over_input_variable_domain = (uint64_t *)malloc(result_len * params->field_bytesize);
    lagrange_cache(L_X_over_input_variable_domain, evaluation_point, &params->input_variable_domain, params);
    uint64_t *f1v_X = &L_X_over_input_variable_domain[0]; // for the constant term
    uint64_t *tmp = new_tmp_var(params);
    for (size_t i = 0; i < data->primary_input_len; ++i)
    {
        field_mul(tmp, &L_X_over_input_variable_domain[field_words * (i + 1)], &data->_primary_input[field_words * i]);
        field_addEqual(f1v_X, tmp, field_words);
    }
    uint64_t *input_vp_X = new_tmp_var(params);
    size_t input_vp_len = vanishing_polynomial_len(&params->input_variable_domain);
    uint64_t *input_vp = (uint64_t *)malloc(input_vp_len * params->field_bytesize);
    vanishing_polynomial(input_vp, input_vp_len, &params->input_variable_domain, params);
    poly_eval(input_vp_X, input_vp, input_vp_len, evaluation_point, field_words);

    field_mul(result, fw_X, input_vp_X);
    field_addEqual(result, f1v_X, field_words);

    free(input_vp);
    free(input_vp_X);
    free(tmp);
    free(L_X_over_input_variable_domain);
    return 0;
}

int rowcheck_oracle_evaluate_content(Oracle *oracle, const Parameters *params)
{
    for (size_t i = 0; i < oracle->constituent_oracles_len; i++)
    {
        if (!oracle->constituent_oracles[i]->content)
            return 1;
    }

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    assert(oracle->content == NULL);
    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);
    const size_t num_cosets_of_H = params->codeword_domain.size / params->constraint_domain.size;
    /** We iterate over all elements in order.
     *  We use 2 loops, so we always know which coset a particular position is in.
     *  The outer loop specifies which coset we are looking at.
     *  The inner loop ranges over all elements of that coset.
     */
    uint64_t *result_basis = (uint64_t *)malloc(num_cosets_of_H * params->field_bytesize);
    uint64_t *transformed_basis = (uint64_t *)malloc(params->codeword_domain.basis_len * params->field_bytesize);

    size_t constraint_vp_len = params->constraint_domain.size + 1;
    uint64_t *constraint_vp = (uint64_t *)malloc(constraint_vp_len * params->field_bytesize);
    vanishing_polynomial(constraint_vp, constraint_vp_len, &params->constraint_domain, params);
    for (size_t i = 0; i < params->codeword_domain.basis_len; i++)
        poly_eval(&transformed_basis[i * params->field_words], constraint_vp, constraint_vp_len, &params->codeword_domain.basis[i * params->field_words], params->field_words);
    uint64_t *transformed_shift = (uint64_t *)malloc(params->field_bytesize);
    poly_eval(transformed_shift, constraint_vp, constraint_vp_len, params->codeword_domain.shift, params->field_words);
    size_t result_basis_count = 0;
    for (size_t i = 0; i < params->codeword_domain.basis_len; i++)
    {
        int is_dup = 0;
        /** The basis element was in the kernel of the vanishing polynomial. */
        if (is_zero(&transformed_basis[i * params->field_words], params->field_words))
        {
            // transformed_basis is constraint_vp(codeword_domain.basis)
            // since codeword_domain.basis = constraint_domain.basis + [some other values]
            // first few transformed_basis are definitely zero
            // Is there any optimization about this if this still holds in cantor basis?
            continue;
        }
        for (size_t j = 0; j < result_basis_count; j++)
        {
            if (field_equal(&result_basis[j * params->field_words], &transformed_basis[i * params->field_words], params->field_words))
            {
                is_dup = 1;
                break;
            }
        }
        if (!is_dup)
        {
            memcpy(&result_basis[result_basis_count * params->field_words], &transformed_basis[i * params->field_words], params->field_bytesize);
            result_basis_count++;
        }
    }
    free(transformed_basis);
    Domain rowcheck_domain = {.size = 1ull << result_basis_count,
                              .basis = result_basis,
                              .basis_len = result_basis_count,
                              .shift = transformed_shift};
    size_t rowcheck_oracle_content_count = 0;
    uint64_t *Z = (uint64_t *)malloc(params->field_bytesize);
    uint64_t *Z_inv = (uint64_t *)malloc(params->field_bytesize);
    for (size_t i = 0; i < num_cosets_of_H; i++)
    {
        get_ith_field_element(Z, &rowcheck_domain, i, params);
        field_inv(Z_inv, Z);
        const size_t coset_pos_upper_bound = (i + 1) * params->constraint_domain.size;
        for (size_t cur_pos = i * params->constraint_domain.size;
             cur_pos < coset_pos_upper_bound; cur_pos++)
        {
            uint64_t *result = &oracle->content[rowcheck_oracle_content_count * params->field_words];
            field_mul(result, &oracle->constituent_oracles[0]->content[cur_pos * params->field_words], &oracle->constituent_oracles[1]->content[cur_pos * params->field_words]);
            field_subEqual(result, &oracle->constituent_oracles[2]->content[cur_pos * params->field_words], params->field_words);
            field_mulEqual(result, Z_inv, params->field_words);
            rowcheck_oracle_content_count++;
        }
    }
    free(result_basis);
    free(transformed_shift);
    free(constraint_vp);
    free(Z);
    free(Z_inv);

    return 0;
}

int rowcheck_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{

    if (constituent_evaluations_len != 3)
    {
        fprintf(stderr, "rowcheck_ABC has three constituent oracles.");
        return 1;
    }

    const size_t field_words = params->field_words;
    const RowcheckOracleData *data = (RowcheckOracleData *)oracle->data;

    const uint64_t *A_X = &constituent_evaluations[0];
    const uint64_t *B_X = &constituent_evaluations[field_words];
    const uint64_t *C_X = &constituent_evaluations[2 * field_words];
    uint64_t *Z_X = new_tmp_var(params);
    poly_eval(Z_X, data->_Z, data->_Z_len, evaluation_point, params->field_words);
    uint64_t *Z_X_inv = new_tmp_var(params);
    field_inv(Z_X_inv, Z_X);

    // (Z_X_inv * (A_X * B_X - C_X)
    field_mul(result, A_X, B_X);
    field_subEqual(result, C_X, field_words);
    field_mulEqual(result, Z_X_inv, field_words);
    free(Z_X_inv);
    free(Z_X);
    return 0;
}

static void multi_lincheck_oracle_compute_p_alpha_prime(Oracle *oracle, const Parameters *params)
{
    MultilinCheckOracleData *data = (MultilinCheckOracleData *)oracle->data;

    /** Set alpha powers */
    uint64_t tmp[params->field_words];
    uint64_t alpha_powers[params->constraint_domain.size * params->field_words];
    memcpy(tmp, params->field_one, params->field_bytesize);

    for (size_t j = 0; j < params->constraint_domain.size; j++)
    {
        memcpy(&alpha_powers[j * params->field_words], tmp, params->field_bytesize);
        field_mulEqual(tmp, data->_alpha, params->field_words);
    }

    /** This essentially places alpha powers into the correct spots,
     *  such that the zeroes when the |constraint domain| < summation domain
     *  are placed correctly. */
    uint64_t *p_alpha_prime_over_summation_domain = (uint64_t *)malloc(params->summation_domain.size * params->field_bytesize);
    memset(p_alpha_prime_over_summation_domain, 0, params->summation_domain.size * params->field_bytesize);
    // size_t element_index = summation_domain.reindex_by_subset(constraint_domain_dimension, j);
    // p_alpha_prime_over_summation_domain[element_index] = alpha_powers[i]; for i = [0..params->constraint_domain.size]
    // This is actually copying alpha_powers to the beginning of p_alpha_prime_over_summation_domain
    // Since element_index is equal to j if using additive field
    memcpy(p_alpha_prime_over_summation_domain, alpha_powers, params->constraint_domain.size * params->field_bytesize);

    data->_p_alpha_prime_len = params->summation_domain.size;
    data->_p_alpha_prime = (uint64_t *)malloc(params->summation_domain.size * params->field_bytesize);
    ifft(data->_p_alpha_prime, p_alpha_prime_over_summation_domain, params->field_words, params->summation_domain.size, params->summation_domain.shift);
    free(p_alpha_prime_over_summation_domain);
}

static void multi_lincheck_oracle_compute_p_alpha_ABC(Oracle *oracle, const Parameters *params)
{
    MultilinCheckOracleData *data = (MultilinCheckOracleData *)oracle->data;
    const R1CS *r1cs = data->r1cs;
    const uint64_t *r_Mz = data->_r_Mz;
    uint64_t tmp[params->field_words];
    uint64_t alpha_powers[params->constraint_domain.size * params->field_words];
    memcpy(tmp, params->field_one, params->field_bytesize);

    for (size_t j = 0; j < params->constraint_domain.size; j++)
    {
        memcpy(&alpha_powers[j * params->field_words], tmp, params->field_bytesize);
        field_mulEqual(tmp, data->_alpha, params->field_words);
    }

    /* Set p_alpha_ABC_evals */
    uint64_t *p_alpha_ABC_evals = (uint64_t *)malloc(params->summation_domain.size * params->field_bytesize);
    memset(p_alpha_ABC_evals, 0, params->summation_domain.size * params->field_bytesize);
    // M is cons_domain X var_domain
    uint64_t **matrics[3] = {r1cs->A, r1cs->B, r1cs->C};
    for (size_t m = 0; m < 3; ++m)
    {
        uint64_t **M = matrics[m];
        for (size_t i = 0; i < params->constraint_domain.size; i++)
        {
            for (size_t j = 0; j < params->variable_domain.size; j++)
            {
                if (is_zero(&M[i][j * params->field_words], params->field_words))
                    continue;
                // const size_t variable_index = this->variable_domain_.reindex_by_subset(
                //     this->input_variable_dim_, term.index_);
                // const size_t summation_index = this->summation_domain_.reindex_by_subset(
                //     this->variable_domain_.dimension(), variable_index);
                // summation_index = variable_index = term.index_ if using additive field
                const size_t variable_index = j;
                const size_t summation_index = variable_index;
                // p_alpha_ABC_evals[summation_index] += this->r_Mz_[m_index] * term.coeff_ * alpha_powers[i]
                field_mul(tmp, &r_Mz[m * params->field_words], &M[i][j * params->field_words]);
                field_mulEqual(tmp, &alpha_powers[i * params->field_words], params->field_words);
                field_addEqual(&p_alpha_ABC_evals[summation_index * params->field_words], tmp, params->field_words);
            }
        }
    }
    data->_p_alpha_ABC_len = params->summation_domain.size;
    data->_p_alpha_ABC = (uint64_t *)malloc(params->summation_domain.size * params->field_bytesize);
    ifft(data->_p_alpha_ABC, p_alpha_ABC_evals, params->field_words, params->summation_domain.size, params->field_zero);
    free(p_alpha_ABC_evals);
}

int multi_lincheck_oracle_set_challenge(Oracle *oracle, const uint64_t *alpha, const uint64_t *r_Mz, size_t r_Mz_len)
{
    if (r_Mz_len != 3)
    {
        fprintf(stderr, "Not enough random linear combination coefficients were provided\n");
        return 1;
    }
    MultilinCheckOracleData *data = (MultilinCheckOracleData *)oracle->data;
    if (data->_alpha)
        free(data->_alpha);
    if (data->_r_Mz)
        free(data->_r_Mz);
    data->_alpha = (uint64_t *)malloc(data->params->field_bytesize);
    memcpy(data->_alpha, alpha, data->params->field_bytesize);
    data->_r_Mz = (uint64_t *)malloc(data->params->field_bytesize * r_Mz_len);
    memcpy(data->_r_Mz, r_Mz, data->params->field_bytesize * r_Mz_len);
    data->_r_Mz_len = r_Mz_len;

    multi_lincheck_oracle_compute_p_alpha_prime(oracle, data->params);
    multi_lincheck_oracle_compute_p_alpha_ABC(oracle, data->params);

    return 0;
}

int multi_lincheck_evaluate_content(Oracle *oracle, const Parameters *params)
{
    for (size_t i = 0; i < oracle->constituent_oracles_len; i++)
    {
        if (!oracle->constituent_oracles[i]->content)
            return 1;
    }

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    const MultilinCheckOracleData *data = (MultilinCheckOracleData *)oracle->data;
    assert(data->_r_Mz);

    assert(oracle->content == NULL);
    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);

    uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);

    /* p_{alpha}^1 in [BCRSVW18] */
    uint64_t *p_alpha_prime_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(p_alpha_prime_over_codeword_domain, data->_p_alpha_prime, data->_p_alpha_prime_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);

    /* p_{alpha}^2 in [BCRSVW18] */
    uint64_t *p_alpha_ABC_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(p_alpha_ABC_over_codeword_domain, data->_p_alpha_ABC, data->_p_alpha_ABC_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);

    /* Random linear combination of Mz's */
    uint64_t *f_combined_Mz = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    memset(f_combined_Mz, 0, params->codeword_domain.size * params->field_bytesize);
    for (size_t i = 0; i < params->codeword_domain.size; i++)
    {
        field_mul(tmp, &data->_r_Mz[0], &oracle->constituent_oracles[1]->content[i * params->field_words]);
        field_addEqual(&f_combined_Mz[i * params->field_words], tmp, params->field_words);

        field_mul(tmp, &data->_r_Mz[params->field_words], &oracle->constituent_oracles[2]->content[i * params->field_words]);
        field_addEqual(&f_combined_Mz[i * params->field_words], tmp, params->field_words);

        field_mul(tmp, &data->_r_Mz[2 * params->field_words], &oracle->constituent_oracles[3]->content[i * params->field_words]);
        field_addEqual(&f_combined_Mz[i * params->field_words], tmp, params->field_words);
    }

    // TODO: Investigate if combining these loops improves speed.
    // It would improve cache efficiency, but its not immediate how it will affect compiler optimizations
    // (re-structuring loop for data parallelism, unrolling, etc.)
    // However, this time may become negligible after making the result over an intermediate sumcheck domain
    memcpy(oracle->content, f_combined_Mz, params->codeword_domain.size * params->field_bytesize);
    for (size_t i = 0; i < params->codeword_domain.size; ++i)
    {
        // oracle->content[i]
        // = f_combined_Mz[i] * p_alpha_prime_over_codeword_domain[i]
        //    - fz[i] * p_alpha_ABC_over_codeword_domain[i];
        field_mulEqual(&oracle->content[i * params->field_words],
                       &p_alpha_prime_over_codeword_domain[i * params->field_words], params->field_words);

        // fz = multi_lincheck_virtual_oracle->constituent_oracles[0]
        memcpy(tmp, &oracle->constituent_oracles[0]->content[i * params->field_words], params->field_bytesize);
        field_mulEqual(tmp, &p_alpha_ABC_over_codeword_domain[i * params->field_words], params->field_words);
        field_subEqual(&oracle->content[i * params->field_words], tmp, params->field_words);
    }
    free(p_alpha_prime_over_codeword_domain);
    free(p_alpha_ABC_over_codeword_domain);
    free(f_combined_Mz);
    free(tmp);

    return 0;
}

static int multi_lincheck_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{
    // if (constituent_oracle_evaluations.size() != this->matrices_.size() + 1)
    if (constituent_evaluations_len != 4)
    {
        fprintf(stderr, "multi_lincheck uses more constituent oracles than what was provided.\n");
        return 1;
    }
    const size_t field_words = params->field_words;
    const MultilinCheckOracleData *data = (MultilinCheckOracleData *)oracle->data;
    assert(data->_r_Mz);
    assert(data->_r_Mz_len);
    assert(data->_p_alpha_ABC);
    assert(data->_p_alpha_ABC_len);
    assert(data->_p_alpha_prime);
    assert(data->_p_alpha_prime_len);
    uint64_t *p_alpha_prime_X = new_tmp_var(params);
    uint64_t *p_alpha_ABC_X = new_tmp_var(params);
    poly_eval(p_alpha_ABC_X, data->_p_alpha_ABC, data->_p_alpha_ABC_len, evaluation_point, field_words);
    poly_eval(p_alpha_prime_X, data->_p_alpha_prime, data->_p_alpha_prime_len, evaluation_point, field_words);
    const uint64_t *fz_X = &constituent_evaluations[0];
    uint64_t *f_combined_Mz_x = result;
    var_set_zero(f_combined_Mz_x, params);
    uint64_t *tmp = new_tmp_var(params);
    for (size_t i = 0; i < data->_r_Mz_len; ++i)
    {
        field_mul(tmp, &data->_r_Mz[field_words * i], &constituent_evaluations[field_words * (i + 1)]);
        field_addEqual(f_combined_Mz_x, tmp, field_words);
    }

    field_mul(tmp, fz_X, p_alpha_ABC_X);

    field_mulEqual(f_combined_Mz_x, p_alpha_prime_X, field_words);
    field_subEqual(f_combined_Mz_x, tmp, field_words);

    free(tmp);
    free(p_alpha_prime_X);
    free(p_alpha_ABC_X);
    return 0;
}

int combined_f_oracle_set_random_coefficients(Oracle *oracle, const uint64_t *random_coefficients, size_t random_coefficients_len)
{
    if (random_coefficients_len != oracle->constituent_oracles_len)
    {
        fprintf(stderr, "Got %lu random coefficients\n", random_coefficients_len);
        fprintf(stderr, "Random Linear Combination Oracle: "
                        "Expected same number of random coefficients as oracles.\n");
        return 1;
    }

    CombinedFOracleData *data = (CombinedFOracleData *)oracle->data;
    if (data->_random_coefficients)
        free(data->_random_coefficients);
    data->_random_coefficients = (uint64_t *)malloc(data->params->field_bytesize * random_coefficients_len);
    memcpy(data->_random_coefficients, random_coefficients, data->params->field_bytesize * random_coefficients_len);
    data->_random_coefficients_len = random_coefficients_len;
    return 0;
}

int combined_f_oracle_evaluate_content(Oracle *oracle, const Parameters *params)
{
    for (size_t i = 0; i < oracle->constituent_oracles_len; i++)
    {
        if (!oracle->constituent_oracles[i]->content)
            return 1;
    }

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    assert(oracle->content == NULL);
    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);

    const CombinedFOracleData *data = (CombinedFOracleData *)oracle->data;
    assert(data->_random_coefficients);
    uint64_t *tmp0 = (uint64_t *)malloc(params->field_bytesize);
    uint64_t *tmp1 = (uint64_t *)malloc(params->field_bytesize);

    for (size_t j = 0; j < params->codeword_domain.size; ++j)
    {
        field_mul(tmp0, &data->_random_coefficients[0], &oracle->constituent_oracles[0]->content[j * params->field_words]);
        field_mul(tmp1, &data->_random_coefficients[1 * params->field_words], &oracle->constituent_oracles[1]->content[j * params->field_words]);
        field_add(&oracle->content[j * params->field_words], tmp0, tmp1, params->field_words);
    }

    free(tmp0);
    free(tmp1);
    return 0;
}

static int combined_f_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{

    if (constituent_evaluations_len != oracle->constituent_oracles_len)
    {
        fprintf(stderr, "Expected same number of evaluations as in registration.\n");
        return 1;
    }

    const size_t field_words = params->field_words;
    const CombinedFOracleData *data = (CombinedFOracleData *)oracle->data;
    assert(data->_random_coefficients);
    assert(data->_random_coefficients_len);

    var_set_zero(result, params);
    uint64_t *tmp = new_tmp_var(params);
    for (size_t i = 0; i < constituent_evaluations_len; ++i)
    {
        field_mul(tmp, &data->_random_coefficients[i * field_words], &constituent_evaluations[i * field_words]);
        field_addEqual(result, tmp, field_words);
    }
    free(tmp);
    return 0;
}

int sumcheck_g_oracle_set_claimed_sum(Oracle *oracle, const uint64_t *claimed_sum)
{
    SumcheckGOracleData *data = (SumcheckGOracleData *)oracle->data;
    const Parameters *params = data->params;
    if (data->field_subset_type == AffineSubspaceType)
    {
        // this->eps_inv_times_claimed_sum_ = this->eps_.inverse() * this->claimed_sum_;
        data->_eps_inv_times_claimed_sum = new_tmp_var(params);
        assert(data->_eps_inv_times_claimed_sum);
        field_inv(data->_eps_inv_times_claimed_sum, data->_eps);
        field_mulEqual(data->_eps_inv_times_claimed_sum, claimed_sum, params->field_words);
        return 0;
    }
    fprintf(stderr, "unsupported field_subset_type\n");
    return 1;
}

int sumcheck_g_oracle_evaluate_content(Oracle *oracle, const Parameters *params)
{
    for (size_t i = 0; i < oracle->constituent_oracles_len; i++)
    {
        if (!oracle->constituent_oracles[i]->content)
            return 1;
    }

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);

    memcpy(oracle->content, oracle->constituent_oracles[0]->content, oracle->constituent_oracles[0]->content_len * params->field_bytesize);
    // const vector<FieldT> Z_over_L = this->Z_.evaluations_over_field_subset(this->codeword_domain_);

    size_t summation_vp_len = vanishing_polynomial_len(&params->summation_domain);
    uint64_t *summation_vp = (uint64_t *)malloc(summation_vp_len * params->field_bytesize);
    vanishing_polynomial(summation_vp, summation_vp_len, &params->summation_domain, params);
    uint64_t *summation_vp_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    poly_eval_over_domain(summation_vp_over_codeword_domain, summation_vp, summation_vp_len, &params->codeword_domain, params->field_words, params->field_bytesize);
    /** In the additive case this is computing p in RS[L, (|H|-1) / L],
     *  where p as described in the paper is:
     *  p(x) = eps * f(x) - mu * x^{|H| - 1} - eps * Z_H(x) * h(x)
     *
     *  It is equivalent to check if the following is low degree:
     *  p'(x) = eps^{-1} * p(x)
     *        = (f(x) - eps^{-1} * mu * x^{|H| - 1} - Z_H(x) * h(x))
     *  We use the latter due to the reduced prover time.
     */

    // const vector<FieldT>
    //     eps_inv_times_claimed_sum_times_x_to_H_minus_1 =
    //         constant_times_subspace_to_order_H_minus_1(0, params->codeword_domain, params->summation_domain.size);
    // In our case this produces an array full of zeros

    /** Compute p, by performing the correct arithmetic on the evaluations */
    uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);
    for (size_t i = 0; i < oracle->content_len; ++i)
    {
        // result->operator[](i) -= (eps_inv_times_claimed_sum_times_x_to_H_minus_1[i] + Z_over_L[i] * constituent_oracle_evaluations[1]->operator[](i));
        // eps_inv_times_claimed_sum_times_x_to_H_minus_1 is an array full of zeros in our cases
        field_mul(tmp, &summation_vp_over_codeword_domain[i * params->field_words], &oracle->constituent_oracles[1]->content[i * params->field_words]);
        field_subEqual(&oracle->content[i * params->field_words], tmp, params->field_words);
    }
    free(summation_vp);
    free(summation_vp_over_codeword_domain);
    free(tmp);
    return 0;
}

static int sumcheck_g_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{
    if (constituent_evaluations_len != 2)
    {
        fprintf(stderr, "sumcheck_g_oracle has two constituent oracles\n");
        return 1;
    }

    const SumcheckGOracleData *data = (SumcheckGOracleData *)oracle->data;
    assert(data->_eps_inv_times_claimed_sum);
    const size_t field_words = params->field_words;
    const uint64_t *f_at_x = &constituent_evaluations[0];
    const uint64_t *h_at_x = &constituent_evaluations[field_words];
    uint64_t *Z_at_x = new_tmp_var(params);
    poly_eval(Z_at_x, data->_Z, data->_Z_len, evaluation_point, field_words);

    // var_set_zero(result, params);
    if (data->field_subset_type == AffineSubspaceType)
    {
        //     return (f_at_x
        //         - this->eps_inv_times_claimed_sum_ * libff::power(evaluation_point, this->summation_domain_.num_elements() - 1)
        //         - Z_at_x * h_at_x);
        assert(data->_eps_inv_times_claimed_sum);
        uint64_t *tmp = new_tmp_var(params);
        field_pow(tmp, evaluation_point, data->params->summation_domain.size - 1, field_words);
        field_mulEqual(tmp, data->_eps_inv_times_claimed_sum, field_words);
        field_sub(result, f_at_x, tmp, field_words);
        field_mul(tmp, Z_at_x, h_at_x);
        field_subEqual(result, tmp, field_words);

        free(tmp);
        free(Z_at_x);
        return 0;
    }
    free(Z_at_x);
    fprintf(stderr, "unsupported field_subset_type\n");
    return 1;
}

int combined_ldt_oracle_set_random_coefficients(Oracle *oracle, const uint64_t *random_coefficients, size_t random_coefficients_len)
{
    CombinedLDTOracleData *data = (CombinedLDTOracleData *)oracle->data;
    assert(data->_coefficients == NULL);
    if (random_coefficients_len != data->_num_random_coefficients)
    {
        fprintf(stderr, "Expected the nunmber of random coefficients to be twice the number of oracles.");
        return 1;
    }
    data->_coefficients_len = random_coefficients_len + 1;
    data->_coefficients = (uint64_t *)malloc(data->_coefficients_len * data->params->field_bytesize);
    memcpy(data->_coefficients, data->params->field_one, data->params->field_bytesize);
    memcpy(&data->_coefficients[data->params->field_words], random_coefficients, random_coefficients_len * data->params->field_bytesize);
    return 0;
}

int combined_ldt_oracle_evaluate_content(Oracle *oracle, const Parameters *params)
{
    for (size_t i = 0; i < oracle->constituent_oracles_len; i++)
    {
        if (!oracle->constituent_oracles[i]->content)
            return 1;
    }

    CombinedLDTOracleData *data = (CombinedLDTOracleData *)oracle->data;

    if (oracle->content_len == 0 || oracle->content_len > params->codeword_domain.size)
        return 1;

    assert(oracle->content == NULL);
    oracle->content = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);
    memset(oracle->content, 0, oracle->content_len * params->field_bytesize);

    uint64_t *temp = (uint64_t *)malloc(oracle->content_len * params->field_bytesize);
    uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);
    for (size_t i = 0; i < oracle->constituent_oracles_len; ++i)
    {
        if (data->input_oracle_degrees[i] == data->_max_degree)
        {
            assert(oracle->constituent_oracles[i]->content_len == oracle->content_len);
            // Handle maximal degree oracles
            // This assumes all combined_ldt_oracle.constituent_oracles are not used after this computation
            memcpy(temp, oracle->constituent_oracles[i]->content, oracle->constituent_oracles[i]->content_len * params->field_bytesize);
            poly_scalarMulEqual(temp, oracle->content_len, &data->_coefficients[i * params->field_words], params->field_words);
            poly_addEqual(oracle->content, oracle->content_len, temp, oracle->content_len, params->field_words);
        }
        else
        {
            size_t submaximal_oracle_index; // This has different meaning from the variable with same name in original Aurora, but eventually result will be the same
            for (submaximal_oracle_index = 0; submaximal_oracle_index < data->_submaximal_oracle_indices_len; submaximal_oracle_index++)
            {
                if (data->_submaximal_oracle_indices[submaximal_oracle_index] == i)
                    break;
            }

            /* Otherwise, raise element in the codeword domain to the power of the degree difference. */
            uint64_t *bump_factors = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
            for (size_t i = 0; i < params->codeword_domain.size; i++)
                memcpy(&bump_factors[i * params->field_words], params->field_one, params->field_bytesize);
            size_t bump_factors_exponent = data->_max_degree - data->input_oracle_degrees[submaximal_oracle_index];
            // bump_factors = subset_element_powers(codeword_domain.size, bump_factors_exponent)
            if (is_power_of_2(bump_factors_exponent))
            {
                subspace_to_power_of_two(bump_factors,
                                         params->codeword_domain.basis, params->codeword_domain.basis_len,
                                         params->codeword_domain.shift, bump_factors_exponent, params->field_words);
            }
            else
            {
                for (size_t i = 0; i < 8 * sizeof(bump_factors_exponent); ++i)
                {
                    if (!(bump_factors_exponent & (1ull << i)))
                        continue;

                    uint64_t *subspace_to_two_to_i = (uint64_t *)malloc((1ull << params->codeword_domain.basis_len) * params->field_bytesize);
                    subspace_to_power_of_two(subspace_to_two_to_i,
                                             params->codeword_domain.basis, params->codeword_domain.basis_len,
                                             params->codeword_domain.shift, 1ull << i, params->field_words);
                    for (size_t j = 0; j < params->codeword_domain.size; ++j)
                    {
                        // bump_factors[j] *= subspace_to_two_to_i[j];
                        field_mulEqual(&bump_factors[j * params->field_words],
                                       &subspace_to_two_to_i[j * params->field_words],
                                       params->field_words);
                    }
                    free(subspace_to_two_to_i);
                }
            }

            // result[j] += (&data->_coefficients[submaximal_oracle_index] +
            //               &data->_coefficients[oracle->constituent_oracles_len + i] *
            //                   bump_factors[j]) *
            //              &oracle->constituent_oracles[submaximal_oracle_index]->operator[](j);
            for (size_t j = 0; j < oracle->content_len; ++j)
            {
                field_mul(tmp, &data->_coefficients[(oracle->constituent_oracles_len + i) * params->field_words], &bump_factors[j * params->field_words]);
                field_addEqual(tmp, &data->_coefficients[submaximal_oracle_index * params->field_words], params->field_words);
                field_mulEqual(tmp, &oracle->constituent_oracles[submaximal_oracle_index]->content[j * params->field_words], params->field_words);
                field_addEqual(&oracle->content[j * params->field_words], tmp, params->field_words);
            }
            free(bump_factors);
        }
    }
    free(tmp);
    free(temp);
    return 0;
}

static int combined_ldt_oracle_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{
    const CombinedLDTOracleData *data = (CombinedLDTOracleData *)oracle->data;
    if (constituent_evaluations_len != data->input_oracle_degrees_len)
    {
        fprintf(stderr, "Expected same number of evaluations as in registration.\n");
        return 1;
    }
    assert(data->_submaximal_oracle_indices);
    assert(data->_submaximal_oracle_indices_len);

    const size_t field_words = params->field_words;

    var_set_zero(result, params);
    uint64_t *tmp = new_tmp_var(params);
    for (size_t i = 0; i < constituent_evaluations_len; ++i)
    {
        field_mul(tmp, &data->_coefficients[field_words * i], &constituent_evaluations[field_words * i]);
        field_addEqual(result, tmp, field_words);
    }

    /* Now deal with submaximal oracles */
    uint64_t *bump_factor = new_tmp_var(params);
    for (size_t i = 0; i < data->_submaximal_oracle_indices_len; ++i)
    {
        /* Which oracle are we dealing with now? */
        const size_t submaximal_oracle_index = data->_submaximal_oracle_indices[i];
        /* Raise each element in the codeword domain to the power of the degree difference. */
        field_pow(bump_factor, evaluation_point, data->_max_degree - data->input_oracle_degrees[submaximal_oracle_index], field_words);

        field_mul(tmp, &data->_coefficients[field_words * (data->input_oracle_degrees_len + i)], bump_factor);
        field_mulEqual(tmp, &constituent_evaluations[field_words * submaximal_oracle_index], field_words);
        field_addEqual(result, tmp, field_words);
    }
    free(bump_factor);
    free(tmp);
    return 0;
}

static Oracle *new_oracle(Oracle *skeleton_oracle, size_t content_len, size_t content_deg, const Domain *domain, oracle_evaluation_at_point_fn evaluation_at_point_fn, void *data, size_t data_size, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(skeleton_oracle->constituent_oracles_len == constituent_oracles_len);
    for (size_t i = 0; i < constituent_oracles_len; ++i)
    {
        assert(constituent_oracles[i]);
        assert(skeleton_oracle->constituent_oracles[i]->_id == constituent_oracles[i]->_id);
    }
    assert(data);
    Oracle *oracle = (Oracle *)malloc(sizeof(Oracle));
    assert(oracle);
    memcpy(oracle, skeleton_oracle, sizeof(Oracle));

    oracle->content = NULL;
    oracle->content_len = content_len;
    oracle->content_deg = content_deg;
    oracle->domain = domain;
    oracle->constituent_oracles = (Oracle **)malloc(sizeof(Oracle *) * constituent_oracles_len);
    assert(oracle->constituent_oracles);
    memcpy(oracle->constituent_oracles, constituent_oracles, sizeof(Oracle *) * constituent_oracles_len);
    oracle->evaluation_at_point = evaluation_at_point_fn;
    oracle->data = malloc(data_size);
    assert(oracle->data);
    memcpy(oracle->data, data, data_size);
    return oracle;
}

static void free_oracle(Oracle *oracle)
{
    free(oracle->constituent_oracles);
    free(oracle->data);
    free(oracle->content);
    free(oracle);
}

inline __attribute__((always_inline)) Oracle *new_fz_oracle(FzOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->primary_input_len);
    assert(data->_primary_input == NULL);
    return new_oracle(&fz_oracle, content_len, content_deg, domain, fz_oracle_evaluation_at_point, data, sizeof(FzOracleData), constituent_oracles, constituent_oracles_len);
}

inline __attribute__((always_inline)) void free_fz_oracle(Oracle *oracle)
{
    free_oracle(oracle);
}

inline __attribute__((always_inline)) Oracle *new_rowcheck_oracle(RowcheckOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->params);
    assert(data->_Z == NULL);
    assert(data->_Z_len == 0);
    Oracle *oracle = new_oracle(&rowcheck_oracle, content_len, content_deg, domain, rowcheck_oracle_evaluation_at_point, data, sizeof(RowcheckOracleData), constituent_oracles, constituent_oracles_len);
    data = (RowcheckOracleData *)oracle->data;
    data->_Z_len = data->params->constraint_domain.size + 1;
    data->_Z = (uint64_t *)malloc(data->_Z_len * data->params->field_bytesize);
    vanishing_polynomial(data->_Z, data->_Z_len, &data->params->constraint_domain, data->params);
    return oracle;
}

inline __attribute__((always_inline)) void free_rowcheck_oracle(Oracle *oracle)
{
    free(((RowcheckOracleData *)oracle->data)->_Z);
    free_oracle(oracle);
}

inline __attribute__((always_inline)) Oracle *new_multi_lincheck_oracle(MultilinCheckOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->params);
    assert(data->r1cs);
    assert(data->_r_Mz == NULL);
    assert(data->_r_Mz_len == 0);
    assert(data->_p_alpha_ABC == NULL);
    assert(data->_p_alpha_ABC_len == 0);
    assert(data->_p_alpha_prime == NULL);
    assert(data->_p_alpha_prime_len == 0);
    return new_oracle(&multi_lincheck_oracle, content_len, content_deg, domain, multi_lincheck_oracle_evaluation_at_point, data, sizeof(MultilinCheckOracleData), constituent_oracles, constituent_oracles_len);
}

inline __attribute__((always_inline)) void free_multi_lincheck_oracle(Oracle *oracle)
{
    free(((MultilinCheckOracleData *)oracle->data)->_alpha);
    free(((MultilinCheckOracleData *)oracle->data)->_r_Mz);
    free(((MultilinCheckOracleData *)oracle->data)->_p_alpha_ABC);
    free(((MultilinCheckOracleData *)oracle->data)->_p_alpha_prime);
    free_oracle(oracle);
}

inline __attribute__((always_inline)) Oracle *new_combined_f_oracle(CombinedFOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->params);
    assert(data->_random_coefficients == NULL);
    assert(data->_random_coefficients_len == 0);
    return new_oracle(&combined_f_oracle, content_len, content_deg, domain, combined_f_oracle_evaluation_at_point, data, sizeof(CombinedFOracleData), constituent_oracles, constituent_oracles_len);
}

inline __attribute__((always_inline)) void free_combined_f_oracle(Oracle *oracle)
{
    free(((CombinedFOracleData *)oracle->data)->_random_coefficients);
    free_oracle(oracle);
}

inline __attribute__((always_inline)) Oracle *new_sumcheck_g_oracle(SumcheckGOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->params);
    assert(data->field_subset_type == AffineSubspaceType);
    assert(data->_Z == NULL);
    assert(data->_Z_len == 0);
    assert(data->_eps == NULL);
    assert(data->_eps_inv_times_claimed_sum == NULL);
    Oracle *oracle = new_oracle(&sumcheck_g_oracle, content_len, content_deg, domain, sumcheck_g_oracle_evaluation_at_point, data, sizeof(SumcheckGOracleData), constituent_oracles, constituent_oracles_len);
    data = (SumcheckGOracleData *)oracle->data;
    data->_Z_len = data->params->summation_domain.size + 1;
    data->_Z = (uint64_t *)malloc(data->_Z_len * data->params->field_bytesize);
    vanishing_polynomial(data->_Z, data->_Z_len, &data->params->summation_domain, data->params);
    data->_eps = &data->_Z[1 * data->params->field_words];
    return oracle;
}

inline __attribute__((always_inline)) void free_sumcheck_g_oracle(Oracle *oracle)
{
    free(((SumcheckGOracleData *)oracle->data)->_eps_inv_times_claimed_sum);
    free(((SumcheckGOracleData *)oracle->data)->_Z);
    free_oracle(oracle);
}

inline __attribute__((always_inline)) Oracle *new_combined_ldt_oracle(CombinedLDTOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len)
{
    assert(data);
    assert(data->params);
    assert(data->input_oracle_degrees);
    assert(data->input_oracle_degrees_len);
    assert(data->_num_random_coefficients == 0);
    assert(data->_coefficients == NULL);
    assert(data->_coefficients_len == 0);
    assert(data->_max_degree == 0);
    assert(data->_submaximal_oracle_indices == NULL);
    assert(data->_submaximal_oracle_indices_len == 0);
    Oracle *oracle = new_oracle(&combined_ldt_oracle, content_len, content_deg, domain, combined_ldt_oracle_evaluation_at_point, data, sizeof(CombinedLDTOracleData), constituent_oracles, constituent_oracles_len);
    data = (CombinedLDTOracleData *)oracle->data;
    data->_num_random_coefficients = data->input_oracle_degrees_len * 2;
    for (size_t i = 0; i < data->input_oracle_degrees_len; ++i)
    {
        if (data->_max_degree < data->input_oracle_degrees[i])
        {
            data->_max_degree = data->input_oracle_degrees[i];
        }
    }
    oracle->content_deg = data->_max_degree;
    for (size_t i = 0; i < data->input_oracle_degrees_len; ++i)
    {
        if (data->input_oracle_degrees[i] < data->_max_degree)
        {
            data->_submaximal_oracle_indices_len++;
        }
    }
    data->_submaximal_oracle_indices = (size_t *)malloc(sizeof(size_t) * data->_submaximal_oracle_indices_len);
    size_t j = 0;
    for (size_t i = 0; i < data->input_oracle_degrees_len; ++i)
    {
        if (data->input_oracle_degrees[i] < data->_max_degree)
        {
            data->_submaximal_oracle_indices[j++] = i;
        }
    }
    assert(j == data->_submaximal_oracle_indices_len);
    return oracle;
}

inline __attribute__((always_inline)) void free_combined_ldt_oracle(Oracle *oracle)
{
    free(((CombinedLDTOracleData *)oracle->data)->_coefficients);
    free(((CombinedLDTOracleData *)oracle->data)->_submaximal_oracle_indices);
    free_oracle(oracle);
}