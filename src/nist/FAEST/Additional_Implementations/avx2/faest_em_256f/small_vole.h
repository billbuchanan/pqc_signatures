#ifndef SMALL_VOLE_H
#define SMALL_VOLE_H

#include <assert.h>

#include "config.h"
#include "block.h"
#include "aes.h"
#include "vole_params.h"

// Given 2**k PRG keys and the chosen VOLE input u, generate the VOLE correlation v and a correction
// c to send to the receiver. u and c must be volumn vectors, while v must be a column-major matrix.
// They must all be VOLE_COL_BLOCKS vole_blocks tall, while v must have k rows. k must be at least
// VOLE_WIDTH_SHIFT. fixed_key is only used for PRGs based on fixed-key Rijndael. The input keys
// must be permuted according to vole_permute_key_index, i.e.,
// keys[i] = original_keys[vole_permute_key_index(i)];
void vole_sender(
	unsigned int k, const block_secpar* restrict keys,
	block128 iv, const prg_vole_fixed_key* restrict fixed_key,
	const vole_block* restrict u, vole_block* restrict v, vole_block* restrict c);

// Given 2**k PRG keys, the secret delta, and the correction string c, generate the VOLE correlation
// q. c and q are stored similarly to c and v in vole_sender. A k-bit delta is represented as k
// bytes in little endian, with each byte being either 0 or 0xff. Input must by permuted by XOR with
// Delta, and then with vole_permute_key_index, i.e.,
// keys[i] = original_keys[vole_permute_key_index(i) ^ delta];
void vole_receiver(
	unsigned int k, const block_secpar* restrict keys,
	block128 iv, const prg_vole_fixed_key* restrict fixed_key,
	const vole_block* restrict c, vole_block* restrict q,
	const uint8_t* restrict delta);

void vole_receiver_apply_correction(
	size_t row_blocks, size_t cols,
	const vole_block* restrict c, vole_block* restrict q, const uint8_t* restrict delta);

inline size_t vole_permute_key_index(size_t i)
{
	// Convert the high bits of i (indicating which chunk of VOLE_WIDTH keys) to Gray's code, while
	// keeping the low bits (indicating the position within the VOLE_WIDTH keys) unchanged.
	return i ^ ((i >> 1) & -VOLE_WIDTH);
}

inline size_t vole_permute_key_index_inv(size_t i)
{
	size_t j = i;

	#ifdef __GNUC__
	#pragma GCC unroll (5)
	#endif
	for (unsigned int shift = 1; shift < VOLE_MAX_K - VOLE_WIDTH_SHIFT; shift <<= 1)
		j ^= (j >> shift);

	return (j & -VOLE_WIDTH) + (i % VOLE_WIDTH);
}

// Optimized computation of vole_permute_key_index_inv(i) ^ vole_permute_key_index_inv(i + offset).
// offset must be a power of 2, and must be at most VOLE_WIDTH.
inline size_t vole_permute_inv_increment(size_t i, size_t offset)
{
	static_assert(VOLE_MAX_K < 16, "");

	size_t diff_in = i ^ (i + offset);
	size_t diff_out_even = diff_in & (0x5555 | (VOLE_WIDTH - 1));
	size_t diff_out_odd = diff_in & (0xAAAA | (VOLE_WIDTH - 1));
	return diff_out_odd > diff_out_even ? diff_out_odd : diff_out_even;
}

inline void vole_fixed_key_init(prg_vole_fixed_key* fixed_key, block_secpar iv)
{
	(void) fixed_key, (void) iv;
#if defined(PRG_RIJNDAEL_EVEN_MANSOUR)
	rijndael_keygen(fixed_key, iv);
#endif
}

#endif
