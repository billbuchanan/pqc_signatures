#ifndef _QUERY_H__
#define _QUERY_H__

#include <stddef.h>
#include <stdint.h>

#include "oracles.h"
#include "params.h"

typedef struct QuerySet
{
    size_t interaction_index;
    size_t LDT_index;
    size_t s0_position;
    // size_t **f_at_s_coset_query; // f_at_s_coset_query[num_reductions][current_coset_size], this may not be required since we can recursively compute it
} QuerySet;

void get_all_query_positions(size_t **result, const uint8_t *hash_state, const Domain *fri_domains, const Parameters *params);
size_t all_query_sets_len(const Parameters *params);
void get_all_query_sets(QuerySet *result, const uint8_t *hash_state, const Domain *fri_domains, const Parameters *params);
void get_all_query_positions_from_all_query_sets(size_t **result, const QuerySet *query_sets, const Domain *fri_domains, const Parameters *params);
size_t compute_first_query_pos(size_t initial_query, size_t i, size_t coset_size);
void get_next_query_positions(size_t *result, size_t cur_query_pos_root, size_t cur_coset_size, size_t next_coset_size);

void get_query_response(uint64_t *result, const Oracle *query_oracle, const size_t query_pos, const Parameters *params);
#endif
