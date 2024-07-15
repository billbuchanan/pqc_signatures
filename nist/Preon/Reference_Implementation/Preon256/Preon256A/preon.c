#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "aurora_inner.h"
#include "aurora.h"
#include "params.h"
#include "preon.h"
#include "r1cs.h"
#include "rand.h"
#include "util.h"

size_t publickey_len(int aes_size)
{
    switch (aes_size)
    {
    case 128:
        return 32;
    case 192:
        return 56;
    case 256:
        return 64;
    default:
        assert(0);
    }
}

size_t privatekey_len(int aes_size)
{
    switch (aes_size)
    {
    case 128:
        return 16;
    case 192:
        return 24;
    case 256:
        return 32;
    default:
        assert(0);
    }
}

void keygen(int aes_size, uint8_t *sk, size_t sk_len, uint8_t *pk)
{
    random_bytes(sk, sk_len);
    random_bytes(pk, 16);
    if (aes_size == 128)
    {
        uint8_t *ct = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(ct, pk, 16 * sizeof(uint8_t));
        struct AES_ctx *test_ctx = AES(sk, ct);
        memcpy(pk + 16 * sizeof(uint8_t), ct, 16 * sizeof(uint8_t));
        free(ct);
        free_AES_ctx(test_ctx);
    }
    else
    {
        if (aes_size == 192)
        {
            memset(pk + 16 * sizeof(uint8_t), 0, 16 * sizeof(uint8_t));
            random_bytes(pk + 16 * sizeof(uint8_t), 8);
        }
        else if (aes_size == 256)
            random_bytes(pk + 16 * sizeof(uint8_t), 16);

        uint8_t *ct = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(ct, pk, 16 * sizeof(uint8_t));
        struct AES_ctx *test_ctx = AES(sk, ct);
        if (aes_size == 192)
        {
            memcpy(pk + 24 * sizeof(uint8_t), ct, 16 * sizeof(uint8_t));
            memset(ct, 0, 16 * sizeof(uint8_t));
            memcpy(ct, pk + 16 * sizeof(uint8_t), 8 * sizeof(uint8_t));
        }
        else if (aes_size == 256)
        {
            memcpy(pk + 32 * sizeof(uint8_t), ct, 16 * sizeof(uint8_t));
            memcpy(ct, pk + 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
        }

        test_ctx = AES(sk, ct);
        if (aes_size == 192)
            memcpy(pk + 40 * sizeof(uint8_t), ct, 16 * sizeof(uint8_t));
        else if (aes_size == 256)
            memcpy(pk + 48 * sizeof(uint8_t), ct, 16 * sizeof(uint8_t));

        free(ct);
        free_AES_ctx(test_ctx);
    }
}

size_t sign(void *sig, size_t sig_len, int sig_type, const void *sk, size_t sk_len, const void *pk, size_t pk_len, const void *msg, size_t msg_len)
{
    // Prepare
    const Parameters *params = get_parameters(sig_type);
    size_t primary_input_count = params->input_variable_domain.size - 1;
    size_t auxiliary_input_count = params->variable_domain.size - params->input_variable_domain.size;
    size_t aurora_input_len = params->variable_domain.size - 1;

    R1CS *r1cs = r1cs_init(params, 0);
    uint64_t *aurora_input = (uint64_t *)malloc(aurora_input_len * params->field_bytesize);

    if (params->aes_size == 128)
    {
        uint8_t *ct = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(ct, pk, 16 * sizeof(uint8_t));

        struct AES_ctx *aes_ctx = AES(sk, ct);

        uint64_t *instance = new_r1cs_instance(pk, params);
        uint64_t *witness = new_r1cs_witness(sk, sk_len, pk, aes_ctx, NULL, params);
        memcpy(aurora_input, instance, primary_input_count * params->field_bytesize);
        memcpy(&aurora_input[primary_input_count * params->field_words], witness, auxiliary_input_count * params->field_bytesize);
        free(ct);
        free_r1cs_instance(instance);
        free_r1cs_witness(witness);
        free_AES_ctx(aes_ctx);
    }
    else if (params->aes_size == 192 || params->aes_size == 256)
    {

        uint8_t *ct0 = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(ct0, pk, 16 * sizeof(uint8_t));
        struct AES_ctx *aes_ctx_0 = AES(sk, ct0);

        uint8_t *ct1 = (uint8_t *)malloc(16 * sizeof(uint8_t));
        if (params->aes_size == 192)
        {
            memset(ct1, 0, 16 * sizeof(uint8_t));
            memcpy(ct1, pk + 16 * sizeof(uint8_t), 8 * sizeof(uint8_t));
        }
        else if (params->aes_size == 256)
            memcpy(ct1, pk + 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));

        struct AES_ctx *aes_ctx_1 = AES(sk, ct1);

        uint64_t *instance = new_r1cs_instance(pk, params);
        uint64_t *witness = new_r1cs_witness(sk, sk_len, pk, aes_ctx_0, aes_ctx_1, params);
        memcpy(aurora_input, instance, primary_input_count * params->field_bytesize);
        memcpy(&aurora_input[primary_input_count * params->field_words], witness, auxiliary_input_count * params->field_bytesize);
        free(ct0);
        free(ct1);
        free_r1cs_instance(instance);
        free_r1cs_witness(witness);
        free_AES_ctx(aes_ctx_0);
        free_AES_ctx(aes_ctx_1);
    }

    size_t max_sig_len = aurora_proof_max_size(sig_type);
    size_t actual_sig_len = aurora_prover(sig, max_sig_len, sig_type, msg, msg_len,
                                          r1cs, aurora_input, primary_input_count, auxiliary_input_count);
    free(aurora_input);
    r1cs_free(r1cs);

    if (actual_sig_len == 0 || actual_sig_len > max_sig_len)
    {
        // printf("FAILURE!\n");
        return 0;
    }

    return actual_sig_len;
}

int verify(const void *sig, size_t sig_len, int sig_type, const void *pk, size_t pk_len, const void *msg, size_t msg_len)
{
    // Prepare
    const Parameters *params = get_parameters(sig_type);
    R1CS *r1cs = r1cs_init(params, 0);

    uint64_t *instance = new_r1cs_instance(pk, params);
    size_t aurora_input_len = params->input_variable_domain.size - 1;
    uint64_t *aurora_input = (uint64_t *)malloc(aurora_input_len * params->field_bytesize);
    memcpy(aurora_input, instance, aurora_input_len * params->field_bytesize);

    int result = aurora_verifier((const uint8_t *)sig, sig_len, sig_type, msg, msg_len, r1cs, aurora_input, aurora_input_len);

    if (!result)
    {
        // printf("FAILURE!\n");
    }

    free(aurora_input);
    free_r1cs_instance(instance);
    r1cs_free(r1cs);

    return result;
}
