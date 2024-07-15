#ifndef _PARAMS_H__
#define _PARAMS_H__

#include <stddef.h>
#include <stdint.h>

#include "domain.h"

typedef struct
{
    size_t aes_size;
    size_t field_bitsize;
    size_t field_bytesize;
    size_t field_words;
    size_t hash_bitsize;
    size_t hash_bytesize;
    size_t hash_zk_bytesize;
    size_t verifier_messages_count[17];
    int query_bound;
    int num_interaction_rounds;
    Domain input_variable_domain;
    Domain variable_domain;
    Domain constraint_domain;
    Domain summation_domain;
    Domain codeword_domain;
    Domain fri_domains[17];
    Domain fri_localizer_domains[16];
    int multilicheck_repetitions;
    int num_ldt_instances;
    int fri_query_repetitions;
    int fri_interactive_repetitions;
    int fri_num_reductions;
    int max_ldt_tested_degree_bound;
    int fri_final_polynomial_degree_bound;
    size_t fri_localization_parameters[16];
    uint64_t field_zero[5];
    uint64_t field_one[5];
} Parameters;

extern const Parameters preon;

#endif
