#ifndef UTILS_H
#define UTILS_H

#include "params.h"
#include "poly.h"
#include "fips202.h"

#define poly_sample_from_typical_set FULEECA_NAMESPACE(_poly_sample_from_typical_set)
void poly_sample_from_typical_set(poly_n_2 *p, keccak_state *state);

#define extended_euclidean_algorithm FULEECA_NAMESPACE(_extended_euclidean_algorithm)
void extended_euclidean_algorithm(poly_n_2 *a_inv, poly_n_2 *a);

#define rel_key_address FULEECA_NAMESPACE(_rel_key_address)
size_t rel_key_address(size_t key_line, size_t rel_position);

#define simple_sign_score FULEECA_NAMESPACE(_simple_sign_score)
void simple_sign_score(int *score_per_row, const poly_n_2 *a, const poly_n_2 *b, const uint8_t *mhash);

#define calc_matches_and_rel_weight_verify FULEECA_NAMESPACE(_calc_matches_and_rel_weight_verify)
void calc_matches_and_rel_weight_verify(int32_t *lmp, size_t *lee_weight, const poly_n *sig, const uint8_t *mhash);

#define gen_signature_candidate FULEECA_NAMESPACE(_gen_signature_candidate)
void gen_signature_candidate(poly_n *v, const poly_n_2 *a, const poly_n_2 *b, const int *score_per_row);

#define find_improvement FULEECA_NAMESPACE(_find_improvement)
void find_improvement(poly_s_d_n *v, uint8_t *allowed_indexes, const poly_s_n gsec[N/2], const uint8_t *exp_chall, int32_t *return_lmp, const uint8_t loopfree);

#define update_cand FULEECA_NAMESPACE(_update_cand)
void update_cand(poly_s_d_n **v_bar, poly_s_d_n **v_bar_new, int32_t *ret_lmp, int32_t *cand_lmp, const uint8_t *allowed_indexes, const uint8_t loopfree, size_t *blocked_index, size_t mcnt, size_t curr_i, size_t target_i);

#define lmp_score FULEECA_NAMESPACE(_lmp_score)
int32_t lmp_score(size_t mcnt, size_t support_size);

#define random_int_range FULEECA_NAMESPACE(_random_int_range)
uint16_t random_int_range(keccak_state *state, uint16_t max_range);

#define shuffle_array_indices FULEECA_NAMESPACE(_shuffle_array_indices)
void shuffle_array_indices(keccak_state *state, uint16_t* array);

#define calc_key_support FULEECA_NAMESPACE(_calc_key_support)
size_t calc_key_support(const poly_n_2 *a, const poly_n_2 *b);

#define calc_sig_support_poly_n FULEECA_NAMESPACE(_calc_sig_support_poly_n)
size_t calc_sig_support_poly_n(const poly_n *v);

#define calc_sig_support_poly_s_d_n FULEECA_NAMESPACE(_calc_sig_support_poly_s_d_n)
size_t calc_sig_support_poly_s_d_n(const poly_s_d_n *v);

#define expand_key FULEECA_NAMESPACE(_expand_key)
void expand_key(poly_s_n gsec[N/2], const poly_s_n_2 *a, const poly_s_n_2 *b);

#define expand_chall FULEECA_NAMESPACE(_expand_chall)
void expand_chall(uint8_t exp_chall[N], const uint8_t *mhash);

#endif
