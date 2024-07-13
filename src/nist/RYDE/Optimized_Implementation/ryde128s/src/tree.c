/**
 * @file ryde_128s_tree.c
 * @brief Implementation of tree related functions
 */

#include <stdio.h>
#include "tree.h"



/**
 * \fn void ryde_128s_tree_expand(ryde_128s_seed_tree_t tree, const uint8_t *salt)
 * \brief This function compute a seed tree by expanding its master seed.
 *
 * The leaves of the tree are stored in the array at position (tree[RYDE_128S_PARAM_N_MPC - 1])
 *
 * \param[out] tree ryde_128s_seed_tree_t Representation of the tree with master seed at position tree[0]
 * \param[in] salt uint8_t* Salt used for the signature
 * \param[in] e uint8_t Current iteration
 */
#ifdef SHAKE_TIMES4
void ryde_128s_tree_expand(ryde_128s_seed_tree_t tree, const uint8_t *salt, uint8_t e) {
  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for(size_t i = 0; i < 3; i++) {
    size_t from = i;
    size_t to = i * 2 + 1;

    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, salt, 2 * RYDE_128S_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, tree[from], RYDE_128S_SECURITY_BYTES);
    hash_sha3_finalize(tree[to], &ctx);
  }

  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  const uint8_t *e_x4[] = {&e, &e, &e, &e};
  const uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  hash_sha3_x4_ctx ctx_x4;

  for(size_t i = 3; i < (RYDE_128S_PARAM_N_MPC - 1); i+=4) {
    size_t from = i;
    size_t to = i * 2 + 1;

    const uint8_t *tree_from_x4[] = {tree[from + 0], tree[from + 1], tree[from + 2], tree[from + 3]};
    uint8_t *tree_to_times4[] = {tree[to + 0], tree[to + 2], tree[to + 4], tree[to + 6]};

    hash_sha3_x4_init(&ctx_x4);
    hash_sha3_x4_absorb(&ctx_x4, domain_separator_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx_x4, salt_x4, 2 * RYDE_128S_SECURITY_BYTES);
    hash_sha3_x4_absorb(&ctx_x4, e_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx_x4, tree_from_x4, RYDE_128S_SECURITY_BYTES);
    hash_sha3_x4_finalize(tree_to_times4, &ctx_x4);
  }
}
#else
void ryde_128s_tree_expand(ryde_128s_seed_tree_t tree, const uint8_t *salt, uint8_t e) {
  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for(size_t i = 0; i < (RYDE_128S_PARAM_N_MPC - 1); i++) {
    size_t from = i;
    size_t to = i * 2 + 1;

    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, salt, 2 * RYDE_128S_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, tree[from], RYDE_128S_SECURITY_BYTES);
    hash_sha3_finalize(tree[to], &ctx);
  }
}
#endif



/**
 * \fn ryde_128s_tree_expand_partial(ryde_128s_seed_tree_t partial_tree, const ryde_128s_seed_tree_node_t partial_tree_seeds[RYDE_128S_PARAM_N_MPC_LOG2],
 *                                        const uint8_t *salt,  uint16_t alpha)
 *
 * @brief This function expands a partial tree of seeds. The alpha-th leaf is set to zero.
 *
 * @param[out] partial_tree ryde_128s_seed_tree_t Representation of the expanded partial seed tree
 * @param[in]  partial_tree_seeds ryde_128s_seed_tree_node_t Representation of the seeds
 * @param[in]  salt uint8_t* Salt used in the signature
 * \param[in] e uint8_t Current iteration
 * @param[in]  alpha uint16_t Missing leaf
 */
#ifdef SHAKE_TIMES4
void ryde_128s_tree_expand_partial(ryde_128s_seed_tree_t partial_tree, const ryde_128s_seed_tree_node_t partial_tree_seeds[RYDE_128S_PARAM_N_MPC_LOG2],
                                        const uint8_t *salt, uint8_t e, uint16_t alpha) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for (size_t i = 0, l = 0, j = 0; i < 3; i++, j++) {
    size_t N = (1U << l);
    if (j >= N) {  // increment depth
      l++;
      j = 0;
    }
    size_t from = i;
    size_t to = i * 2 + 1;
    size_t missing = (alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - l));            // missing node for the depth l
    size_t is_right = (~alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - 1 - l)) & 1;  // position in the depth l + 1

    if (j == missing) {
      memcpy(partial_tree[to + is_right], partial_tree_seeds[l], RYDE_128S_SECURITY_BYTES);
    } else {
      hash_sha3_init(&ctx);
      hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, salt, 2 * RYDE_128S_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, partial_tree[from], RYDE_128S_SECURITY_BYTES);
      hash_sha3_finalize(partial_tree[to], &ctx);
    }
  }

  hash_sha3_x4_ctx ctx_x4;
  const uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  const uint8_t *e_x4[] = {&e, &e, &e, &e};
  const uint8_t *tree_from_x4[4];
  uint8_t *tree_to_times4[4];
  uint8_t discard_buffer[2 * RYDE_128S_SECURITY_BYTES];

  for(size_t i = 3, l = 1, j = 2; i < (RYDE_128S_PARAM_N_MPC - 1); i++, j++) {
    size_t N = (1U << l);
    if (j >= N) {  // increment depth
      l++;
      j = 0;
    }
    size_t from = i;
    size_t to = i * 2 + 1;

    size_t missing = (alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - l));            // missing node for the depth l
    size_t is_right = (~alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - 1 - l)) & 1;  // position in the depth l + 1
    size_t times4i = (i - 3) & 0x3;
    tree_from_x4[times4i] = partial_tree[from];

    if (j == missing) {
        memcpy(partial_tree[to + is_right], partial_tree_seeds[l], RYDE_128S_SECURITY_BYTES);
        tree_to_times4[times4i] = discard_buffer;
    } else {
        tree_to_times4[times4i] = partial_tree[to];
    }

    if (times4i == 3) {
      hash_sha3_x4_init(&ctx_x4);
      hash_sha3_x4_absorb(&ctx_x4, domain_separator_x4, sizeof(uint8_t));
      hash_sha3_x4_absorb(&ctx_x4, salt_x4, 2 * RYDE_128S_SECURITY_BYTES);
      hash_sha3_x4_absorb(&ctx_x4, e_x4, sizeof(uint8_t));
      hash_sha3_x4_absorb(&ctx_x4, tree_from_x4, RYDE_128S_SECURITY_BYTES);
      hash_sha3_x4_finalize(tree_to_times4, &ctx_x4);
    }
  }
}
#else
void ryde_128s_tree_expand_partial(ryde_128s_seed_tree_t partial_tree, const ryde_128s_seed_tree_node_t partial_tree_seeds[RYDE_128S_PARAM_N_MPC_LOG2],
                                        const uint8_t *salt, uint8_t e, uint16_t alpha) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_TREE;
  hash_sha3_ctx ctx;

  for(size_t i = 0, l = 0, j = 0; i < (RYDE_128S_PARAM_N_MPC - 1); i++, j++) {
    size_t N = (1U << l);
    if (j >= N) {  // increment depth
      l++;
      j = 0;
    }
    size_t from = i;
    size_t to = i * 2 + 1;
    size_t missing = (alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - l));            // missing node for the depth l
    size_t is_right = (~alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - 1 - l)) & 1;  // position in the depth l + 1

    if (j == missing) {
      memcpy(partial_tree[to + is_right], partial_tree_seeds[l], RYDE_128S_SECURITY_BYTES);
    } else {
      hash_sha3_init(&ctx);
      hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, salt, 2 * RYDE_128S_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, partial_tree[from], RYDE_128S_SECURITY_BYTES);
      hash_sha3_finalize(partial_tree[to], &ctx);
    }
  }
}
#endif



/**
 * \fn void ryde_128s_tree_compute_partial(ryde_128s_seed_tree_node_t partial_tree_seeds[RYDE_128S_PARAM_N_MPC_LOG2],
 *                                                const ryde_128s_seed_tree_t tree, const uint16_t alpha)
 *
 * @brief This function returns the seed tree needed to compute all leaves except the alpha-th leaf
 *
 * @param[out] partial_tree_seeds Array of seeds needed to rebuild the tree with the missing alpha leaf
 * \param[in] tree ryde_128s_seed_tree_t Representation of the tree with master seed at position tree[0]
 * @param[in] alpha uint16_t Missing leaf
 */
void ryde_128s_tree_compute_partial(ryde_128s_seed_tree_node_t partial_tree_seeds[RYDE_128S_PARAM_N_MPC_LOG2],
                                           const ryde_128s_seed_tree_t tree, uint16_t alpha) {

    for (size_t i = 0; i < RYDE_128S_PARAM_N_MPC_LOG2; i++) {
        size_t depth = (1U << (i + 1U)) - 1;
        size_t node = (alpha >> (RYDE_128S_PARAM_N_MPC_LOG2 - 1U - i)) ^ 1U;
        memcpy(partial_tree_seeds[i], tree[depth + node], RYDE_128S_SECURITY_BYTES);
    }
}
