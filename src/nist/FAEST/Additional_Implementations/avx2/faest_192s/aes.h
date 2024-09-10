// Outside header guard to handle mutual inclusion.
#include "config.h"
#include "vole_params.h"

#ifndef AES_H
#define AES_H

#include "block.h"
#if SECURITY_PARAM == 128
#define AES_ROUNDS 10
#elif SECURITY_PARAM == 192
#define AES_ROUNDS 12
#elif SECURITY_PARAM == 256
#define AES_ROUNDS 14
#endif

#define RIJNDAEL192_ROUNDS 12
#define RIJNDAEL256_ROUNDS 14

// Number of AES blocks to run in parallel, for maximum performance.
#define AES_PREFERRED_WIDTH (1 << AES_PREFERRED_WIDTH_SHIFT)

// Number of Rijndael256 blocks to run in parallel, for maximum performance.
#define RIJNDAEL256_PREFERRED_WIDTH (1 << RIJNDAEL256_PREFERRED_WIDTH_SHIFT)

#define FIXED_KEY_PREFERRED_WIDTH (1 << FIXED_KEY_PREFERRED_WIDTH_SHIFT)

// An IV is included even though one can use the round keys without an iv. It's just a convenient
// place to put it.
typedef struct
{
	block128 keys[AES_ROUNDS + 1];
	block128 iv;
} aes_round_keys;

typedef struct
{
	block192 keys[RIJNDAEL192_ROUNDS + 1];
} rijndael192_round_keys;

typedef struct
{
	block256 keys[RIJNDAEL256_ROUNDS + 1];
} rijndael256_round_keys;

extern unsigned char aes_round_constants[];

#include "aes_impl.h"

// Interface defined by aes_impl.h:

// #define AES_MAX_CTR /**/
// #define AES_PREFERRED_WIDTH_SHIFT /**/
// #define RIJNDAEL256_PREFERRED_WIDTH_SHIFT /**/

void aes_keygen(aes_round_keys* round_keys, block_secpar key);
void rijndael192_keygen(rijndael192_round_keys* round_keys, block192 key);
void rijndael256_keygen(rijndael256_round_keys* round_keys, block256 key);

void rijndael192_encrypt_block(
	const rijndael192_round_keys* restrict fixed_key, block192* restrict block);

// Apply 1 round of the cipher, writing the state after the SBox into after_sbox, and writing the
// new state back into state. round is the index of the round key to use, so it should start from
// one.
inline void aes_round_function(
	const aes_round_keys* restrict round_keys, block128* restrict block,
	block128* restrict after_sbox, int round);
void rijndael192_round_function(
	const rijndael192_round_keys* restrict round_keys, block192* restrict block,
	block192* restrict after_sbox, int round);
void rijndael256_round_function(
	const rijndael256_round_keys* restrict round_keys, block256* restrict block,
	block256* restrict after_sbox, int round);

// Run AES key schedule on num_keys keys, the generate num_blocks block128s of output from each.
// Each key has it's own iv. Outputs from the same key are grouped together in output. counter must
// not exceed AES_MAX_CTR on any block.
ALWAYS_INLINE void aes_keygen_ctr(
	aes_round_keys* restrict aeses, const block_secpar* restrict keys, const block128* restrict ivs,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block128* restrict output);

// Given num_keys AES keys, generate num_blocks block128s of output from each, starting at
// counter. Each key has it's own iv. Outputs from the same key are grouped together in output.
// Need 1 <= num_blocks <= 4. counter must not exceed AES_MAX_CTR on any block.
inline void aes_ctr(
	const aes_round_keys* restrict aeses,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block128* restrict output);

// Given num_keys Even-Mansour keys, generate num_blocks block_secpars of output from each in CTR
// mode, starting at counter.
inline void aes_fixed_key_ctr(
	const aes_round_keys* restrict fixed_key, const block128* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block128* restrict output);
inline void rijndael192_fixed_key_ctr(
	const rijndael192_round_keys* restrict fixed_key, const block192* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block192* restrict output);
inline void rijndael256_fixed_key_ctr(
	const rijndael256_round_keys* restrict fixed_key, const block256* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block256* restrict output);

// Same, but for block size = security parameter.
#if SECURITY_PARAM == 128

#define FIXED_KEY_PREFERRED_WIDTH_SHIFT AES_PREFERRED_WIDTH_SHIFT
typedef aes_round_keys rijndael_round_keys;
inline void rijndael_keygen(rijndael_round_keys* round_keys, block_secpar key)
{
	aes_keygen(round_keys, key);
}
inline void rijndael_fixed_key_ctr(
	const rijndael_round_keys* restrict fixed_key, const block_secpar* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block_secpar* restrict output)
{
	aes_fixed_key_ctr(fixed_key, keys, num_keys, num_blocks, counter, output);
}

#elif SECURITY_PARAM == 192

typedef rijndael192_round_keys rijndael_round_keys;
inline void rijndael_keygen(rijndael_round_keys* round_keys, block_secpar key)
{
	rijndael192_keygen(round_keys, key);
}
inline void rijndael_fixed_key_ctr(
	const rijndael_round_keys* restrict fixed_key, const block_secpar* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block_secpar* restrict output)
{
	rijndael192_fixed_key_ctr(fixed_key, keys, num_keys, num_blocks, counter, output);
}

#elif SECURITY_PARAM == 256

#define FIXED_KEY_PREFERRED_WIDTH_SHIFT RIJNDAEL256_PREFERRED_WIDTH_SHIFT
typedef rijndael256_round_keys rijndael_round_keys;
inline void rijndael_keygen(rijndael_round_keys* round_keys, block_secpar key)
{
	rijndael256_keygen(round_keys, key);
}
inline void rijndael_fixed_key_ctr(
	const rijndael_round_keys* restrict fixed_key, const block_secpar* restrict keys,
	size_t num_keys, uint32_t num_blocks, uint32_t counter, block_secpar* restrict output)
{
	rijndael256_fixed_key_ctr(fixed_key, keys, num_keys, num_blocks, counter, output);
}

#endif

#endif
