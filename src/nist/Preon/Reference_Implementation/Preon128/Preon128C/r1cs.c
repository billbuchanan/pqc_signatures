#include "r1cs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "aes.h"
#include "r1cs_ABC.h"

static const uint8_t g_table[] = {0x1, 0x8d, 0xf6, 0xcb, 0x52, 0x7b, 0xd1, 0xe8, 0x4f, 0x29, 0xc0, 0xb0, 0xe1, 0xe5, 0xc7, 0x74, 0xb4, 0xaa, 0x4b, 0x99, 0x2b, 0x60, 0x5f, 0x58, 0x3f, 0xfd, 0xcc, 0xff, 0x40, 0xee, 0xb2, 0x3a, 0x6e, 0x5a, 0xf1, 0x55, 0x4d, 0xa8, 0xc9, 0xc1, 0xa, 0x98, 0x15, 0x30, 0x44, 0xa2, 0xc2, 0x2c, 0x45, 0x92, 0x6c, 0xf3, 0x39, 0x66, 0x42, 0xf2, 0x35, 0x20, 0x6f, 0x77, 0xbb, 0x59, 0x19, 0x1d, 0xfe, 0x37, 0x67, 0x2d, 0x31, 0xf5, 0x69, 0xa7, 0x64, 0xab, 0x13, 0x54, 0x25, 0xe9, 0x9, 0xed, 0x5c, 0x5, 0xca, 0x4c, 0x24, 0x87, 0xbf, 0x18, 0x3e, 0x22, 0xf0, 0x51, 0xec, 0x61, 0x17, 0x16, 0x5e, 0xaf, 0xd3, 0x49, 0xa6, 0x36, 0x43, 0xf4, 0x47, 0x91, 0xdf, 0x33, 0x93, 0x21, 0x3b, 0x79, 0xb7, 0x97, 0x85, 0x10, 0xb5, 0xba, 0x3c, 0xb6, 0x70, 0xd0, 0x6, 0xa1, 0xfa, 0x81, 0x82, 0x83, 0x7e, 0x7f, 0x80, 0x96, 0x73, 0xbe, 0x56, 0x9b, 0x9e, 0x95, 0xd9, 0xf7, 0x2, 0xb9, 0xa4, 0xde, 0x6a, 0x32, 0x6d, 0xd8, 0x8a, 0x84, 0x72, 0x2a, 0x14, 0x9f, 0x88, 0xf9, 0xdc, 0x89, 0x9a, 0xfb, 0x7c, 0x2e, 0xc3, 0x8f, 0xb8, 0x65, 0x48, 0x26, 0xc8, 0x12, 0x4a, 0xce, 0xe7, 0xd2, 0x62, 0xc, 0xe0, 0x1f, 0xef, 0x11, 0x75, 0x78, 0x71, 0xa5, 0x8e, 0x76, 0x3d, 0xbd, 0xbc, 0x86, 0x57, 0xb, 0x28, 0x2f, 0xa3, 0xda, 0xd4, 0xe4, 0xf, 0xa9, 0x27, 0x53, 0x4, 0x1b, 0xfc, 0xac, 0xe6, 0x7a, 0x7, 0xae, 0x63, 0xc5, 0xdb, 0xe2, 0xea, 0x94, 0x8b, 0xc4, 0xd5, 0x9d, 0xf8, 0x90, 0x6b, 0xb1, 0xd, 0xd6, 0xeb, 0xc6, 0xe, 0xcf, 0xad, 0x8, 0x4e, 0xd7, 0xe3, 0x5d, 0x50, 0x1e, 0xb3, 0x5b, 0x23, 0x38, 0x34, 0x68, 0x46, 0x3, 0x8c, 0xdd, 0x9c, 0x7d, 0xa0, 0xcd, 0x1a, 0x41, 0x1c};
static const uint8_t h_table[] = {0x0, 0x1, 0x1, 0x3, 0x1, 0x1, 0x2, 0x7, 0x2, 0x1, 0x7, 0x7, 0x4, 0x5, 0x4, 0x7, 0xb, 0xb, 0x4, 0xb, 0x2, 0x7, 0x4, 0x7, 0x2, 0x9, 0xb, 0xb, 0x7, 0xb, 0xd, 0x7, 0xd, 0xb, 0x1e, 0xb, 0x8, 0x17, 0x1a, 0x1f, 0x1, 0x17, 0x2, 0x7, 0xb, 0x13, 0x1d, 0x7, 0xc, 0x1b, 0xb, 0x13, 0x4, 0xb, 0xd, 0x17, 0x4, 0x7, 0x8, 0xb, 0x1a, 0xd, 0x2, 0x7, 0x3d, 0xd, 0x18, 0xb, 0xc, 0x3d, 0x1a, 0x2f, 0x1b, 0x2d, 0x4, 0x17, 0x8, 0x3d, 0x2, 0x37, 0x13, 0x1, 0x3d, 0x17, 0xb, 0x29, 0x24, 0x7, 0xd, 0xb, 0x37, 0x13, 0x33, 0x1d, 0x4, 0x7, 0x1d, 0x3d, 0x2c, 0x1b, 0x3d, 0xb, 0x18, 0x27, 0x1a, 0x31, 0x28, 0xb, 0x32, 0xd, 0x8, 0x17, 0x32, 0x3d, 0x3a, 0x7, 0x30, 0x33, 0xb, 0x37, 0x17, 0x27, 0x1, 0x33, 0x29, 0x3d, 0x3d, 0x47, 0x3d, 0x3d, 0x47, 0x4f, 0x3a, 0x5b, 0x29, 0x4f, 0x4d, 0x49, 0x6e, 0x7b, 0x1, 0x5d, 0x53, 0x67, 0x31, 0x1b, 0x32, 0x67, 0x49, 0x4f, 0x3d, 0x17, 0xb, 0x45, 0x4f, 0x73, 0x63, 0x4d, 0x45, 0x67, 0x33, 0x13, 0x7c, 0x53, 0x4f, 0x3d, 0x2f, 0x17, 0x7f, 0xb, 0x2d, 0x7f, 0x6e, 0x73, 0x3d, 0x7, 0x67, 0xd, 0x60, 0xb, 0x30, 0x37, 0x32, 0x4f, 0x5d, 0x33, 0x1a, 0x43, 0x43, 0x5b, 0x24, 0x7, 0x1f, 0x1d, 0x7c, 0x5f, 0x5b, 0x4f, 0x4, 0x7f, 0x1a, 0x3d, 0x3, 0xb, 0x43, 0x7f, 0x49, 0x27, 0x2, 0x73, 0x2c, 0x5b, 0x52, 0x43, 0x45, 0x67, 0x6e, 0x5f, 0x52, 0x63, 0x4f, 0x67, 0x28, 0x67, 0x4, 0x43, 0x54, 0x4f, 0x5, 0x49, 0x6e, 0x7, 0x3d, 0x45, 0x54, 0x33, 0x37, 0xb, 0x60, 0x37, 0x1e, 0x17, 0x13, 0x27, 0x3d, 0x1, 0x7b, 0x4f, 0x73, 0x29, 0x67, 0x43, 0x9, 0x3d, 0xb};

R1CS *r1cs_init(const Parameters *params, int init_empty)
{
    R1CS *r1cs = (R1CS *)malloc(sizeof(R1CS));
    r1cs->public_input_count = params->input_variable_domain.size - 1;
    r1cs->private_input_count = params->variable_domain.size - params->input_variable_domain.size;
    r1cs->constraint_count = params->constraint_domain.size;

    assert((r1cs->A = (uint64_t **)malloc(r1cs->constraint_count * sizeof(uint64_t *))));
    assert((r1cs->B = (uint64_t **)malloc(r1cs->constraint_count * sizeof(uint64_t *))));
    assert((r1cs->C = (uint64_t **)malloc(r1cs->constraint_count * sizeof(uint64_t *))));
    size_t constriant_len = params->variable_domain.size;
    if (!init_empty)
    {
        if (params->aes_size == 128)
        {
            for (size_t i = 0; i < r1cs->constraint_count; i++)
            {
                assert((r1cs->A[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->A[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->B[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->B[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->C[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->C[i], 0, constriant_len * params->field_bytesize);
            }
            for (size_t i = 0; i < 28992; i += 3)
                r1cs->A[r1cs_128_A[i]][(r1cs_128_A[i + 1] + 1) * params->field_words - 1] = r1cs_128_A[i + 2];
            for (size_t i = 0; i < 34368; i += 3)
                r1cs->B[r1cs_128_B[i]][(r1cs_128_B[i + 1] + 1) * params->field_words - 1] = r1cs_128_B[i + 2];
            for (size_t i = 0; i < 5568; i += 3)
                r1cs->C[r1cs_128_C[i]][(r1cs_128_C[i + 1] + 1) * params->field_words - 1] = r1cs_128_C[i + 2];

            return r1cs;
        }
        else if (params->aes_size == 192)
        {
            for (size_t i = 0; i < r1cs->constraint_count; i++)
            {
                assert((r1cs->A[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->A[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->B[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->B[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->C[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->C[i], 0, constriant_len * params->field_bytesize);
            }
            for (size_t i = 0; i < 63792; i += 3)
                r1cs->A[r1cs_192_A[i]][(r1cs_192_A[i + 1] + 1) * params->field_words - 1] = r1cs_192_A[i + 2];
            for (size_t i = 0; i < 71328; i += 3)
                r1cs->B[r1cs_192_B[i]][(r1cs_192_B[i + 1] + 1) * params->field_words - 1] = r1cs_192_B[i + 2];
            for (size_t i = 0; i < 11424; i += 3)
                r1cs->C[r1cs_192_C[i]][(r1cs_192_C[i + 1] + 1) * params->field_words - 1] = r1cs_192_C[i + 2];
            return r1cs;
        }
        else if (params->aes_size == 256)
        {
            for (size_t i = 0; i < r1cs->constraint_count; i++)
            {
                assert((r1cs->A[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->A[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->B[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->B[i], 0, constriant_len * params->field_bytesize);
                assert((r1cs->C[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
                memset(r1cs->C[i], 0, constriant_len * params->field_bytesize);
            }
            for (size_t i = 0; i < 75084; i += 3)
                r1cs->A[r1cs_256_A[i]][(r1cs_256_A[i + 1] + 1) * params->field_words - 1] = r1cs_256_A[i + 2];
            for (size_t i = 0; i < 85752; i += 3)
                r1cs->B[r1cs_256_B[i]][(r1cs_256_B[i + 1] + 1) * params->field_words - 1] = r1cs_256_B[i + 2];
            for (size_t i = 0; i < 13752; i += 3)
                r1cs->C[r1cs_256_C[i]][(r1cs_256_C[i + 1] + 1) * params->field_words - 1] = r1cs_256_C[i + 2];
            return r1cs;
        }
    }

    for (size_t i = 0; i < r1cs->constraint_count; i++)
    {
        assert((r1cs->A[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
        memset(r1cs->A[i], 0, constriant_len * params->field_bytesize);
        assert((r1cs->B[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
        memset(r1cs->B[i], 0, constriant_len * params->field_bytesize);
        assert((r1cs->C[i] = (uint64_t *)malloc(constriant_len * params->field_bytesize)));
        memset(r1cs->C[i], 0, constriant_len * params->field_bytesize);
    }
    return r1cs;
}

void r1cs_free(R1CS *r1cs)
{
    for (size_t i = 0; i < r1cs->constraint_count; i++)
    {
        free(r1cs->A[i]);
        free(r1cs->B[i]);
        free(r1cs->C[i]);
    }

    free(r1cs->A);
    free(r1cs->B);
    free(r1cs->C);
    free(r1cs);
}

void set_dummy_constraint_system(R1CS *r1cs, const Parameters *params)
{
    size_t constriants_count = params->constraint_domain.size;

    for (size_t i = 0; i < constriants_count; i++)
    {
        memcpy(&r1cs->A[i][0], params->field_one, params->field_bytesize);
        memcpy(&r1cs->B[i][0], params->field_one, params->field_bytesize);
        memcpy(&r1cs->C[i][0], params->field_one, params->field_bytesize);
    }
}

void r1cs_get_g(uint64_t *result, int idx, const Parameters *params)
{
    memset(result, 0, params->field_bytesize);
    if (idx == 0)
    {
        return;
    }
    memset(result, 0, params->field_bytesize);
    result[params->field_words - 1] = (uint64_t)g_table[idx - 1];
}

void r1cs_get_h(uint64_t *result, int idx, const Parameters *params)
{
    assert(idx);
    memset(result, 0, params->field_bytesize);
    result[params->field_words - 1] = (uint64_t)h_table[idx - 1];
}

uint64_t *new_aes_key_expansion(size_t aes_bits, const Parameters *params)
{
    assert(aes_bits == 128); // currently, only support AES128
    const size_t words = 4;
    const size_t rounds = 11;
    return (uint64_t *)malloc(words * rounds * params->field_bytesize);
}

void convert_key_expansion_to_field(uint64_t *result, uint8_t *ke, size_t aes_bits, const Parameters *params)
{
    assert(aes_bits == 128); // currently, only support AES128
    const size_t words = 4;
    const size_t rounds = 11;
    memset(result, 0, words * rounds * params->field_bytesize);
    for (size_t i = 0; i < rounds; ++i)
    {
        for (size_t j = 0; j < words; ++j)
        {
            result[i * words + j] = (uint64_t)ke[i * words + j];
        }
    }
}

void free_aes_key_expansion(uint64_t *ke)
{
    free(ke);
}

static void set_values_bits(uint64_t *input, size_t start, const uint8_t *values, size_t len, const Parameters *params)
{
    for (size_t i = 0; i < len; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if (values[i] & 1 << j)
            {
                memcpy(&input[(start + i * 8 + j) * params->field_words], params->field_one, params->field_bytesize);
            }
        }
    }
}

static void set_values(uint64_t *input, size_t start, const uint8_t *values, size_t len, const Parameters *params)
{
    for (size_t i = 0; i < len; i++)
    {
        input[(start + i + 1) * params->field_words - 1] = values[i];
    }
}

uint64_t *new_r1cs_instance(const uint8_t *pk, const Parameters *params)
{
    uint64_t *result = (uint64_t *)malloc((params->variable_domain.size - 1) * params->field_bytesize);
    memset(result, 0, (params->variable_domain.size - 1) * params->field_bytesize);
    size_t cur = 0;

    // set public input v
    if (params->aes_size == 128)
    {
        assert((cur + 16) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk + 16 * sizeof(uint8_t), 16, params);
        cur += 16;
        assert((cur + 16) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk, 16, params);
        cur += 16;
    }
    else if (params->aes_size == 192)
    {
        assert((cur + 32) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk + 24 * sizeof(uint8_t), 32, params);
        cur += 32;
        assert((cur + 32) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk, 24, params);
        cur += 32;
    }
    else if (params->aes_size == 256)
    {
        assert((cur + 32) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk + 32 * sizeof(uint8_t), 32, params);
        cur += 32;
        assert((cur + 32) < (params->input_variable_domain.size - 1));
        set_values(result, cur, pk, 32, params);
        cur += 32;
    }
    assert(cur < (params->variable_domain.size - 1));

    return result;
}

uint64_t *new_r1cs_witness(const uint8_t *aes_key, size_t aes_key_len,
                           const uint8_t *pk, const struct AES_ctx *aes_ctx0,
                           const struct AES_ctx *aes_ctx1,
                           const Parameters *params)
{
    assert(((params->aes_size > 128) && aes_ctx1) || (params->aes_size == 128));
    size_t max_result_len = params->variable_domain.size - params->input_variable_domain.size;
    uint64_t *result = (uint64_t *)malloc(max_result_len * params->field_bytesize);
    memset(result, 0, max_result_len * params->field_bytesize);
    size_t cur = 0;

    // set private input w
    size_t aes_round = 10;
    size_t r_count = 10;
    if (params->aes_size == 192)
    {
        aes_round = 12;
        r_count = 8;
    }
    else if (params->aes_size == 256)
    {
        aes_round = 14;
        r_count = 13;
    }

    for (size_t i = 0; i < aes_round; i++)
    {
        uint8_t gs[16];
        uint8_t hs[16];
        for (size_t j = 0; j < 16; j++)
        {
            uint8_t b = pk[j];
            uint8_t k = aes_ctx0->RoundKey[i * 16 + j];
            if (i != 0)
            {
                b = aes_ctx0->inter_states[(i - 1) * 16 + j];
            }
            b ^= k;

            if (b == 0)
            {
                gs[j] = 0;
                hs[j] = 0x80;
            }
            else
            {
                gs[j] = g_table[b - 1];
                hs[j] = h_table[b - 1];
            }
        }
        assert((cur + 16 * 8) < max_result_len);
        set_values_bits(result, cur, hs, 16, params);
        cur += 16 * 8;
        assert((cur + 16 * 8) < max_result_len);
        set_values_bits(result, cur, gs, 16, params);
        cur += 16 * 8;
    }

    if (params->aes_size == 192 || params->aes_size == 256)
    {
        for (size_t i = 0; i < aes_round; i++)
        {
            uint8_t gs[16];
            uint8_t hs[16];
            for (size_t j = 0; j < 16; j++)
            {
                uint8_t b = 0;
                uint8_t k = aes_ctx1->RoundKey[i * 16 + j];
                if (i != 0)
                    b = aes_ctx1->inter_states[(i - 1) * 16 + j];
                else if (params->aes_size == 256 || (params->aes_size == 192 && j < 8))
                    b = pk[16 + j];

                b ^= k;

                if (b == 0)
                {
                    gs[j] = 0;
                    hs[j] = 0x80;
                }
                else
                {
                    gs[j] = g_table[b - 1];
                    hs[j] = h_table[b - 1];
                }
            }
            assert((cur + 16 * 8) < max_result_len);
            set_values_bits(result, cur, hs, 16, params);
            cur += 16 * 8;
            assert((cur + 16 * 8) < max_result_len);
            set_values_bits(result, cur, gs, 16, params);
            cur += 16 * 8;
        }
    }

    assert((cur + aes_key_len) < max_result_len);
    set_values(result, cur, aes_key, aes_key_len, params);
    cur += aes_key_len;

    size_t tempas_len = r_count * 4;
    assert((cur + tempas_len) < max_result_len);
    set_values(result, cur, aes_ctx0->tempas, tempas_len, params);
    cur += tempas_len;

    for (size_t i = 0; i < r_count; i++)
    {
        uint8_t gs[4];
        uint8_t hs[4];

        for (size_t j = 0; j < 4; j++)
        {
            size_t b_index = i * 16 + 12 + j;
            if (params->aes_size == 192)
            {
                b_index = i * 24 + 20 + j;
            }
            else if (params->aes_size == 256)
            {
                b_index = (i + 1) * 16 + 12 + j;
            }
            uint8_t b = aes_ctx0->RoundKey[b_index];
            if (b == 0)
            {
                gs[j] = 0;
                hs[j] = 0x80;
            }
            else
            {
                gs[j] = g_table[b - 1];
                hs[j] = h_table[b - 1];
            }
        }
        assert((cur + 4 * 8) < max_result_len);
        set_values_bits(result, cur, hs, 4, params);
        cur += 4 * 8;
        assert((cur + 4 * 8) < max_result_len);
        set_values_bits(result, cur, gs, 4, params);
        cur += 4 * 8;
    }

    assert(cur < max_result_len);

    return result;
}

void free_r1cs_instance(uint64_t *instance)
{
    free(instance);
}

void free_r1cs_witness(uint64_t *witness)
{
    free(witness);
}
