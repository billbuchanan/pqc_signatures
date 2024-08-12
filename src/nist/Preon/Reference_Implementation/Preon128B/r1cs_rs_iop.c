#include "r1cs_rs_iop.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aurora_inner.h"
#include "field.h"
#include "fft.h"
#include "params.h"
#include "poly.h"
#include "r1cs.h"
#include "rand.h"

static void calculate_and_submit_fw(uint64_t *f1v_coeff, const uint64_t *input, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params)
{
    // compute fw over codeword domain
    uint64_t *f1v = (uint64_t *)malloc(params->input_variable_domain.size * params->field_bytesize);
    memcpy(f1v, params->field_one, params->field_bytesize);
    memcpy(&f1v[params->field_words], input, primary_input_count * params->field_bytesize);
    // IFFT over input_variable_domain is actually an subset of variable_domain with order primary_input_count + 1
    // Basis may be different?
    // uint64_t *f1v_coeff = (uint64_t *)malloc(params->input_variable_domain.size * params->field_bytesize);
    ifft(f1v_coeff, f1v, params->field_words, params->input_variable_domain.size, params->field_zero);
    uint64_t *f1v_over_variable_domain = (uint64_t *)malloc(params->variable_domain.size * params->field_bytesize);
    fft(f1v_over_variable_domain, f1v_coeff, params->input_variable_domain.size, params->field_words, params->variable_domain.size, params->field_zero);

    // create_fw_prime_evals
    uint64_t *fw_prime_over_variable_domain = (uint64_t *)malloc(params->variable_domain.size * params->field_bytesize);
    memset(fw_prime_over_variable_domain, 0, params->variable_domain.size * params->field_bytesize);
    for (size_t i = 0; i < auxiliary_input_count; i++)
    {
        size_t variable_index = i + params->input_variable_domain.size;
        field_sub(&fw_prime_over_variable_domain[variable_index * params->field_words],
                  &input[(primary_input_count + i) * params->field_words],
                  &f1v_over_variable_domain[variable_index * params->field_words], params->field_words);
    }

    uint64_t *fw_prime = (uint64_t *)malloc(params->variable_domain.size * params->field_bytesize);
    ifft(fw_prime, fw_prime_over_variable_domain, params->field_words, params->variable_domain.size, params->field_zero);

    size_t var_vp_len = vanishing_polynomial_len(&params->variable_domain);
    uint64_t *var_vp = (uint64_t *)malloc(var_vp_len * params->field_bytesize);
    vanishing_polynomial(var_vp, var_vp_len, &params->variable_domain, params);
    uint64_t *fw_mask = (uint64_t *)malloc(params->query_bound * params->field_bytesize);
    random_bytes(fw_mask, params->query_bound * params->field_bytesize);

    // var_vp_times_fw_mask = var_vp * fw_mask
    // deg(var_vp_times_fw_mask) = deg(var_vp) + deg(fw_mask)
    // len(var_vp_times_fw_mask) = deg(var_vp_times_fw_mask) + 1 = deg(var_vp) + deg(fw_mask) + 1
    // = (deg(var_vp) + 1) + (deg(fw_mask) + 1) - 1 = var_vp_size + fw_mask_size(params->query_bound) - 1
    const size_t full_fw_prime_len = var_vp_len + params->query_bound - 1;
    uint64_t *full_fw_prime = (uint64_t *)malloc(full_fw_prime_len * params->field_bytesize);
    poly_mul(full_fw_prime, var_vp, var_vp_len, fw_mask, params->query_bound, params->field_words);
    poly_addEqual(full_fw_prime, full_fw_prime_len, fw_prime, params->variable_domain.size, params->field_words);

    size_t input_vp_len = vanishing_polynomial_len(&params->input_variable_domain);
    uint64_t *input_vp = (uint64_t *)malloc(input_vp_len * params->field_bytesize);
    vanishing_polynomial(input_vp, input_vp_len, &params->input_variable_domain, params);
    const size_t fw_prime_over_input_vp_remainder_len = input_vp_len - 1;
    const size_t fw_prime_over_input_vp_quotient_len = full_fw_prime_len - fw_prime_over_input_vp_remainder_len;
    uint64_t *fw_prime_over_input_vp_quotient = (uint64_t *)malloc(fw_prime_over_input_vp_quotient_len * params->field_bytesize);
    uint64_t *fw_prime_over_input_vp_remainder = (uint64_t *)malloc(fw_prime_over_input_vp_remainder_len * params->field_bytesize);
    poly_div(fw_prime_over_input_vp_quotient, fw_prime_over_input_vp_remainder,
             full_fw_prime, full_fw_prime_len,
             input_vp, input_vp_len,
             params->field_words);
    uint64_t *fw_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(fw_over_codeword_domain, fw_prime_over_input_vp_quotient, fw_prime_over_input_vp_quotient_len,
        params->field_words, params->codeword_domain.size, params->codeword_domain.shift);
    fw_oracle.content = fw_over_codeword_domain;
    fw_oracle.content_len = params->codeword_domain.size;

    free(f1v);
    free(f1v_over_variable_domain);
    free(fw_prime_over_variable_domain);
    free(fw_prime);
    free(var_vp);
    free(input_vp);
    free(fw_mask);
    free(full_fw_prime);
    free(fw_prime_over_input_vp_quotient);
    free(fw_prime_over_input_vp_remainder);
}

static void calculate_and_submit_fprime_ABCz(const R1CS *r1cs, const uint64_t *input, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params)
{
    // Az, Bz, Cz related computations
    uint64_t *Az, *Bz, *Cz;

    // compute_ABCz
    Az = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    Bz = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    Cz = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    for (size_t i = 0; i < params->constraint_domain.size; ++i)
    {
        memcpy(&Az[i * params->field_words], &r1cs->A[i][0 * params->field_words], params->field_bytesize);
        memcpy(&Bz[i * params->field_words], &r1cs->B[i][0 * params->field_words], params->field_bytesize);
        memcpy(&Cz[i * params->field_words], &r1cs->C[i][0 * params->field_words], params->field_bytesize);
        for (size_t j = 1; j < params->variable_domain.size; ++j)
        {
            uint64_t *tmp = (uint64_t *)malloc(params->field_bytesize);
            // ABCz[i * params->field_words] += ((uint64_t *)input)[(j - 1) * params->field_words] * ABC[i][j * params->field_words];
            if (!is_zero(&r1cs->A[i][j * params->field_words], params->field_words))
            {
                field_mul(tmp, &r1cs->A[i][j * params->field_words], &input[(j - 1) * params->field_words]);
                field_addEqual(&Az[i * params->field_words], tmp, params->field_words);
            }
            if (!is_zero(&r1cs->B[i][j * params->field_words], params->field_words))
            {
                field_mul(tmp, &r1cs->B[i][j * params->field_words], &input[(j - 1) * params->field_words]);
                field_addEqual(&Bz[i * params->field_words], tmp, params->field_words);
            }
            if (!is_zero(&r1cs->C[i][j * params->field_words], params->field_words))
            {
                field_mul(tmp, &r1cs->C[i][j * params->field_words], &input[(j - 1) * params->field_words]);
                field_addEqual(&Cz[i * params->field_words], tmp, params->field_words);
            }
            free(tmp);
        }
    }

    // compute_fprime_ABCz_over_codeword_domain
    uint64_t *fAz = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    ifft(fAz, Az, params->field_words, params->constraint_domain.size, params->field_zero);
    uint64_t *fBz = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    ifft(fBz, Bz, params->field_words, params->constraint_domain.size, params->field_zero);
    uint64_t *fCz = (uint64_t *)malloc(params->constraint_domain.size * params->field_bytesize);
    ifft(fCz, Cz, params->field_words, params->constraint_domain.size, params->field_zero);
    free(Az);
    free(Bz);
    free(Cz);

    uint64_t *random_A = (uint64_t *)malloc(params->query_bound * params->field_bytesize);
    uint64_t *random_B = (uint64_t *)malloc(params->query_bound * params->field_bytesize);
    uint64_t *random_C = (uint64_t *)malloc(params->query_bound * params->field_bytesize);
    random_bytes(random_A, params->query_bound * params->field_bytesize);
    random_bytes(random_B, params->query_bound * params->field_bytesize);
    random_bytes(random_C, params->query_bound * params->field_bytesize);

    size_t constraint_vp_len = vanishing_polynomial_len(&params->constraint_domain);
    uint64_t *constraint_vp = (uint64_t *)malloc(constraint_vp_len * params->field_bytesize);
    vanishing_polynomial(constraint_vp, constraint_vp_len, &params->constraint_domain, params);
    size_t full_ABCz_len = params->query_bound + constraint_vp_len - 1;
    uint64_t *full_fAz = (uint64_t *)malloc(full_ABCz_len * params->field_bytesize);
    memset(full_fAz, 0, full_ABCz_len * params->field_bytesize);
    poly_mul(full_fAz, random_A, params->query_bound, constraint_vp, constraint_vp_len, params->field_words);
    poly_addEqual(full_fAz, full_ABCz_len, fAz, params->constraint_domain.size, params->field_words);
    uint64_t *full_fBz = (uint64_t *)malloc(full_ABCz_len * params->field_bytesize);
    memset(full_fBz, 0, full_ABCz_len * params->field_bytesize);
    poly_mul(full_fBz, random_B, params->query_bound, constraint_vp, constraint_vp_len, params->field_words);
    poly_addEqual(full_fBz, full_ABCz_len, fBz, params->constraint_domain.size, params->field_words);
    uint64_t *full_fCz = (uint64_t *)malloc(full_ABCz_len * params->field_bytesize);
    memset(full_fCz, 0, full_ABCz_len * params->field_bytesize);
    poly_mul(full_fCz, random_C, params->query_bound, constraint_vp, constraint_vp_len, params->field_words);
    poly_addEqual(full_fCz, full_ABCz_len, fCz, params->constraint_domain.size, params->field_words);

    free(constraint_vp);
    free(random_A);
    free(random_B);
    free(random_C);
    free(fAz);
    free(fBz);
    free(fCz);

    uint64_t *fprime_Az_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(fprime_Az_over_codeword_domain, full_fAz, full_ABCz_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);
    uint64_t *fprime_Bz_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(fprime_Bz_over_codeword_domain, full_fBz, full_ABCz_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);
    uint64_t *fprime_Cz_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    fft(fprime_Cz_over_codeword_domain, full_fCz, full_ABCz_len, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);

    fAz_oracle.content = fprime_Az_over_codeword_domain;
    fAz_oracle.content_len = params->codeword_domain.size;
    fBz_oracle.content = fprime_Bz_over_codeword_domain;
    fBz_oracle.content_len = params->codeword_domain.size;
    fCz_oracle.content = fprime_Cz_over_codeword_domain;
    fCz_oracle.content_len = params->codeword_domain.size;

    free(full_fAz);
    free(full_fBz);
    free(full_fCz);
}

void r1cs_rs_iop(uint64_t *f1v_coeff, const R1CS *r1cs, const uint64_t *input, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params)
{
    calculate_and_submit_fw(f1v_coeff, input, primary_input_count, auxiliary_input_count, params);
    calculate_and_submit_fprime_ABCz(r1cs, input, primary_input_count, auxiliary_input_count, params);
}
