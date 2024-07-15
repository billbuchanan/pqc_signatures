#ifndef _ORACLE_H__
#define _ORACLE_H__

#include <string.h>
#include <stdint.h>

#include "domain.h"
#include "params.h"

typedef struct Oracle Oracle;

typedef int (*oracle_evaluation_at_point_fn)(uint64_t *result, Oracle *oracle, const Parameters *params, size_t evaluation_position, const uint64_t *evaluation_point, const uint64_t *constituent_evaluations, size_t constituent_evaluations_len);

typedef struct Oracle
{
    int _id;
    uint64_t *content;
    size_t content_len;
    size_t content_deg;
    int is_virtual;
    const Domain *domain;
    struct Oracle **constituent_oracles;
    size_t constituent_oracles_len;
    oracle_evaluation_at_point_fn evaluation_at_point;
    void *data;
} Oracle;

extern Oracle fw_oracle;
extern Oracle fAz_oracle;
extern Oracle fBz_oracle;
extern Oracle fCz_oracle;
extern Oracle sumcheck_masking_poly_oracle;
extern Oracle ldt_blinding_vector_oracle;
extern Oracle sumcheck_h_oracle;
extern Oracle fz_oracle;
extern Oracle rowcheck_oracle;
extern Oracle multi_lincheck_oracle;
extern Oracle combined_f_oracle;
extern Oracle sumcheck_g_oracle;
extern Oracle combined_ldt_oracle;

void init_oracles(const Parameters *params);
void deinit_oracles();

#endif
