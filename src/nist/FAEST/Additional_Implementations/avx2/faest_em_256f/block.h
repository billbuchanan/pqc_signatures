#ifndef BLOCK_H
#define BLOCK_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

#include "config.h"

typedef struct
{
	uint64_t data[3];
} block192;

inline block192 block192_xor(block192 x, block192 y)
{
	// Plain c version for now at least. Hopefully it will be autovectorized.
	block192 out;
	out.data[0] = x.data[0] ^ y.data[0];
	out.data[1] = x.data[1] ^ y.data[1];
	out.data[2] = x.data[2] ^ y.data[2];
	return out;
}

inline block192 block192_and(block192 x, block192 y)
{
	block192 out;
	out.data[0] = x.data[0] & y.data[0];
	out.data[1] = x.data[1] & y.data[1];
	out.data[2] = x.data[2] & y.data[2];
	return out;
}

inline block192 block192_set_all_8(uint8_t x)
{
	uint64_t x64 = x;
	x64 *= UINT64_MAX / 0xff;

	block192 out = {{x64, x64, x64}};
	return out;
}

inline block192 block192_set_low64(uint64_t x)
{
	block192 out = {{x, 0, 0}};
	return out;
}

inline block192 block192_set_low32(uint32_t x)
{
	return block192_set_low64(x);
}

inline block192 block192_set_zero()
{
	return block192_set_low64(0);
}

#include "block_impl.h"

// Interface defined by block_impl.h

// typedef /**/ block128;
// typedef /**/ block256;
// typedef /**/ block384;
// typedef /**/ block512;
//
// // Block representing a chunck of a column for the small field VOLE. Used when reducing the PRG
// // outputs down to a VOLE correlation. THis will be at least as big as vole_cipher_block.
// typedef /**/ vole_block;
// #define VOLE_BLOCK_SHIFT /**/

// Number of block128s in a vole_block.
#define VOLE_BLOCK (1 << VOLE_BLOCK_SHIFT)

static_assert(sizeof(block128) == 16, "Padding in block128.");
static_assert(sizeof(block192) == 24, "Padding in block192.");
static_assert(sizeof(block256) == 32, "Padding in block256.");
static_assert(sizeof(block384) == 48, "Padding in block384.");
static_assert(sizeof(block512) == 64, "Padding in block512.");

inline block128 block128_xor(block128 x, block128 y);
inline block256 block256_xor(block256 x, block256 y);
inline block384 block384_xor(block384 x, block384 y);
inline block512 block512_xor(block512 x, block512 y);
inline vole_block vole_block_xor(vole_block x, vole_block y);

inline block128 block128_and(block128 x, block128 y);
inline block256 block256_and(block256 x, block256 y);
inline block384 block384_and(block384 x, block384 y);
inline block512 block512_and(block512 x, block512 y);
inline vole_block vole_block_and(vole_block x, vole_block y);

inline block128 block128_set_zero();
inline block256 block256_set_zero();
inline block384 block384_set_zero();
inline block512 block512_set_zero();

inline block128 block128_set_all_8(uint8_t x);
inline block256 block256_set_all_8(uint8_t x);
inline block384 block384_set_all_8(uint8_t x);
inline block512 block512_set_all_8(uint8_t x);
inline vole_block vole_block_set_all_8(uint8_t x);

inline block128 block128_set_low32(uint32_t x);
inline block256 block256_set_low32(uint32_t x);
inline block384 block384_set_low32(uint32_t x);
inline block512 block512_set_low32(uint32_t x);
inline vole_block vole_block_set_low32(uint32_t x);

inline block128 block128_set_low64(uint64_t x);
inline block256 block256_set_low64(uint64_t x);
inline block384 block384_set_low64(uint64_t x);
inline block512 block512_set_low64(uint64_t x);
inline vole_block vole_block_set_low64(uint64_t x);

inline block256 block256_set_128(block128 x0, block128 x1);
inline block256 block256_set_low128(block128 x);

inline bool block128_any_zeros(block128 x);
inline bool block192_any_zeros(block192 x);
inline bool block256_any_zeros(block256 x);

inline block128 block128_byte_reverse(block128 x);

inline block256 block256_from_2_block128(block128 x, block128 y);

#if SECURITY_PARAM == 128
#define BLOCK_SECPAR_LEN_SHIFT 0
#define BLOCK_2SECPAR_LEN 2
typedef block128 block_secpar;
typedef block256 block_2secpar;
inline block_secpar block_secpar_xor(block_secpar x, block_secpar y) { return block128_xor(x, y); }
inline block_secpar block_secpar_and(block_secpar x, block_secpar y) { return block128_and(x, y); }
inline block_secpar block_secpar_set_all_8(uint8_t x) { return block128_set_all_8(x); }
inline block_secpar block_secpar_set_low32(uint32_t x) { return block128_set_low32(x); }
inline block_secpar block_secpar_set_low64(uint64_t x) { return block128_set_low64(x); }
inline block_secpar block_secpar_set_zero() { return block128_set_zero(); }
inline bool block_secpar_any_zeros(block_secpar x) { return block128_any_zeros(x); }
inline block_2secpar block_2secpar_xor(block_2secpar x, block_2secpar y) { return block256_xor(x, y); }
inline block_2secpar block_2secpar_and(block_2secpar x, block_2secpar y) { return block256_and(x, y); }
inline block_2secpar block_2secpar_set_all_8(uint8_t x) { return block256_set_all_8(x); }
inline block_2secpar block_2secpar_set_low32(uint32_t x) { return block256_set_low32(x); }
inline block_2secpar block_2secpar_set_low64(uint64_t x) { return block256_set_low64(x); }
inline block_2secpar block_2secpar_set_zero() { return block256_set_zero(); }
#elif SECURITY_PARAM == 192
#define BLOCK_2SECPAR_LEN 3
typedef block192 block_secpar;
typedef block384 block_2secpar;
inline block_secpar block_secpar_xor(block_secpar x, block_secpar y) { return block192_xor(x, y); }
inline block_secpar block_secpar_and(block_secpar x, block_secpar y) { return block192_and(x, y); }
inline block_secpar block_secpar_set_all_8(uint8_t x) { return block192_set_all_8(x); }
inline block_secpar block_secpar_set_low32(uint32_t x) { return block192_set_low32(x); }
inline block_secpar block_secpar_set_low64(uint64_t x) { return block192_set_low64(x); }
inline block_secpar block_secpar_set_zero() { return block192_set_zero(); }
inline bool block_secpar_any_zeros(block_secpar x) { return block192_any_zeros(x); }
inline block_2secpar block_2secpar_xor(block_2secpar x, block_2secpar y) { return block384_xor(x, y); }
inline block_2secpar block_2secpar_and(block_2secpar x, block_2secpar y) { return block384_and(x, y); }
inline block_2secpar block_2secpar_set_all_8(uint8_t x) { return block384_set_all_8(x); }
inline block_2secpar block_2secpar_set_low32(uint32_t x) { return block384_set_low32(x); }
inline block_2secpar block_2secpar_set_low64(uint64_t x) { return block384_set_low64(x); }
inline block_2secpar block_2secpar_set_zero() { return block384_set_zero(); }
#elif SECURITY_PARAM == 256
#define BLOCK_SECPAR_LEN_SHIFT 1
#define BLOCK_2SECPAR_LEN 4
typedef block256 block_secpar;
typedef block512 block_2secpar;
inline block_secpar block_secpar_xor(block_secpar x, block_secpar y) { return block256_xor(x, y); }
inline block_secpar block_secpar_and(block_secpar x, block_secpar y) { return block256_and(x, y); }
inline block_secpar block_secpar_set_all_8(uint8_t x) { return block256_set_all_8(x); }
inline block_secpar block_secpar_set_low32(uint32_t x) { return block256_set_low32(x); }
inline block_secpar block_secpar_set_low64(uint64_t x) { return block256_set_low64(x); }
inline block_secpar block_secpar_set_zero() { return block256_set_zero(); }
inline bool block_secpar_any_zeros(block_secpar x) { return block256_any_zeros(x); }
inline block_2secpar block_2secpar_xor(block_2secpar x, block_2secpar y) { return block512_xor(x, y); }
inline block_2secpar block_2secpar_and(block_2secpar x, block_2secpar y) { return block512_and(x, y); }
inline block_2secpar block_2secpar_set_all_8(uint8_t x) { return block512_set_all_8(x); }
inline block_2secpar block_2secpar_set_low32(uint32_t x) { return block512_set_low32(x); }
inline block_2secpar block_2secpar_set_low64(uint64_t x) { return block512_set_low64(x); }
inline block_2secpar block_2secpar_set_zero() { return block512_set_zero(); }
#endif

// Number of block128s in a block_secpar, assuming that this is a whole number.
#define BLOCK_SECPAR_LEN (1 << BLOCK_SECPAR_LEN_SHIFT)

#endif
