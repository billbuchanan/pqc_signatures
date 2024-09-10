#include "lincheck.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "aurora_inner.h"
#include "oracles.h"
#include "params.h"
#include "virtual_oracles.h"

void multi_lincheck_calculate_and_submit_proof(Oracle *multi_lincheck_virtual_oracle, const uint8_t *hash_state, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params)
{
    size_t r_Mz_count = 3;
    uint64_t *alpha_and_r_Mz = (uint64_t *)malloc((r_Mz_count + 1) * params->field_bytesize);
    for (size_t i = 0; i < params->multilicheck_repetitions; i++)
    {
        squeeze_field_elements(alpha_and_r_Mz, r_Mz_count + 1, 1, hash_state, params);
        multi_lincheck_oracle_set_challenge(multi_lincheck_virtual_oracle, alpha_and_r_Mz, &alpha_and_r_Mz[1 * params->field_words], r_Mz_count);
        multi_lincheck_evaluate_content(multi_lincheck_virtual_oracle, params);
    }
    free(alpha_and_r_Mz);
}