#ifndef _VIRTUAL_ORACLES_H__
#define _VIRTUAL_ORACLES_H__

#include "domain.h"
#include "oracles.h"
#include "r1cs.h"

Oracle **init_virtual_oracles(const R1CS *r1cs, const Parameters *params);
void deinit_virtual_oracles(Oracle **);

uint64_t *new_oracle_evaulation_result(const Parameters *params);
void free_oracle_evaulation_result(uint64_t *);

typedef struct
{
    size_t primary_input_len;

    const uint64_t *_primary_input;
} FzOracleData;

Oracle *new_fz_oracle(FzOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int fz_oracle_set_primary_input(Oracle *oracle, const uint64_t *primary_input, size_t primary_input_len);
int fz_oracle_evaluate_content(Oracle *oracle, const uint64_t *f1v_coeff, const size_t f1v_coeff_len, const Parameters *params);
void free_fz_oracle(Oracle *oracle);

typedef struct
{
    const Parameters *params;
    uint64_t *_Z;
    size_t _Z_len;
} RowcheckOracleData;

Oracle *new_rowcheck_oracle(RowcheckOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int rowcheck_oracle_evaluate_content(Oracle *oracle, const Parameters *params);
void free_rowcheck_oracle(Oracle *oracle);

typedef struct
{
    const Parameters *params;
    const R1CS *r1cs;
    uint64_t *_alpha;
    uint64_t *_r_Mz;
    size_t _r_Mz_len;
    uint64_t *_p_alpha_ABC;
    size_t _p_alpha_ABC_len;
    uint64_t *_p_alpha_prime;
    size_t _p_alpha_prime_len;
} MultilinCheckOracleData;

Oracle *new_multi_lincheck_oracle(MultilinCheckOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int multi_lincheck_oracle_set_challenge(Oracle *oracle, const uint64_t *alpha, const uint64_t *r_Mz, size_t r_Mz_len);
int multi_lincheck_evaluate_content(Oracle *oracle, const Parameters *params);
void free_multi_lincheck_oracle(Oracle *oracle);

typedef struct
{
    const Parameters *params;
    uint64_t *_random_coefficients;
    size_t _random_coefficients_len;
} CombinedFOracleData;

Oracle *new_combined_f_oracle(CombinedFOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int combined_f_oracle_set_random_coefficients(Oracle *oracle, const uint64_t *random_coefficients, size_t random_coefficients_len);
int combined_f_oracle_evaluate_content(Oracle *oracle, const Parameters *params);
void free_combined_f_oracle(Oracle *oracle);

typedef enum
{
    AffineSubspaceType,
    MultiplicativeCosetType,
} FieldSubsetType;

typedef struct
{
    const Parameters *params;
    FieldSubsetType field_subset_type;
    uint64_t *_Z;
    size_t _Z_len;
    uint64_t *_eps;
    uint64_t *_eps_inv_times_claimed_sum;
} SumcheckGOracleData;

Oracle *new_sumcheck_g_oracle(SumcheckGOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int sumcheck_g_oracle_set_claimed_sum(Oracle *oracle, const uint64_t *claimed_sum);
int sumcheck_g_oracle_evaluate_content(Oracle *oracle, const Parameters *params);
void free_sumcheck_g_oracle(Oracle *oracle);

typedef struct
{
    const size_t *input_oracle_degrees;
    size_t input_oracle_degrees_len;
    const Parameters *params;

    size_t _num_random_coefficients;
    uint64_t *_coefficients;
    size_t _coefficients_len;
    size_t _max_degree;
    size_t *_submaximal_oracle_indices;
    size_t _submaximal_oracle_indices_len;
} CombinedLDTOracleData;

Oracle *new_combined_ldt_oracle(CombinedLDTOracleData *data, size_t content_len, size_t content_deg, const Domain *domain, Oracle **constituent_oracles, size_t constituent_oracles_len);
int combined_ldt_oracle_set_random_coefficients(Oracle *oracle, const uint64_t *random_coefficients, size_t random_coefficients_len);
int combined_ldt_oracle_evaluate_content(Oracle *oracle, const Parameters *params);
void free_combined_ldt_oracle(Oracle *oracle);

#endif