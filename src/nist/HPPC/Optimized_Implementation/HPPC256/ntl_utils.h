#ifndef NTL_UTILS_H
#define NTL_UTILS_H

#include <m4ri/m4ri.h>
#include <flint/flint.h>
#include <flint/fq_poly.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    void InitNTL(int n, fq_ctx_t ctx, unsigned long seed);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"{
#endif
    bool RootPrivatePoly(mzd_t *_private_polynomial, mzd_t *root, mzd_t *_const_term, int n, int degree, fq_ctx_t ctx);
#ifdef __cplusplus
}
#endif

#endif