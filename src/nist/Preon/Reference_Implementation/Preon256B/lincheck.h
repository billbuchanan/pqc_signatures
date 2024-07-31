#ifndef _MULTI_LINCHECK_H__
#define _MULTI_LINCHECK_H__

#include <stddef.h>
#include <stdint.h>

#include "oracles.h"
#include "params.h"
#include "r1cs.h"

void multi_lincheck_calculate_and_submit_proof(Oracle *multi_lincheck_virtual_oracle, const uint8_t *hash_state, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params);

#endif