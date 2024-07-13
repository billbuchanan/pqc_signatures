#ifndef _SUMCHECK_H__
#define _SUMCHECK_H__

#include "oracles.h"
#include "params.h"

void submit_sumcheck_masking_poly(const Parameters *params);
void sumcheck_calculate_and_submit_proof(Oracle *combined_f_virtual_oracle, const Parameters *params);

#endif