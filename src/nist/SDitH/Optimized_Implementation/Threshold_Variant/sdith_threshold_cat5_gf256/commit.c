#include "commit.h"
#include "hash.h"

// Commitment x1
static void commit_string(uint8_t* digest, const uint8_t* str, size_t len, const uint8_t* salt, uint16_t e, uint16_t i) {
    hash_context ctx;
    hash_init_prefix(&ctx, HASH_PREFIX_COMMITMENT);
    hash_update(&ctx, salt, PARAM_SALT_SIZE);
    hash_update_uint16_le(&ctx, e);
    hash_update_uint16_le(&ctx, i);
    hash_update(&ctx, str, len);
    hash_final(&ctx, digest);
}

void commit_share(uint8_t* digest, const mpc_share_t* share, const uint8_t* salt, uint16_t e, uint16_t i) {
    commit_string(digest, (uint8_t*) share, PARAM_SHARE_SIZE, salt, e, i);
}

void commit_seed(uint8_t* digest, const uint8_t* seed, const uint8_t* salt, uint16_t e, uint16_t i) {
    commit_string(digest, seed, PARAM_SEED_SIZE, salt, e, i);
}

void commit_seed_and_aux(uint8_t* digest, const uint8_t* seed, const mpc_wit_t* wit, const mpc_corr_t* corr, const uint8_t* salt, uint16_t e, uint16_t i) {
    hash_context ctx;
    hash_init_prefix(&ctx, HASH_PREFIX_COMMITMENT);
    hash_update(&ctx, salt, PARAM_SALT_SIZE);
    hash_update_uint16_le(&ctx, e);
    hash_update_uint16_le(&ctx, i);
    hash_update(&ctx, seed, PARAM_SEED_SIZE);
    hash_update(&ctx, (uint8_t*) wit, PARAM_WIT_SIZE);
    hash_update(&ctx, (uint8_t*) corr, PARAM_CORR_SIZE);
    hash_final(&ctx, digest);
}


// Commitment x4
static void commit_string_x4(uint8_t** digest, uint8_t const* const* str, size_t len, const uint8_t* salt, uint16_t e, const uint16_t* i) {
    hash_context_x4 ctx;
    hash_init_prefix_x4(&ctx, HASH_PREFIX_COMMITMENT);
    hash_update_x4_1(&ctx, salt, PARAM_SALT_SIZE);
    hash_update_x4_uint16_le(&ctx, e);
    hash_update_x4_uint16s_le(&ctx, i);
    hash_update_x4(&ctx, str, len);
    hash_final_x4(&ctx, digest);
}

void commit_share_x4(uint8_t** digest,  mpc_share_t const* const* share, const uint8_t* salt, uint16_t e, const uint16_t* i) {
    commit_string_x4(digest, (uint8_t const* const*) share, sizeof(mpc_share_t), salt, e, i);
}

void commit_seed_x4(uint8_t** digest, uint8_t const* const* seed, const uint8_t* salt, uint16_t e, const uint16_t* i) {
    commit_string_x4(digest, seed, PARAM_SEED_SIZE, salt, e, i);
}

void commit_seed_and_aux_x4(uint8_t** digest, uint8_t const* const* seed, mpc_wit_t const* const* wit, mpc_corr_t const* const* corr, const uint8_t* salt, uint16_t e, const uint16_t* i) {
    hash_context_x4 ctx;
    hash_init_prefix_x4(&ctx, HASH_PREFIX_COMMITMENT);
    hash_update_x4_1(&ctx, salt, PARAM_SALT_SIZE);
    hash_update_x4_uint16_le(&ctx, e);
    hash_update_x4_uint16s_le(&ctx, i);
    hash_update_x4(&ctx, seed, PARAM_SEED_SIZE);
    hash_update_x4(&ctx, (uint8_t const* const*) wit, PARAM_WIT_SIZE);
    hash_update_x4(&ctx, (uint8_t const* const*) corr, PARAM_CORR_SIZE);
    hash_final_x4(&ctx, digest);
}
