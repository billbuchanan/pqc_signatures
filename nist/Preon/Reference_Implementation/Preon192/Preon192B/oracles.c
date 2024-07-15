#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "oracles.h"

Oracle fw_oracle = {._id = 1, .content = NULL, .is_virtual = 0};

Oracle fAz_oracle = {._id = 2, .content = NULL, .is_virtual = 0};

Oracle fBz_oracle = {._id = 3, .content = NULL, .is_virtual = 0};

Oracle fCz_oracle = {._id = 4, .content = NULL, .is_virtual = 0};

Oracle sumcheck_masking_poly_oracle = {._id = 5, .content = NULL, .is_virtual = 0};

Oracle ldt_blinding_vector_oracle = {._id = 6, .content = NULL, .is_virtual = 0};

Oracle sumcheck_h_oracle = {._id = 7, .content = NULL, .is_virtual = 0};

static int default_evaluation_at_point(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len)
{
    fprintf(stderr, "virtual oracle variable should not be used directy, use new_XXX_oracle to create dedicated object instead.\n");
    return 1;
}

Oracle fz_oracle = {
    ._id = 101,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};

static void init_oracle(Oracle *oracle, size_t constituent_oracles_len, ...)
{
    va_list constituent_oracles;
    va_start(constituent_oracles, constituent_oracles_len);

    oracle->constituent_oracles_len = constituent_oracles_len;

    oracle->constituent_oracles = (Oracle **)malloc(constituent_oracles_len * sizeof(Oracle *));
    for (size_t i = 0; i < constituent_oracles_len; ++i)
    {
        oracle->constituent_oracles[i] = va_arg(constituent_oracles, Oracle *);
    }
    va_end(constituent_oracles);
}

void init_oracles(const Parameters *params)
{
    init_oracle(&fz_oracle, 1, &fw_oracle);
    init_oracle(&rowcheck_oracle, 3, &fAz_oracle, &fBz_oracle, &fCz_oracle);
    init_oracle(&multi_lincheck_oracle, 4, &fz_oracle, &fAz_oracle, &fBz_oracle, &fCz_oracle);
    init_oracle(&combined_f_oracle, 2, &sumcheck_masking_poly_oracle, &multi_lincheck_oracle);
    init_oracle(&sumcheck_g_oracle, 2, &combined_f_oracle, &sumcheck_h_oracle);
    init_oracle(&combined_ldt_oracle, 9, &sumcheck_masking_poly_oracle, &sumcheck_h_oracle, &sumcheck_g_oracle, &fw_oracle, &fAz_oracle, &fBz_oracle, &fCz_oracle, &rowcheck_oracle, &ldt_blinding_vector_oracle);

    // init based on parameter
    fw_oracle.content_deg = params->variable_domain.size - params->input_variable_domain.size + params->query_bound;
    fw_oracle.domain = &params->codeword_domain;
    fAz_oracle.content_deg = params->constraint_domain.size + params->query_bound;
    fAz_oracle.domain = &params->codeword_domain;
    fBz_oracle.content_deg = params->constraint_domain.size + params->query_bound;
    fBz_oracle.domain = &params->codeword_domain;
    fCz_oracle.content_deg = params->constraint_domain.size + params->query_bound;
    fCz_oracle.domain = &params->codeword_domain;
    const size_t fz_degree = params->variable_domain.size + params->query_bound;
    const size_t Mz_degree = params->constraint_domain.size + params->query_bound;

    if (fz_degree > Mz_degree)
    {
        sumcheck_h_oracle.content_deg = fz_degree - 1;
        sumcheck_masking_poly_oracle.content_deg = fz_degree + params->summation_domain.size - 1;
    }
    else
    {
        sumcheck_h_oracle.content_deg = Mz_degree - 1;
        sumcheck_masking_poly_oracle.content_deg = Mz_degree + params->summation_domain.size - 1;
    }
    sumcheck_h_oracle.domain = &params->codeword_domain;
    sumcheck_masking_poly_oracle.domain = &params->codeword_domain;

    ldt_blinding_vector_oracle.content_deg = params->max_ldt_tested_degree_bound;
    ldt_blinding_vector_oracle.domain = &params->codeword_domain;
}

void deinit_oracles()
{
    free(fz_oracle.constituent_oracles);
    free(rowcheck_oracle.constituent_oracles);
    free(multi_lincheck_oracle.constituent_oracles);
    free(combined_f_oracle.constituent_oracles);
    free(sumcheck_g_oracle.constituent_oracles);
    free(combined_ldt_oracle.constituent_oracles);
}

Oracle rowcheck_oracle = {
    ._id = 102,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};

Oracle multi_lincheck_oracle = {
    ._id = 103,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};

Oracle combined_f_oracle = {
    ._id = 104,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};

Oracle sumcheck_g_oracle = {
    ._id = 105,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};

Oracle combined_ldt_oracle = {
    ._id = 106,
    .content = NULL,
    .is_virtual = 1,
    .evaluation_at_point = default_evaluation_at_point,
    .data = NULL};
