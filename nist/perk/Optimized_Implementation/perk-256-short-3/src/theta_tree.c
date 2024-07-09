
/**
 * @file theta_tree.c
 * @brief Implementation of tree related functions
 */

#include "theta_tree.h"
#include "symmetric_times4.h"

void sig_perk_expand_theta_tree(salt_t salt, perk_theta_seeds_tree_t theta_tree) {
    sig_perk_hash_state_t state;
    sig_perk_hash_times4_state_t state_times4;
    unsigned i;

    for (i = 0; i < 3; i++) {
        unsigned from = i;
        uint8_t idx = i;
        unsigned to = i * 2 + 1;
        sig_perk_hash_init(&state, salt, &idx, NULL);
        sig_perk_hash_update(&state, theta_tree[from], sizeof(theta_t));
        sig_perk_hash_final(&state, theta_tree[to], H3);
    }
    for (; i < (PARAM_N - 1); i += 4) {
        unsigned from = i;
        uint8_t idx4[4] = {i + 0, i + 1, i + 2, i + 3};
        unsigned to = i * 2 + 1;
        const uint8_t *theta_tree_from_times4[] = {theta_tree[from + 0], theta_tree[from + 1], theta_tree[from + 2],
                                                   theta_tree[from + 3]};
        uint8_t *theta_tree_to_times4[] = {theta_tree[to + 0], theta_tree[to + 2], theta_tree[to + 4],
                                           theta_tree[to + 6]};
        sig_perk_hash_times4_init(&state_times4, salt, idx4, NULL);
        sig_perk_hash_times4_update(&state_times4, theta_tree_from_times4, sizeof(theta_t));
        sig_perk_hash_times4_final(&state_times4, theta_tree_to_times4, H3);
    }
}

void sig_perk_expand_theta_partial_tree(salt_t salt, perk_theta_seeds_tree_t partial_theta_tree,
                                        const theta_t partial_tree_seeds[THETA_TREE_LEVELS], const uint16_t alpha) {
    sig_perk_hash_state_t state;
    for (unsigned i = 0, l = 0, j = 0; i < 3; i++, j++) {
        unsigned N = (1U << l);
        if (j >= N) {  // increment level
            l++;
            j = 0;
        }
        unsigned from = i;
        uint8_t idx = i;
        unsigned to = i * 2 + 1;
        unsigned missing = (alpha >> (THETA_TREE_LEVELS - l));            // missing node for the level l
        unsigned is_right = (~alpha >> (THETA_TREE_LEVELS - 1 - l)) & 1;  // position in the level l + 1
        if (j == missing) {
            memcpy(partial_theta_tree[to + is_right], partial_tree_seeds[l], sizeof(theta_t));
        } else {
            sig_perk_hash_init(&state, salt, &idx, NULL);
            sig_perk_hash_update(&state, partial_theta_tree[from], sizeof(theta_t));
            sig_perk_hash_final(&state, partial_theta_tree[to], H3);
        }
    }

    sig_perk_hash_times4_state_t state_times4;
    const uint8_t *theta_tree_from_times4[4];
    uint8_t *theta_tree_to_times4[4];
    uint8_t idx4[4];
    uint8_t discard_buffer[2 * sizeof(theta_t)];
    for (unsigned i = 3, l = 1, j = 2; i < (PARAM_N - 1); i++, j++) {
        unsigned N = (1U << l);
        if (j >= N) {  // increment level
            l++;
            j = 0;
        }
        unsigned from = i;
        unsigned to = i * 2 + 1;
        unsigned missing = (alpha >> (THETA_TREE_LEVELS - l));            // missing node for the level l
        unsigned is_right = (~alpha >> (THETA_TREE_LEVELS - 1 - l)) & 1;  // position in the level l + 1
        unsigned times4i = (i - 3) & 0x3;

        theta_tree_from_times4[times4i] = partial_theta_tree[from];
        idx4[times4i] = i;
        if (j == missing) {
            memcpy(partial_theta_tree[to + is_right], partial_tree_seeds[l], sizeof(theta_t));
            theta_tree_to_times4[times4i] = discard_buffer;

        } else {
            theta_tree_to_times4[times4i] = partial_theta_tree[to];
        }
        if (times4i == 3) {
            sig_perk_hash_times4_init(&state_times4, salt, idx4, NULL);
            sig_perk_hash_times4_update(&state_times4, theta_tree_from_times4, sizeof(theta_t));
            sig_perk_hash_times4_final(&state_times4, theta_tree_to_times4, H3);
        }
    }
}

void sig_perk_get_theta_partial_tree_seeds(theta_t partial_tree_seeds[THETA_TREE_LEVELS],
                                           const perk_theta_seeds_tree_t theta_tree, const uint16_t alpha) {
    for (unsigned i = 0; i < THETA_TREE_LEVELS; i++) {
        unsigned level = (1U << (i + 1U)) - 1;
        unsigned node = (alpha >> (THETA_TREE_LEVELS - 1U - i)) ^ 1U;
        memcpy(partial_tree_seeds[i], theta_tree[level + node], sizeof(theta_t));
    }
}
