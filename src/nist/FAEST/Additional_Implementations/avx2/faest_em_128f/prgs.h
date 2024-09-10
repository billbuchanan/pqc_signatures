#ifndef PRGS_H
#define PRGS_H

#include "aes.h"
#include "hash.h"

#define DEFINE_PRG_AES_CTR(name) \
	typedef aes_round_keys prg_##name##_key; \
	typedef block128 prg_##name##_iv; \
	typedef block128 prg_##name##_block; \
	typedef char prg_##name##_fixed_key; /* Unused. */ \
	/* Initialize num_keys prgs, and generate num_blocks blocks from each. */ \
	static ALWAYS_INLINE void prg_##name##_init( \
		prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		const block_secpar* restrict keys, const prg_##name##_iv* restrict ivs, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		(void) fixed_key; \
		aes_keygen_ctr(prgs, keys, ivs, num_keys, num_blocks, counter, output); \
	} \
	static ALWAYS_INLINE void prg_##name##_gen( \
		const prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		(void) fixed_key; \
		aes_ctr(prgs, num_keys, num_blocks, counter, output); \
	}

#define DEFINE_PRG_RIJNDAEL_FIXED_KEY_CTR(name) \
	typedef block_secpar prg_##name##_key; \
	typedef char prg_##name##_iv; /* Unused. */ \
	typedef block_secpar prg_##name##_block; \
	typedef rijndael_round_keys prg_##name##_fixed_key; \
	/* Initialize num_keys prgs, and generate num_blocks blocks from each. */ \
	static ALWAYS_INLINE void prg_##name##_init( \
		prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		const block_secpar* restrict keys, const prg_##name##_iv* restrict ivs, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		(void) ivs; \
		memcpy(prgs, keys, num_keys * sizeof(keys[0])); \
		rijndael_fixed_key_ctr(fixed_key, prgs, num_keys, num_blocks, counter, output); \
	} \
	static ALWAYS_INLINE void prg_##name##_gen( \
		const prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		rijndael_fixed_key_ctr(fixed_key, prgs, num_keys, num_blocks, counter, output); \
	}

#define DEFINE_PRG_SHAKE(name) \
	typedef char prg_##name##_key; /* Unused. */ \
	typedef block128 prg_##name##_iv; \
	typedef block_secpar prg_##name##_block; \
	typedef char prg_##name##_fixed_key; /* Unused. */ \
	static ALWAYS_INLINE void prg_##name##_init( \
		prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		const block_secpar* restrict keys, const prg_##name##_iv* restrict ivs, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		(void) prgs; \
		(void) fixed_key; \
		(void) counter; \
		assert(counter == 0); \
		shake_prg(keys, ivs, num_keys, num_blocks * sizeof(block_secpar), (uint8_t*) output); \
	} \
	static ALWAYS_INLINE void prg_##name##_gen( \
		const prg_##name##_key* restrict prgs, const prg_##name##_fixed_key* restrict fixed_key, \
		size_t num_keys, uint32_t num_blocks, uint32_t counter, prg_##name##_block* restrict output) \
	{ \
		/* This PRG is only used to generate leafs of size 3*secpar, which */ \
		/* should be small enough * that it's handled by just prg_init. */ \
		(void) prgs; \
		(void) fixed_key; \
		(void) counter; \
		(void) output; \
		(void) num_keys; \
		(void) num_blocks; \
		assert(num_keys == 0 || num_blocks == 0); \
	}

#if defined(PRG_AES_CTR)
#define PRG_VOLE_PREFERRED_WIDTH AES_PREFERRED_WIDTH
#define PRG_VOLE_PREFERRED_WIDTH_SHIFT AES_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_AES_CTR(vole)
#elif defined(PRG_RIJNDAEL_EVEN_MANSOUR)
#define PRG_VOLE_PREFERRED_WIDTH FIXED_KEY_PREFERRED_WIDTH
#define PRG_VOLE_PREFERRED_WIDTH_SHIFT FIXED_KEY_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_RIJNDAEL_FIXED_KEY_CTR(vole)
#endif

#if defined(TREE_PRG_AES_CTR)
#define PRG_TREE_PREFERRED_WIDTH AES_PREFERRED_WIDTH
#define PRG_TREE_PREFERRED_WIDTH_SHIFT AES_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_AES_CTR(tree)
#elif defined(TREE_PRG_RIJNDAEL_EVEN_MANSOUR)
#define PRG_TREE_PREFERRED_WIDTH FIXED_KEY_PREFERRED_WIDTH
#define PRG_TREE_PREFERRED_WIDTH_SHIFT FIXED_KEY_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_RIJNDAEL_FIXED_KEY_CTR(tree)
#endif

#if defined(LEAF_PRG_AES_CTR)
#define PRG_LEAF_PREFERRED_WIDTH AES_PREFERRED_WIDTH
#define PRG_LEAF_PREFERRED_WIDTH_SHIFT AES_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_AES_CTR(leaf)
#elif defined(LEAF_PRG_RIJNDAEL_EVEN_MANSOUR)
#define PRG_LEAF_PREFERRED_WIDTH FIXED_KEY_PREFERRED_WIDTH
#define PRG_LEAF_PREFERRED_WIDTH_SHIFT FIXED_KEY_PREFERRED_WIDTH_SHIFT
DEFINE_PRG_RIJNDAEL_FIXED_KEY_CTR(leaf)
#elif defined(LEAF_PRG_SHAKE)
#define PRG_LEAF_PREFERRED_WIDTH (1 << PRG_LEAF_PREFERRED_WIDTH_SHIFT)
#define PRG_LEAF_PREFERRED_WIDTH_SHIFT 3
DEFINE_PRG_SHAKE(leaf)
#endif

#undef DEFINE_PRG_AES_CTR
#undef DEFINE_PRG_RIJNDAEL_FIXED_KEY_CTR

inline void init_fixed_keys(
	prg_tree_fixed_key* fixed_key_tree, prg_leaf_fixed_key* fixed_key_leaf,
	block_secpar iv)
{
	(void) fixed_key_tree, (void) fixed_key_leaf, (void) iv;
#if defined(TREE_PRG_RIJNDAEL_EVEN_MANSOUR)
	rijndael_keygen(fixed_key_tree, iv);
#endif
#if defined(LEAF_PRG_RIJNDAEL_EVEN_MANSOUR)
	rijndael_keygen(fixed_key_leaf, iv);
#endif
}


#endif
