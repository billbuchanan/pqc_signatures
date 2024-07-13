#ifndef _R1CS_H__
#define _R1CS_H__

#include <stddef.h>
#include <stdint.h>

#include "aes.h"
#include "params.h"

typedef struct
{
    uint64_t **A;
    uint64_t **B;
    uint64_t **C;
    size_t public_input_count;
    size_t private_input_count;
    size_t constraint_count;
} R1CS;

R1CS *r1cs_init(const Parameters *params, int init_empty);
void r1cs_free(R1CS *);

void set_dummy_constraint_system(R1CS *r1cs, const Parameters *params);

void r1cs_get_g(uint64_t *result, int idx, const Parameters *params);
void r1cs_get_h(uint64_t *result, int idx, const Parameters *params);
uint64_t *new_aes_key_expansion(size_t aes_bits, const Parameters *params);
void convert_key_expansion_to_field(uint64_t *result, uint8_t *ke, size_t aes_bits, const Parameters *params);

uint64_t *new_r1cs_instance(const uint8_t *pk, const Parameters *params);
uint64_t *new_r1cs_witness(const uint8_t *aes_key, size_t aes_key_len,
                           const uint8_t *pk, const struct AES_ctx *aes_ctx0,
                           const struct AES_ctx *aes_ctx1,
                           const Parameters *params);
void free_r1cs_instance(uint64_t *instance);
void free_r1cs_witness(uint64_t *witness);
#endif