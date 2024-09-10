#include "poly.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "aurora_inner.h"
#include "domain.h"
#include "field.h"
#include "fft.h"

int poly_add(uint64_t *sum, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words)
{
    size_t sum_len = a_len;
    if (b_len > a_len)
        sum_len = b_len;
    // Assume sum is already allocated?
    // sum = realloc(ptr, new_size);
    // new_ptr = realloc(ptr, new_size);
    // if(new_ptr == null){  //用 !new_ptr 檢查也可以
    //     // 錯誤處理
    // }
    // ptr = new_ptr
    uint64_t *zero = (uint64_t *)malloc(field_words * sizeof(uint64_t));
    memset(zero, 0, field_words * sizeof(uint64_t));
    for (size_t i = 0; i < sum_len; i++)
    {
        if (i < a_len && i < b_len)
            field_add(&sum[i * field_words], &a[i * field_words], &b[i * field_words], field_words);
        else if (i >= a_len && i < b_len)
            field_add(&sum[i * field_words], zero, &b[i * field_words], field_words);
        else if (i >= b_len && i < a_len)
            field_add(&sum[i * field_words], &a[i * field_words], zero, field_words);
        else
            return -1; // Or any error code about out of range
    }
    free(zero);
    return 0;
}

void poly_addEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words)
{
    // A potential reallocation could happen here
    // Or is it ok to only support a_len >= b_len?
    // So that a is already allocated to desired length
    assert(a_len >= b_len);
    for (size_t i = 0; i < a_len * field_words; i += field_words)
    {
        if (i < b_len * field_words)
            field_addEqual(&a[i], &b[i], field_words);
    }
}

void poly_scalarMul(uint64_t *product, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t field_words)
{
    memset(product, 0, a_len * field_words * sizeof(uint64_t));
    if (is_zero(b, field_words))
    {
        return;
    }

    for (size_t i = 0; i < a_len * field_words; i += field_words)
    {
        if (!is_zero(&a[i], field_words))
        {
            field_mul(&product[i], &a[i], b);
        }
    }
}

void poly_scalarMulEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t field_words)
{
    if (is_zero(b, field_words))
    {
        memset(a, 0, a_len * field_words * sizeof(uint64_t));
        return;
    }
    for (size_t i = 0; i < a_len * field_words; i += field_words)
    {
        if (!is_zero(&a[i], field_words))
        {
            field_mulEqual(&a[i], b, field_words);
        }
    }
}

void poly_mul(uint64_t *product, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words)
{
    // product should be as length a_len + b_len - 1
    size_t product_len = a_len + b_len - 1;
    memset(product, 0, product_len * field_words * sizeof(uint64_t));
    uint64_t *temp = (uint64_t *)malloc(b_len * field_words * sizeof(uint64_t));
    memset(temp, 0, b_len * field_words * sizeof(uint64_t));
    for (size_t i = 0; i < a_len * field_words; i += field_words)
    {
        if (!is_zero(&a[i], field_words))
        {
            poly_scalarMul(temp, b, b_len, &a[i], field_words);
            poly_addEqual(&product[i], b_len, temp, b_len, field_words);
        }
    }
    free(temp);
}
void poly_mulEqual(uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words)
{
    // May not be as straightforward as poly_mul
    // Huge cost when duplicating, probably not implementing this
    assert("not implemented yet" && 0);
}

void poly_div(uint64_t *quotient, uint64_t *remainder, const uint64_t *a, const size_t a_len, const uint64_t *b, const size_t b_len, const size_t field_words)
{
    // a = b * quotient + remainder
    // inverse of the leading term of b
    size_t field_bytesize = field_words * sizeof(uint64_t);
    uint64_t *leadingTermInverse = (uint64_t *)malloc(field_bytesize);
    field_inv(leadingTermInverse, &b[(b_len - 1) * field_words]);
    const size_t remainder_len = b_len - 1;

    if (a_len < b_len)
    {
        memset(quotient, 0, field_bytesize);
        memcpy(remainder, a, a_len * field_bytesize);
        free(leadingTermInverse);
        return;
    }

    memcpy(remainder, a, remainder_len * field_bytesize);
    memcpy(quotient, &a[remainder_len * field_words], (a_len - remainder_len) * field_bytesize);

    for (size_t i = (a_len - remainder_len) * field_words; i > 0; i -= field_words)
    {
        // printf("%zu\n", i);
        uint64_t *twist = (uint64_t *)malloc(field_bytesize);
        field_mul(twist, &quotient[i - field_words], leadingTermInverse);
        memcpy(&quotient[i - field_words], twist, field_bytesize);

        /* subtract twist*Z * y^i thus clearing the i-th term of P */
        if (b_len >= 2 && !is_zero(twist, field_words))
        {
            for (size_t j = (b_len - 1) * field_words; j > 0; j -= field_words)
            {
                // printf("%zu\n", j);
                uint64_t *deduction = (uint64_t *)malloc(field_bytesize);
                field_mul(deduction, twist, &b[j - field_words]);
                if (!is_zero(deduction, field_words))
                {
                    if ((i - field_words + j - field_words) < remainder_len * field_words)
                        field_addEqual(&remainder[i - field_words + j - field_words], deduction, field_words);
                    else
                        field_addEqual(&quotient[i - field_words + j - field_words - remainder_len * field_words], deduction, field_words);
                }
                free(deduction);
            }
        }
        free(twist);
    }
    free(leadingTermInverse);
}

void poly_eval(uint64_t *result, const uint64_t *poly, const size_t poly_len, const uint64_t *x, const size_t field_words)
{
    memset(result, 0, field_words * sizeof(uint64_t));
    for (size_t i = poly_len * field_words; i > 0; i -= field_words)
    {
        field_mulEqual(result, x, field_words);
        field_addEqual(result, &poly[i - field_words], field_words);
    }
}

void poly_eval_over_domain(uint64_t *result, const uint64_t *poly, const size_t poly_len, const Domain *domain, const size_t field_words, const size_t field_bytesize)
{
    fft(result, poly, poly_len, field_words, domain->size, domain->shift);
    // TODO: fix below optimized code
    /*
        We implement /affine/ linearized polynomials for which the
        bilinear property is not directly applicable, as we need to make
        sure that constant term is included only once.

        Therefore, evaluating over subspace below, we subtract constant
        term from evaluations over the basis, but include the constant
        term in the shift calculation.
        */

    // size_t domain_dimension = domain->basis_len;
    // uint64_t *eval_at_basis = (uint64_t *)malloc(domain_dimension * field_bytesize);
    // for (size_t i = 0; i < domain_dimension; i++)
    // {
    //     poly_eval(&eval_at_basis[i * field_words], poly, poly_len, &domain->basis[i * field_words], field_words);
    //     field_subEqual(&eval_at_basis[i * field_words], &poly[0], field_words);
    // }
    // uint64_t *eval_at_shift = (uint64_t *)malloc(field_bytesize);
    // poly_eval(eval_at_shift, poly, poly_len, domain->shift, field_words);
    // all_subset_sums(result, eval_at_basis, domain_dimension, eval_at_shift, field_words);

    // free(eval_at_basis);
    // free(eval_at_shift);
}

size_t poly_deg(const uint64_t *poly, const size_t poly_len, const size_t field_words)
{
    for (size_t i = poly_len; i > 0; i--)
    {
        if (!is_zero(&poly[(i - 1) * field_words], field_words))
        {
            return i - 1;
            break;
        }
    }
    return 0;
}