#ifndef _AURORA_H__
#define _AURORA_H__

#include <stddef.h>
#include <stdint.h>

#include "aurora_inner.h"
#include "params.h"
#include "r1cs.h"

// Proving system

size_t aurora_prover(uint8_t *sig, size_t sig_len, int sig_type, const uint8_t *msg, size_t msg_len,
                     const R1CS *r1cs, const uint64_t *input, size_t primary_input_count, size_t auxiliary_input_count);
int aurora_verifier(const uint8_t *sig, size_t sig_len, int sig_type, const uint8_t *msg, size_t msg_len,
                    const R1CS *r1cs, const uint64_t *input, size_t input_len);

#endif
