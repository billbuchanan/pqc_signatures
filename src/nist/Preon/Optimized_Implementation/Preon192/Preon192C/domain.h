#ifndef _domain_H__
#define _domain_H__

#include <string.h>
#include <stdint.h>

typedef struct Domain
{
    // int is_standard_basis;
    size_t size;
    size_t basis_len; // basis_len = log2(size), should it be renamed into dimension?
    const uint64_t *basis;
    const uint64_t *shift;
} Domain;

#endif
