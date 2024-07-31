#ifndef _R1CS_RS_IOP_H__
#define _R1CS_RS_IOP_H__

#include <stddef.h>
#include <stdint.h>

#include "oracles.h"
#include "params.h"
#include "r1cs.h"

void r1cs_rs_iop(uint64_t *f1v_coeff, const R1CS *r1cs, const uint64_t *input, const size_t primary_input_count, const size_t auxiliary_input_count, const Parameters *params);

#endif
