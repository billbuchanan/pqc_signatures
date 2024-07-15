#include "sumcheck.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "aurora_inner.h"
#include "oracles.h"
#include "params.h"
#include "fft.h"
#include "poly.h"
#include "rand.h"
#include "virtual_oracles.h"

void submit_sumcheck_masking_poly(const Parameters *params)
{
    /** The sum of any polynomial m, over all of H is:
     *  Σ_{a in H} m(a) = Σ_{a in H} g(a) + Z_H * h(a) = Σ_{a in H} g(a), where deg(g) < |H|
     *  We seek to sample a random polynomial of degree d, which sums to 0 over H.
     *  We do this as follows:
     *  1) sample a random polynomial g of deg |H| - 1, and h of degree (d - |H|)
     *  2) alter g such that its sum over H is 0
     *  3) compute m using the identity m = Z_H * h + g
     *  4) convert m to the codeword domain and submit it */
    const size_t masking_h_poly_len = params->summation_domain.size + params->query_bound - 1;
    const size_t degree_bound = masking_h_poly_len + params->summation_domain.size;

    uint64_t *sumcheck_masking_poly_over_codeword_domain = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    uint64_t *sumcheck_masking_poly = (uint64_t *)malloc(degree_bound * params->field_bytesize);

    uint64_t *masking_g_poly = (uint64_t *)malloc(params->summation_domain.size * params->field_bytesize);
    random_bytes(masking_g_poly, params->summation_domain.size * params->field_bytesize);
    uint64_t *masking_h_poly = (uint64_t *)malloc(masking_h_poly_len * params->field_bytesize);
    random_bytes(masking_h_poly, masking_h_poly_len * params->field_bytesize);

    size_t summation_vp_len = vanishing_polynomial_len(&params->summation_domain);
    uint64_t *summation_vp = (uint64_t *)malloc(summation_vp_len * params->field_bytesize);
    vanishing_polynomial(summation_vp, summation_vp_len, &params->summation_domain, params);

    /** When H is an additive subspace, Σ_{a in H} g(a) = beta * Σ_{a in H} a^{|H| - 1},
     *  where beta is the term of g of degree (|H| - 1).
     *  Σ_{a in H} a^{|H| - 1} is equal to the linear term of Z_H.
     *  See section 5 of Aurora for more detail of the above.
     *  Consequently, it sums to 0 if and only if the coefficient of a^{|H| - 1} is 0 */
    // sumcheck_masking_poly = (summation_vp * masking_h_poly) + masking_g_poly;
    memcpy(&masking_g_poly[(params->summation_domain.size - 1) * params->field_words], params->field_zero, params->field_bytesize);
    // len(sumcheck_masking_poly) = degree_bound + 1 = masking_h_poly_len + params->summation_domain.size
    // len(summation_vp * masking_h_poly) = len(summation_vp) + len(masking_h_poly) - 1
    //     = params->summation_domain.size + 1 + masking_h_poly_len - 1
    //     = params->summation_domain.size + masking_h_poly_len = len(sumcheck_masking_poly)
    poly_mul(sumcheck_masking_poly, summation_vp, summation_vp_len, masking_h_poly, masking_h_poly_len, params->field_words);
    poly_addEqual(sumcheck_masking_poly, degree_bound, masking_g_poly, params->summation_domain.size, params->field_words);
    fft(sumcheck_masking_poly_over_codeword_domain, sumcheck_masking_poly, degree_bound, params->field_words, params->codeword_domain.size, params->codeword_domain.shift);
    free(sumcheck_masking_poly);
    free(masking_g_poly);
    free(masking_h_poly);
    free(summation_vp);

    sumcheck_masking_poly_oracle.content = sumcheck_masking_poly_over_codeword_domain;
    sumcheck_masking_poly_oracle.content_len = params->codeword_domain.size;
}

void sumcheck_calculate_and_submit_proof(Oracle *combined_f_virtual_oracle, const Parameters *params)
{
    // sumchecks_calculate_and_submit_proof();
    const size_t degree_bound = 2 * params->summation_domain.size + params->query_bound - 1;

    uint64_t *sumcheck_challenges = (uint64_t *)malloc(2 * params->field_bytesize);
    memcpy(&sumcheck_challenges[0 * params->field_words], params->field_one, params->field_bytesize);
    memcpy(&sumcheck_challenges[1 * params->field_words], params->field_one, params->field_bytesize);

    assert(combined_f_oracle_set_random_coefficients(combined_f_virtual_oracle, sumcheck_challenges, 2) == 0);
    combined_f_oracle_evaluate_content(combined_f_virtual_oracle, params);

    const size_t minimal_subspace_size = 1ull << (size_t)ceil(log2(degree_bound));
    uint64_t *combined_f_oracle_evaluations = (uint64_t *)malloc(minimal_subspace_size * params->field_bytesize);
    memcpy(combined_f_oracle_evaluations, combined_f_virtual_oracle->content, minimal_subspace_size * params->field_bytesize);
    uint64_t *combined_f_oracle_polynomial = (uint64_t *)malloc(minimal_subspace_size * params->field_bytesize);
    memset(combined_f_oracle_polynomial, 0, minimal_subspace_size * params->field_bytesize);
    ifft(combined_f_oracle_polynomial,
         combined_f_oracle_evaluations,
         params->field_words, minimal_subspace_size,
         params->codeword_domain.shift);
    // combined_f_oracle_polynomial.resize(degree_bound);
    // TODO: This is required for division later, bit resize cause the coeff with larger power 0, is this intended?

    // claimed_sum is always zero!
    // is this correct?
    // g_oracle_set_claimed_sum(0);
    size_t summation_vp_len = vanishing_polynomial_len(&params->summation_domain);
    uint64_t *summation_vp = (uint64_t *)malloc(summation_vp_len * params->field_bytesize);
    vanishing_polynomial(summation_vp, summation_vp_len, &params->summation_domain, params);
    /* Calculate g, h and actual sum from f */
    size_t quotient_len = degree_bound - params->summation_domain.size;
    uint64_t *combined_f_oracle_polynomial_over_summation_vp_quotient = (uint64_t *)malloc(quotient_len * params->field_bytesize);
    uint64_t *combined_f_oracle_polynomial_over_summation_vp_remainder = (uint64_t *)malloc((summation_vp_len - 1) * params->field_bytesize);
    poly_div(combined_f_oracle_polynomial_over_summation_vp_quotient,
             combined_f_oracle_polynomial_over_summation_vp_remainder,
             combined_f_oracle_polynomial, degree_bound,
             summation_vp, summation_vp_len,
             params->field_words);

    // The prover does not submit g, as it is computed using f, h, and the claimed sum
    sumcheck_h_oracle.content_len = params->codeword_domain.size;
    sumcheck_h_oracle.content = (uint64_t *)malloc(params->codeword_domain.size * params->field_bytesize);
    // It submits h as 6th oracle
    fft(sumcheck_h_oracle.content, combined_f_oracle_polynomial_over_summation_vp_quotient, quotient_len,
        params->field_words, params->codeword_domain.size, params->codeword_domain.shift);

    free(sumcheck_challenges);
    free(summation_vp);
    free(combined_f_oracle_evaluations);
    free(combined_f_oracle_polynomial);
    free(combined_f_oracle_polynomial_over_summation_vp_quotient);
    free(combined_f_oracle_polynomial_over_summation_vp_remainder);
}