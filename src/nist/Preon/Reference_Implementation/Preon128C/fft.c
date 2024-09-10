#include "fft.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aurora_inner.h"
#include "field.h"

static void bitreverse_vector(uint64_t *result, size_t result_len, size_t field_words)
{
    const size_t dimension = (size_t)log2(result_len);

    uint64_t *temp = (uint64_t *)malloc(sizeof(uint64_t) * field_words);
    for (size_t i = 0; i < result_len; ++i)
    {
        // const size_t ri = bitreverse(i, dimension);
        size_t ri = 0;
        size_t n = i;
        for (size_t k = 0; k < dimension; ++k)
        {
            ri = (ri << 1) | (n & 1);
            n >>= 1;
        }

        if (i < ri)
        {
            field_swap_with_tmp(&result[i * field_words], &result[ri * field_words], temp, sizeof(uint64_t) * field_words);
        }
    }
    free(temp);
}

void fft(uint64_t *result, const uint64_t *v, const size_t v_len, const size_t field_words, const size_t domain_size, const uint64_t *domain_shift)
{
    const size_t filed_bytesize = field_words * sizeof(uint64_t);

    // result should be allocated to domain_size before
    memcpy(result, v, v_len * filed_bytesize);
    if (domain_size > v_len)
        memset(&result[v_len * field_words], 0, (domain_size - v_len) * filed_bytesize);

    // const size_t domain_size = result.size();
    const size_t domain_dimension = (size_t)log2(domain_size);
    assert(domain_size == (1ull << domain_dimension));

    uint64_t *recursed_betas = (uint64_t *)malloc((domain_dimension + 1) * domain_dimension / 2 * filed_bytesize);
    uint64_t *recursed_shifts = (uint64_t *)malloc(domain_dimension * filed_bytesize);
    size_t recursed_betas_ptr = 0;

    uint64_t *betas2 = (uint64_t *)malloc(domain_dimension * filed_bytesize);
    for (size_t i = 0; i < domain_dimension; i++)
    {
        uint64_t *basis = (uint64_t *)malloc(filed_bytesize);
        memset(basis, 0, filed_bytesize);
        uint64_t partial_basis = 1ull << (i % 64);
        basis[field_words - (size_t)(i / 64) - 1] = partial_basis;
        memcpy(&betas2[i * field_words], basis, filed_bytesize);
        free(basis);
    }

    uint64_t *shift2 = (uint64_t *)malloc(filed_bytesize);
    memcpy(shift2, domain_shift, filed_bytesize);
    for (size_t j = 0; j < domain_dimension; ++j)
    {
        uint64_t *beta = &betas2[(domain_dimension - 1 - j) * field_words];
        uint64_t *betai = (uint64_t *)malloc(filed_bytesize);
        memset(betai, 0, filed_bytesize);
        betai[field_words - 1] = 1;

        /* twist by beta. TODO: this can often be elided by a careful choice of betas */
        for (size_t ofs = 0; ofs < domain_size; ofs += (1ull << j))
        {
            for (size_t p = 0; p < (1ull << j); ++p)
                field_mulEqual(&result[(ofs + p) * field_words], betai, field_words);

            field_mulEqual(betai, beta, field_words);
        }

        /* perform radix conversion */
        for (size_t stride = domain_size / 4; stride >= (1ul << j); stride >>= 1)
        {
            for (size_t ofs = 0; ofs < domain_size; ofs += stride * 4)
            {
                for (size_t i = 0; i < stride; ++i)
                {
                    field_addEqual(&result[(ofs + 2 * stride + i) * field_words],
                                   &result[(ofs + 3 * stride + i) * field_words], field_words);
                    field_addEqual(&result[(ofs + 1 * stride + i) * field_words],
                                   &result[(ofs + 2 * stride + i) * field_words], field_words);
                }
            }
        }

        /* compute deltas used in the reverse process */
        uint64_t *betainv = (uint64_t *)malloc(filed_bytesize);
        field_inv(betainv, beta);
        for (size_t i = 0; i < domain_dimension - 1 - j; ++i)
        {
            uint64_t *newbeta = (uint64_t *)malloc(filed_bytesize);
            field_mul(newbeta, &betas2[i * field_words], betainv);
            // recursed_betas[recursed_betas_ptr++] = newbeta;
            memcpy(&recursed_betas[(recursed_betas_ptr++) * field_words], newbeta, filed_bytesize);
            uint64_t *newbeta_squared = (uint64_t *)malloc(filed_bytesize);
            memcpy(newbeta_squared, newbeta, filed_bytesize);
            field_mulEqual(newbeta_squared, newbeta_squared, field_words);
            field_sub(&betas2[i * field_words], newbeta_squared, newbeta, field_words);
            free(newbeta);
            free(newbeta_squared);
        }

        uint64_t *newshift = (uint64_t *)malloc(filed_bytesize);
        field_mul(newshift, shift2, betainv);
        uint64_t *newshift_squared = (uint64_t *)malloc(filed_bytesize);
        memcpy(newshift_squared, newshift, filed_bytesize);
        field_mulEqual(newshift_squared, newshift_squared, field_words);
        memcpy(&recursed_shifts[j * field_words], newshift, filed_bytesize);
        field_sub(shift2, newshift_squared, newshift, field_words);
        free(betainv);
        free(betai);
        free(newshift);
        free(newshift_squared);
    }

    // bitreverse_vector<FieldT>(result);
    bitreverse_vector(result, domain_size, field_words);

    /* unwind the recursion */
    for (size_t j = 0; j < domain_dimension; ++j)
    {
        recursed_betas_ptr -= j;
        /* note that this devolves to empty range for the first loop iteration */
        uint64_t *popped_betas = (uint64_t *)malloc(j * filed_bytesize);
        memcpy(popped_betas, &recursed_betas[recursed_betas_ptr * field_words], j * filed_bytesize);
        const uint64_t *popped_shift = &recursed_shifts[(domain_dimension - 1 - j) * field_words];

        size_t stride = 1ull << j;
        // uint64_t *sums = all_subset_sums<FieldT>(popped_betas, popped_shift);
        uint64_t *sums = (uint64_t *)malloc(stride * filed_bytesize);
        all_subset_sums(sums, popped_betas, j, popped_shift, field_words);

        for (size_t ofs = 0; ofs < domain_size; ofs += 2 * stride)
        {
            for (size_t i = 0; i < stride; ++i)
            {
                uint64_t *temp = (uint64_t *)malloc(filed_bytesize);
                // S[ofs+i] += S[ofs+stride+i] * sums[i];
                // memcpy(temp, &result[(ofs + stride + i) * field_words], filed_bytesize);
                field_mul(temp, &result[(ofs + stride + i) * field_words], &sums[i * field_words]);
                field_addEqual(&result[(ofs + i) * field_words], temp, field_words);
                // S[ofs+stride+i] += S[ofs+i];
                field_addEqual(&result[(ofs + stride + i) * field_words], &result[(ofs + i) * field_words], field_words);
                free(temp);
            }
        }
        free(popped_betas);
        free(sums);
    }
    assert(recursed_betas_ptr == 0);

    free(recursed_betas);
    free(recursed_shifts);
    free(betas2);
    free(shift2);
}

void ifft(uint64_t *result, const uint64_t *v, const size_t field_words, const size_t domain_size, const uint64_t *domain_shift)
{
    // const size_t domain_size = evals.size();
    const size_t filed_bytesize = field_words * sizeof(uint64_t);
    const size_t domain_dimension = (size_t)log2(domain_size);
    assert(domain_size == (1ull << domain_dimension));

    memcpy(result, v, domain_size * filed_bytesize);
    uint64_t *recursed_twists = (uint64_t *)malloc(domain_dimension * filed_bytesize);
    memset(recursed_twists, 0, domain_dimension * filed_bytesize);

    uint64_t *betas2 = (uint64_t *)malloc(domain_dimension * filed_bytesize);
    for (size_t i = 0; i < domain_dimension; i++)
    {
        uint64_t *basis = (uint64_t *)malloc(filed_bytesize);
        memset(basis, 0, filed_bytesize);
        uint64_t partial_basis = 1ull << (i % 64);
        basis[field_words - (size_t)(i / 64) - 1] = partial_basis;
        memcpy(&betas2[i * field_words], basis, filed_bytesize);
        free(basis);
    }

    uint64_t *shift2 = (uint64_t *)malloc(filed_bytesize);
    memcpy(shift2, domain_shift, filed_bytesize);

    for (size_t j = 0; j < domain_dimension; ++j)
    {
        const uint64_t *beta = &betas2[(domain_dimension - 1 - j) * field_words];
        uint64_t *betainv = (uint64_t *)malloc(filed_bytesize);
        field_inv(betainv, beta);
        memcpy(&recursed_twists[j * field_words], betainv, filed_bytesize);

        uint64_t *newbetas = (uint64_t *)malloc((domain_dimension - 1 - j) * filed_bytesize);

        for (size_t i = 0; i < domain_dimension - 1 - j; ++i)
        {
            uint64_t *newbeta = (uint64_t *)malloc(filed_bytesize);
            field_mul(newbeta, &betas2[i * field_words], betainv);
            memcpy(&newbetas[i * field_words], newbeta, filed_bytesize);
            uint64_t *newbeta_squared = (uint64_t *)malloc(filed_bytesize);
            memcpy(newbeta_squared, newbeta, filed_bytesize);
            field_mulEqual(newbeta_squared, newbeta_squared, field_words);
            field_sub(&betas2[i * field_words], newbeta_squared, newbeta, field_words);
            free(newbeta);
            free(newbeta_squared);
        }

        uint64_t *newshift = (uint64_t *)malloc(filed_bytesize);
        field_mul(newshift, shift2, betainv);
        uint64_t *newshift_squared = (uint64_t *)malloc(filed_bytesize);
        memcpy(newshift_squared, newshift, filed_bytesize);
        field_mulEqual(newshift_squared, newshift_squared, field_words);
        field_sub(shift2, newshift_squared, newshift, field_words);

        // const uint64_t *sums = all_subset_sums<FieldT>(newbetas, newshift);
        const size_t sums_len = 1ull << (domain_dimension - 1 - j);
        uint64_t *sums = (uint64_t *)malloc(sums_len * filed_bytesize);
        all_subset_sums(sums, newbetas, domain_dimension - 1 - j, newshift, field_words);

        for (size_t ofs = 0; ofs < domain_size; ofs += 2 * sums_len)
        {
            for (size_t p = 0; p < sums_len; ++p)
            {
                field_addEqual(&result[(ofs + sums_len + p) * field_words], &result[(ofs + p) * field_words], field_words);
                uint64_t *temp = (uint64_t *)malloc(filed_bytesize);
                memcpy(temp, &result[(ofs + sums_len + p) * field_words], filed_bytesize);
                field_mulEqual(temp, &sums[p * field_words], field_words);
                field_addEqual(&result[(ofs + p) * field_words], temp, field_words);
                free(temp);
            }
        }

        free(betainv);
        free(newbetas);
        free(newshift);
        free(newshift_squared);
        free(sums);
    }

    // bitreverse_vector<FieldT>(result);
    bitreverse_vector(result, domain_size, field_words);

    for (size_t j = 0; j < domain_dimension; ++j)
    {
        size_t n = 4ull << (domain_dimension - 1 - j);
        /* perform radix combinations */
        while (n <= domain_size)
        {
            const size_t quarter = n / 4;
            for (size_t ofs = 0; ofs < domain_size; ofs += n)
            {
                for (size_t i = 0; i < quarter; ++i)
                {
                    field_addEqual(&result[(ofs + 1 * quarter + i) * field_words],
                                   &result[(ofs + 2 * quarter + i) * field_words],
                                   field_words);
                    field_addEqual(&result[(ofs + 2 * quarter + i) * field_words],
                                   &result[(ofs + 3 * quarter + i) * field_words],
                                   field_words);
                }
            }
            n *= 2;
        }

        /* twist by \beta^{-1} */
        const uint64_t *betainv = &recursed_twists[(domain_dimension - 1 - j) * field_words];
        uint64_t *betainvi = (uint64_t *)malloc(filed_bytesize);
        memset(betainvi, 0, filed_bytesize);
        betainvi[field_words - 1] = 1;
        for (size_t ofs = 0; ofs < domain_size; ofs += (1ull << (domain_dimension - 1 - j)))
        {
            for (size_t p = 0; p < (1ull << (domain_dimension - 1 - j)); ++p)
            {
                field_mulEqual(&result[(ofs + p) * field_words], betainvi, field_words);
            }
            field_mulEqual(betainvi, betainv, field_words);
        }
        free(betainvi);
    }

    free(recursed_twists);
    free(betas2);
    free(shift2);
}
