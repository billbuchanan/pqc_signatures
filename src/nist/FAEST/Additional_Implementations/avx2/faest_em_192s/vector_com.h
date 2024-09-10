#ifndef VECTOR_COM_H
#define VECTOR_COM_H

#include "config.h"
#include "aes.h"
#include "block.h"
#include "vole_params.h"

#define VECTOR_COMMIT_LEAVES ((VOLES_MIN_K << VOLE_MIN_K) + (VOLES_MAX_K << VOLE_MAX_K))
#define VECTOR_COMMIT_NODES (2 * VECTOR_COMMIT_LEAVES - BITS_PER_WITNESS)
#define VECTOR_OPEN_BITS (SECURITY_PARAM * SECURITY_PARAM + 2 * SECURITY_PARAM * BITS_PER_WITNESS)
#define VECTOR_OPEN_SIZE (VECTOR_OPEN_BITS / 8)

// The GGM trees are all expanded from seed. leaves and hashed_leaves must each be
// VECTOR_COMMIT_LEAVES blocks long. forest must be VECTOR_COMMIT_NODES blocks long. leaves (but not
// hashed_leaves) will be permuted according to vole_permute_key_index.
void vector_commit(
	const block_secpar seed, block128 iv,
	block_secpar* restrict forest, block_secpar* restrict leaves,
	block_2secpar* restrict hashed_leaves);

/// Create vector commitments given the roots instead of deriving the roots from a seed.
/// Same interface as `vector_commit`, except that
/// - `roots` is of length `BITS_PER_WITNESS`
/// - `fixed_key_tree` and `fixed_key_leaf` are initialized (if used)
void vector_commit_from_roots(
    block_secpar* roots, block128 iv, block_secpar* restrict forest,
    block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves,
    const prg_tree_fixed_key* fixed_key_tree, const prg_leaf_fixed_key* fixed_key_leaf);

// Using decommitment data from vector_commit, open at delta. delta is represented as SECURITY_PARAM
// bytes, each 0 or 0xff, with each segment (corresponding to a single VOLE) ordered in little
// endian. opening must be VECTOR_OPEN_SIZE bytes long.
void vector_open(
	const block_secpar* restrict forest, const block_2secpar* restrict hashed_leaves,
	const uint8_t* restrict delta, unsigned char* restrict opening);

// Given an opening, get all but one of the leaves and all of the hashed leaves. The hashed_leaves
// must be verified against the output from vector_commit. leaves will be permuted according to
// delta first, then vole_permute_key_index.
void vector_verify(
	block128 iv, const unsigned char* restrict opening, const uint8_t* restrict delta,
	block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves);

#endif
