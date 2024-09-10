#include "vector_com.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdlib.h>

#include "prgs.h"
#include "small_vole.h"
#include "util.h"

// TODO: probably can ditch most of the "restrict"s in inlined functions.

#define TREE_CHUNK_SIZE (PRG_TREE_PREFERRED_WIDTH / 2)
#define TREE_CHUNK_SIZE_SHIFT (PRG_TREE_PREFERRED_WIDTH_SHIFT - 1)
#define LEAF_CHUNK_SIZE (PRG_LEAF_PREFERRED_WIDTH / 2)
#define LEAF_CHUNK_SIZE_SHIFT (PRG_LEAF_PREFERRED_WIDTH_SHIFT - 1)

#define MAX_CHUNK_SIZE_SHIFT \
	(LEAF_CHUNK_SIZE_SHIFT > TREE_CHUNK_SIZE_SHIFT ? LEAF_CHUNK_SIZE_SHIFT : TREE_CHUNK_SIZE_SHIFT)
#define MAX_CHUNK_SIZE (LEAF_CHUNK_SIZE > TREE_CHUNK_SIZE ? LEAF_CHUNK_SIZE : TREE_CHUNK_SIZE)

static ALWAYS_INLINE void copy_prg_output(
	bool leaf, size_t n, size_t stretch, uint32_t j, uint32_t num_blocks, size_t num_bytes,
	const prg_tree_block* prg_output_tree, const prg_leaf_block* prg_output_leaf,
	block_secpar* output)
{
	size_t prg_block_size = !leaf ? sizeof(prg_tree_block) : sizeof(prg_leaf_block);
	for (size_t k = 0; k < n; ++k)
		memcpy(((unsigned char*) &output[stretch * k]) + j * prg_block_size,
		       !leaf ? (void*) &prg_output_tree[num_blocks * k]
		             : (void*) &prg_output_leaf[num_blocks * k], num_bytes);
}

// Take each of n block_secpars from input and expand it into stretch adjacent blocks in output.
// fixed_key_tree, fixed_key_leaf is only used for PRGs based on fixed-key Rijndael. Works for
// n <= TREE_CHUNK_SIZE (or LEAF_CHUNK_SIZE if leaf).
static ALWAYS_INLINE void expand_chunk(
	bool leaf, size_t n, uint32_t stretch, block128 iv,
	const prg_tree_fixed_key* restrict fixed_key_tree,
	const prg_leaf_fixed_key* restrict fixed_key_leaf,
	const block_secpar* restrict input, block_secpar* restrict output)
{
	assert(n <= (!leaf ? TREE_CHUNK_SIZE : LEAF_CHUNK_SIZE));

	block_secpar keys[MAX_CHUNK_SIZE];
	prg_tree_iv ivs_tree[TREE_CHUNK_SIZE];
	prg_leaf_iv ivs_leaf[LEAF_CHUNK_SIZE];
	prg_tree_key prgs_tree[TREE_CHUNK_SIZE];
	prg_leaf_key prgs_leaf[LEAF_CHUNK_SIZE];
	prg_tree_block prg_output_tree[TREE_CHUNK_SIZE * 3];
	prg_leaf_block prg_output_leaf[LEAF_CHUNK_SIZE * 3];

	memcpy(&keys[0], input, n * sizeof(block_secpar));

	for (size_t i = 0; i < n; ++i)
	{
		memcpy(&ivs_tree[i], &iv, sizeof(ivs_tree[i]));
		memcpy(&ivs_leaf[i], &iv, sizeof(ivs_leaf[i]));
	}

	size_t prg_block_size = !leaf ? sizeof(prg_tree_block) : sizeof(prg_leaf_block);
	uint32_t blocks_per_key = (stretch * sizeof(block_secpar) + prg_block_size - 1) / prg_block_size;
	size_t bytes_extra_per_key = blocks_per_key * prg_block_size - stretch * sizeof(block_secpar);

	assert(blocks_per_key >= 2);
	uint32_t num_blocks = blocks_per_key % 2 ? 3 : 2;
	if (!leaf)
		prg_tree_init(&prgs_tree[0], fixed_key_tree, &keys[0], &ivs_tree[0],
		              n, num_blocks, 0, &prg_output_tree[0]);
	else
		prg_leaf_init(&prgs_leaf[0], fixed_key_leaf, &keys[0], &ivs_leaf[0],
		              n, num_blocks, 0, &prg_output_leaf[0]);

	assert(blocks_per_key > num_blocks || bytes_extra_per_key == 0);
	copy_prg_output(leaf, n, stretch, 0, num_blocks, num_blocks * prg_block_size,
	                prg_output_tree, prg_output_leaf, output);

	for (uint32_t j = num_blocks; j < blocks_per_key; j += num_blocks)
	{
		num_blocks = 2;
		if (!leaf)
			prg_tree_gen(&prgs_tree[0], fixed_key_tree, n, num_blocks, j, &prg_output_tree[0]);
		else
			prg_leaf_gen(&prgs_leaf[0], fixed_key_leaf, n, num_blocks, j, &prg_output_leaf[0]);

		if (j + num_blocks < blocks_per_key || bytes_extra_per_key == 0)
			copy_prg_output(leaf, n, stretch, j, num_blocks, num_blocks * prg_block_size,
			                prg_output_tree, prg_output_leaf, output);
		else
			copy_prg_output(leaf, n, stretch, j, num_blocks,
			                num_blocks * prg_block_size - bytes_extra_per_key,
			                prg_output_tree, prg_output_leaf, output);

	}
}

// Allow n to be hardcoded by the compiler into expand_chunk:
#define DEF_EXPAND_CHUNK_N(n) \
	static void expand_chunk_n_##n( \
		block128 iv, const prg_tree_fixed_key* restrict fixed_key_tree, \
		const block_secpar* restrict input, block_secpar* restrict output) \
	{ \
		if (n <= TREE_CHUNK_SIZE) \
			expand_chunk(false, n, 2, iv, fixed_key_tree, NULL, input, output); \
	}

// Most of these will be unused, and so removed by the compiler.
static_assert(TREE_CHUNK_SIZE <= 32, "");
DEF_EXPAND_CHUNK_N(1)
DEF_EXPAND_CHUNK_N(2)
DEF_EXPAND_CHUNK_N(3)
DEF_EXPAND_CHUNK_N(4)
DEF_EXPAND_CHUNK_N(5)
DEF_EXPAND_CHUNK_N(6)
DEF_EXPAND_CHUNK_N(7)
DEF_EXPAND_CHUNK_N(8)
DEF_EXPAND_CHUNK_N(9)
DEF_EXPAND_CHUNK_N(10)
DEF_EXPAND_CHUNK_N(11)
DEF_EXPAND_CHUNK_N(12)
DEF_EXPAND_CHUNK_N(13)
DEF_EXPAND_CHUNK_N(14)
DEF_EXPAND_CHUNK_N(15)
DEF_EXPAND_CHUNK_N(16)
DEF_EXPAND_CHUNK_N(17)
DEF_EXPAND_CHUNK_N(18)
DEF_EXPAND_CHUNK_N(19)
DEF_EXPAND_CHUNK_N(20)
DEF_EXPAND_CHUNK_N(21)
DEF_EXPAND_CHUNK_N(22)
DEF_EXPAND_CHUNK_N(23)
DEF_EXPAND_CHUNK_N(24)
DEF_EXPAND_CHUNK_N(25)
DEF_EXPAND_CHUNK_N(26)
DEF_EXPAND_CHUNK_N(27)
DEF_EXPAND_CHUNK_N(28)
DEF_EXPAND_CHUNK_N(29)
DEF_EXPAND_CHUNK_N(30)
DEF_EXPAND_CHUNK_N(31)
DEF_EXPAND_CHUNK_N(32)

// Use a switch to select which size. The case should always be resolved at compile time. This is
// just a way to get the compiler to select the right function to call.
static ALWAYS_INLINE void expand_chunk_switch( \
	size_t n, block128 iv, const prg_tree_fixed_key* restrict fixed_key_tree,
	const block_secpar* restrict input, block_secpar* restrict output) \
{ \
	switch (n)
	{
#define EXPAND_CHUNK_SWITCH_CASE(n) \
	case n: \
		expand_chunk_n_##n(iv, fixed_key_tree, input, output); \
		break;
		EXPAND_CHUNK_SWITCH_CASE(1)
		EXPAND_CHUNK_SWITCH_CASE(2)
		EXPAND_CHUNK_SWITCH_CASE(3)
		EXPAND_CHUNK_SWITCH_CASE(4)
		EXPAND_CHUNK_SWITCH_CASE(5)
		EXPAND_CHUNK_SWITCH_CASE(6)
		EXPAND_CHUNK_SWITCH_CASE(7)
		EXPAND_CHUNK_SWITCH_CASE(8)
		EXPAND_CHUNK_SWITCH_CASE(9)
		EXPAND_CHUNK_SWITCH_CASE(10)
		EXPAND_CHUNK_SWITCH_CASE(11)
		EXPAND_CHUNK_SWITCH_CASE(12)
		EXPAND_CHUNK_SWITCH_CASE(13)
		EXPAND_CHUNK_SWITCH_CASE(14)
		EXPAND_CHUNK_SWITCH_CASE(15)
		EXPAND_CHUNK_SWITCH_CASE(16)
		EXPAND_CHUNK_SWITCH_CASE(17)
		EXPAND_CHUNK_SWITCH_CASE(18)
		EXPAND_CHUNK_SWITCH_CASE(19)
		EXPAND_CHUNK_SWITCH_CASE(20)
		EXPAND_CHUNK_SWITCH_CASE(21)
		EXPAND_CHUNK_SWITCH_CASE(22)
		EXPAND_CHUNK_SWITCH_CASE(23)
		EXPAND_CHUNK_SWITCH_CASE(24)
		EXPAND_CHUNK_SWITCH_CASE(25)
		EXPAND_CHUNK_SWITCH_CASE(26)
		EXPAND_CHUNK_SWITCH_CASE(27)
		EXPAND_CHUNK_SWITCH_CASE(28)
		EXPAND_CHUNK_SWITCH_CASE(29)
		EXPAND_CHUNK_SWITCH_CASE(30)
		EXPAND_CHUNK_SWITCH_CASE(31)
		EXPAND_CHUNK_SWITCH_CASE(32)
	default:
		assert(0 < n && n <= 32);
	}
}

static void expand_chunk_leaf_n_leaf_chunk_size(
	block128 iv, const prg_leaf_fixed_key* restrict fixed_key_leaf,
	const block_secpar* restrict input, block_secpar* restrict output)
{
	expand_chunk(true, LEAF_CHUNK_SIZE, 3, iv, NULL, fixed_key_leaf, input, output);
}

#define TREES_IN_FOREST(verifier) ((verifier) ? SECURITY_PARAM : BITS_PER_WITNESS)
#define PARENT(x, verifier) (((x) - TREES_IN_FOREST(verifier)) / 2)
#define FIRST_CHILD(x, verifier) (2 * (x) + TREES_IN_FOREST(verifier))

// Equivalent to FIRST_CHILD(..., verifier) iterated d times, starting from x.
#define FIRST_DESCENDENT_DEPTH(x, d, verifier) \
	((((x) + TREES_IN_FOREST(verifier)) << (d)) - TREES_IN_FOREST(verifier))

// Duplicate the same function many times for recursion, so that it will all get inlined.
#define EXPAND_ROOTS_RECURSION(n, next) \
	static ALWAYS_INLINE void expand_roots_##n( \
		bool partial, block128 iv, const prg_tree_fixed_key* restrict fixed_key_tree, \
		block_secpar* restrict forest, size_t i) \
	{ \
		if (n >= TREE_CHUNK_SIZE_SHIFT) \
			return; \
		size_t this_chunk_size = partial ? TREES_IN_FOREST(false) % TREE_CHUNK_SIZE : TREE_CHUNK_SIZE; \
		expand_chunk_switch(this_chunk_size, iv, fixed_key_tree, &forest[i], &forest[FIRST_CHILD(i, false)]); \
		next(partial, iv, fixed_key_tree, forest, FIRST_CHILD(i, false)); \
		next(partial, iv, fixed_key_tree, forest, FIRST_CHILD(i, false) + this_chunk_size); \
	}
#define FINISHED_RECURSION(a,b,c,d,e) do {} while (0)

static_assert(TREE_CHUNK_SIZE_SHIFT <= 5, "");
EXPAND_ROOTS_RECURSION(4, FINISHED_RECURSION)
EXPAND_ROOTS_RECURSION(3, expand_roots_4)
EXPAND_ROOTS_RECURSION(2, expand_roots_3)
EXPAND_ROOTS_RECURSION(1, expand_roots_2)
EXPAND_ROOTS_RECURSION(0, expand_roots_1)
#undef FINISHED_RECURSION

static void write_leaves(
	block128 iv, const prg_leaf_fixed_key* restrict fixed_key_leaf,
	block_secpar* restrict starting_node, size_t starting_leaf_idx, size_t* permuted_leaf_idx,
	block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves)
{
	size_t perm_leaf_idx = *permuted_leaf_idx;
	size_t leaf_idx = starting_leaf_idx;

	for (size_t j = 0; j < MAX_CHUNK_SIZE; j += LEAF_CHUNK_SIZE)
	{
		block_secpar prg_output[3 * LEAF_CHUNK_SIZE];
		expand_chunk_leaf_n_leaf_chunk_size(iv, fixed_key_leaf, &starting_node[j], prg_output);

		for (size_t k = 0; k < LEAF_CHUNK_SIZE; k += VOLE_WIDTH)
		{
			// Simplest to compute permuted_leaf_idx in each iteration, but
			// vole_permute_key_index leaves the last VOLE_WIDTH_SHIFT bits unchanged, so it
			// only needs to be called once every VOLE_WIDTH blocks.
			for (size_t l = 0; l < VOLE_WIDTH && l < LEAF_CHUNK_SIZE; l++)
			{
				leaves[perm_leaf_idx] = prg_output[3 * (k + l)];
				memcpy(&hashed_leaves[leaf_idx], &prg_output[3 * (k + l) + 1], sizeof(block_2secpar));
				perm_leaf_idx ^= vole_permute_inv_increment(leaf_idx, 1);
				leaf_idx++;
			}
		}
	}

	*permuted_leaf_idx = perm_leaf_idx;
}

static ALWAYS_INLINE void expand_tree(
	bool verifier, size_t delta, block128 iv, const prg_tree_fixed_key* restrict fixed_key_tree,
	const prg_leaf_fixed_key* restrict fixed_key_leaf, block_secpar* restrict forest,
	unsigned int height, size_t root, size_t starting_leaf_idx,
	block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves)
{
	size_t permuted_leaf_idx = vole_permute_key_index_inv(starting_leaf_idx ^ delta);

	// Loop over blocks of size max(TREE_CHUNK_SIZE, LEAF_CHUNK_SIZE).
	size_t pow_height = (size_t) 1 << height;
	for (size_t chunk = 0; chunk < pow_height;
	     chunk += MAX_CHUNK_SIZE / TREE_CHUNK_SIZE, starting_leaf_idx += TREE_CHUNK_SIZE)
	{
		size_t ancestor;
		for (size_t i = chunk; i < chunk + (MAX_CHUNK_SIZE / TREE_CHUNK_SIZE) && i < pow_height; ++i)
		{
			unsigned int ancestor_height = count_trailing_zeros(i | pow_height);
			unsigned int ancestor_depth = height - ancestor_height;

			ancestor = FIRST_DESCENDENT_DEPTH(root, ancestor_depth, verifier) +
				TREE_CHUNK_SIZE * (i >> ancestor_height);

			for (int d = ancestor_height - 1; d >= 0; --d)
			{
				size_t first_child = FIRST_CHILD(ancestor, verifier);
				expand_chunk_switch(TREE_CHUNK_SIZE, iv, fixed_key_tree,
				                    &forest[ancestor], &forest[first_child]);
				ancestor = first_child;
			}
		}

		if (chunk + (MAX_CHUNK_SIZE / TREE_CHUNK_SIZE) <= pow_height)
		{
			// We've just finished a block of size MAX_CHUNK_SIZE (at least LEAF_CHUNK_SIZE), so
			// apply the leaf prgs and write to leaves and hashed_leaves.
			size_t starting_node = ancestor + TREE_CHUNK_SIZE - MAX_CHUNK_SIZE;
			write_leaves(iv, fixed_key_leaf, &forest[starting_node], starting_leaf_idx,
			             &permuted_leaf_idx, leaves, hashed_leaves);
		}
	}
}

void vector_commit(
	const block_secpar seed, block128 iv,
	block_secpar* restrict forest, block_secpar* restrict leaves,
	block_2secpar* restrict hashed_leaves)
{
	block_secpar fixed_key_iv = block_secpar_set_zero();
	memcpy(&fixed_key_iv, &iv, sizeof(iv));
	prg_tree_fixed_key fixed_key_tree;
	prg_leaf_fixed_key fixed_key_leaf;
	init_fixed_keys(&fixed_key_tree, &fixed_key_leaf, fixed_key_iv);

	block_secpar roots[BITS_PER_WITNESS];
	expand_chunk(false, 1, BITS_PER_WITNESS, iv, &fixed_key_tree, NULL, &seed, &roots[0]);
	vector_commit_from_roots(roots, iv, forest, leaves, hashed_leaves,
	                         &fixed_key_tree, &fixed_key_leaf);
}

void vector_commit_from_roots(
	block_secpar* roots, block128 iv, block_secpar* restrict forest,
	block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves,
	const prg_tree_fixed_key* fixed_key_tree, const prg_leaf_fixed_key* fixed_key_leaf)
{
	memcpy(forest, roots, TREES_IN_FOREST(false) * sizeof(block_secpar));

	// First expand each tree far enough to have TREE_CHUNK_SIZE nodes.
	static_assert(VOLE_MIN_K >= TREE_CHUNK_SIZE_SHIFT, "");
	for (size_t i = 0; i + TREE_CHUNK_SIZE <= TREES_IN_FOREST(false); i += TREE_CHUNK_SIZE)
		expand_roots_0(false, iv, fixed_key_tree, forest, i);
	size_t remaining = TREES_IN_FOREST(false) % TREE_CHUNK_SIZE;
	if (remaining)
		expand_roots_0(true, iv, fixed_key_tree, forest, TREES_IN_FOREST(false) - remaining);

	// Expand each tree, now that they are each 1 chunk in size.
	for (size_t i = 0; i < BITS_PER_WITNESS; ++i)
	{
		// First VOLES_MAX_K trees are 1 taller.
		unsigned int height = i < VOLES_MAX_K ? VOLE_MAX_K : VOLE_MIN_K;

		size_t root = FIRST_DESCENDENT_DEPTH(i, TREE_CHUNK_SIZE_SHIFT, false);
		expand_tree(false, 0, iv, fixed_key_tree, fixed_key_leaf, forest,
		            height - TREE_CHUNK_SIZE_SHIFT, root, 0, leaves, hashed_leaves);

		size_t pow_height = (size_t) 1 << height;
		leaves += pow_height;
		hashed_leaves += pow_height;
	}
}

void vector_open(
	const block_secpar* restrict forest, const block_2secpar* restrict hashed_leaves,
	const uint8_t* restrict delta, unsigned char* restrict opening)
{
	for (size_t i = 0; i < BITS_PER_WITNESS; ++i)
	{
		unsigned int depth = i < VOLES_MAX_K ? VOLE_MAX_K : VOLE_MIN_K;
		size_t node = FIRST_CHILD(i, false);
		size_t leaf_idx = 0;
		for (unsigned int d = 1; d <= depth; ++d)
		{
			unsigned int hole = delta[depth - d] & 1;
			leaf_idx = 2*leaf_idx + hole;

			memcpy(opening, &forest[node + (1 - hole)], sizeof(block_secpar));
			opening += sizeof(block_secpar);
			node = FIRST_CHILD(node + hole, false);
		}

		memcpy(opening, &hashed_leaves[leaf_idx], sizeof(block_2secpar));
		opening += sizeof(block_2secpar);

		delta += depth;
		hashed_leaves += (1 << depth);
	}
}

#define EXPAND_VERIFIER_SUBTREES_RECURSION(n, next) \
	static ALWAYS_INLINE void expand_verifier_subtrees_##n( \
		size_t this_chunk_size, block128 iv, const prg_tree_fixed_key* restrict fixed_key_tree, \
		block_secpar* restrict forest, size_t i, size_t node) \
	{ \
		if (n >= TREE_CHUNK_SIZE_SHIFT) \
			return; \
		size_t level_lim = (TREES_IN_FOREST(true) - (n + 1) * BITS_PER_WITNESS) << n; \
		if (i >= level_lim) \
			return; \
		if (i + this_chunk_size > level_lim) \
		{ \
			this_chunk_size = level_lim % TREE_CHUNK_SIZE; \
			assert(this_chunk_size == level_lim - i); /* All preceding calls were on whole chunks. */ \
			size_t child = FIRST_CHILD(node, true); \
			expand_chunk_switch(this_chunk_size, iv, fixed_key_tree, &forest[node], &forest[child]); \
			size_t output_size = 2 * this_chunk_size; \
			i *= 2; \
			if (output_size >= TREE_CHUNK_SIZE) \
			{ \
				next(TREE_CHUNK_SIZE, iv, fixed_key_tree, forest, i, child); \
				i += TREE_CHUNK_SIZE; \
				child += TREE_CHUNK_SIZE; \
				output_size -= TREE_CHUNK_SIZE; \
			} \
			next(output_size, iv, fixed_key_tree, forest, i, child); \
		} \
		else \
		{ \
			/* this_chunk_size must be TREE_CHUNK_SIZE here, because at most the last call is not on a */ \
			/* whole chunk, and that call will be the one that hits the level_lim limit above. */ \
			size_t child = FIRST_CHILD(node, true); \
			expand_chunk_switch(TREE_CHUNK_SIZE, iv, fixed_key_tree, &forest[node], &forest[child]); \
			next(TREE_CHUNK_SIZE, iv, fixed_key_tree, forest, 2*i, child); \
			next(TREE_CHUNK_SIZE, iv, fixed_key_tree, forest, 2*i + TREE_CHUNK_SIZE, child + TREE_CHUNK_SIZE); \
		} \
	}
#define FINISHED_RECURSION(a,b,c,d,e,f) do {} while (0)

static_assert(TREE_CHUNK_SIZE_SHIFT <= 5, "");
EXPAND_VERIFIER_SUBTREES_RECURSION(4, FINISHED_RECURSION)
EXPAND_VERIFIER_SUBTREES_RECURSION(3, expand_verifier_subtrees_4)
EXPAND_VERIFIER_SUBTREES_RECURSION(2, expand_verifier_subtrees_3)
EXPAND_VERIFIER_SUBTREES_RECURSION(1, expand_verifier_subtrees_2)
EXPAND_VERIFIER_SUBTREES_RECURSION(0, expand_verifier_subtrees_1)
#undef FINISHED_RECURSION

// Reorder the keys in the opening, to group them by height.
static ALWAYS_INLINE void reorder_verifier_keys(
	const unsigned char* opening, block_secpar* reordered_keys)
{
	block_secpar* dst = reordered_keys;
	for (size_t i = 0; i < VOLE_MAX_K; ++i)
	{
		size_t src_idx = i;
		for (size_t j = 0; j < VOLES_MAX_K; ++j, ++dst, src_idx += VOLE_MAX_K + 2)
			memcpy(dst, &opening[src_idx * sizeof(block_secpar)], sizeof(block_secpar));
		if (i >= VOLE_MAX_K - VOLE_MIN_K)
		{
			src_idx -= VOLE_MAX_K - VOLE_MIN_K;
			for (size_t j = 0; j < VOLES_MIN_K; ++j, ++dst, src_idx += VOLE_MIN_K + 2)
				memcpy(dst, &opening[src_idx * sizeof(block_secpar)], sizeof(block_secpar));
		}
	}
}

void vector_verify(
	block128 iv, const unsigned char* restrict opening, const uint8_t* restrict delta,
	block_secpar* restrict leaves, block_2secpar* restrict hashed_leaves)
{
	block_secpar fixed_key_iv = block_secpar_set_zero();
	memcpy(&fixed_key_iv, &iv, sizeof(iv));
	prg_tree_fixed_key fixed_key_tree;
	prg_leaf_fixed_key fixed_key_leaf;
	init_fixed_keys(&fixed_key_tree, &fixed_key_leaf, fixed_key_iv);

	block_secpar* verifier_subtrees = aligned_alloc(
		alignof(block_secpar),
		TREES_IN_FOREST(true) * ((1 << VOLE_MAX_K) - 1) * sizeof(block_secpar));

	// Need the keys in transposed order.
	reorder_verifier_keys(opening, verifier_subtrees);

	// Expand all subtrees from opening to depth TREE_CHUNK_SIZE_SHIFT, except for the ones too close to
	// the leaves, which get expanded fewer times. Splitting it up by d like this isn't necessary,
	// as the expand_verifier_subtrees_* functions already take case of splitting it up. It just
	// helps the compiler to see what things are constant.
	size_t i = 0;
	#ifdef __GNUC__
	#pragma GCC unroll (5)
	#endif
	for (unsigned int d = TREE_CHUNK_SIZE_SHIFT; d > 0; --d)
	{
		// Expand subtrees that fully use depth d.
		size_t end = TREES_IN_FOREST(true) - d * BITS_PER_WITNESS;
		for (; (i + TREE_CHUNK_SIZE) <= end; i += TREE_CHUNK_SIZE)
			expand_verifier_subtrees_0(TREE_CHUNK_SIZE, iv, &fixed_key_tree, verifier_subtrees, i, i);

		// Expand the subtree that partially reaches depth d, but also has parts that only reach d-1
		// or less. But only if this subtree exists. Again, this is separated so that the compiler
		// can see that take advantage of constants.
		if (end % TREE_CHUNK_SIZE != 0)
		{
			assert(i == end - (end % TREE_CHUNK_SIZE));
			expand_verifier_subtrees_0(TREE_CHUNK_SIZE, iv, &fixed_key_tree, verifier_subtrees, i, i);
			i += TREE_CHUNK_SIZE;
		}
	}

	// Fully expand each subtree.
	for (i = 0; i < BITS_PER_WITNESS; ++i)
	{
		unsigned int tree_depth = i < VOLES_MAX_K ? VOLE_MAX_K : VOLE_MIN_K;
		size_t root = i + (i < VOLES_MAX_K ? 0 : VOLES_MAX_K);

		size_t this_delta = 0;
		for (unsigned int d = 1; d <= tree_depth; ++d)
			this_delta = 2*this_delta + (delta[tree_depth - d] & 1);

		block_secpar last_chunk[MAX_CHUNK_SIZE];
		for (unsigned int j = 0; j < tree_depth; ++j)
		{
			unsigned int height = tree_depth - 1 - j;
			size_t pow_height = (size_t) 1 << height;
			size_t leaf_index = (this_delta & -pow_height) ^ pow_height;

			// Expand the rest of this tree, and hash the leaf nodes if there are at least
			// LEAF_CHUNK_SIZE of them.
			if (height >= TREE_CHUNK_SIZE_SHIFT)
			{
				size_t tree_chunk_root = FIRST_DESCENDENT_DEPTH(root, TREE_CHUNK_SIZE_SHIFT, true);
				expand_tree(true, this_delta, iv, &fixed_key_tree, &fixed_key_leaf, verifier_subtrees,
				            height - TREE_CHUNK_SIZE_SHIFT, tree_chunk_root, leaf_index, leaves, hashed_leaves);
			}

			if (height < MAX_CHUNK_SIZE_SHIFT)
				memcpy(&last_chunk[leaf_index % MAX_CHUNK_SIZE],
				       &verifier_subtrees[FIRST_DESCENDENT_DEPTH(root, height, true)],
				       sizeof(block_secpar) << height);

			if (i < VOLES_MAX_K && j == 0)
				root += VOLES_MAX_K;
			else
				root += BITS_PER_WITNESS;
		}

		// There's 1 leaf node we cannot compute. At least stop it from being uninitialized memory.
		memset(&last_chunk[this_delta % MAX_CHUNK_SIZE], 0, sizeof(block_secpar));

		// Hash the 1 remaining MAX_CHUNK_SIZE sized chunk of the tree, which didn't get hashed
		// because it was too small.
		size_t starting_leaf_idx = this_delta - (this_delta % MAX_CHUNK_SIZE);
		size_t permuted_leaf_idx = vole_permute_key_index_inv(starting_leaf_idx ^ this_delta);
		write_leaves(iv, &fixed_key_leaf, last_chunk, starting_leaf_idx, &permuted_leaf_idx, leaves, hashed_leaves);

		// Currently leaves[0] and hashed_leaves[this_delta] contain garbage (specifically, PRG(0)),
		// because we don't know the keys on the active path. Fix them up.
		memset(&leaves[0], 0, sizeof(block_secpar));
		memcpy(&hashed_leaves[this_delta], opening + tree_depth * sizeof(block_secpar), sizeof(block_2secpar));

		size_t pow_tree_depth = (size_t) 1 << tree_depth;
		leaves += pow_tree_depth;
		hashed_leaves += pow_tree_depth;
		opening += (tree_depth + 2) * sizeof(block_secpar);
		delta += tree_depth;
	}

	free(verifier_subtrees);
}
