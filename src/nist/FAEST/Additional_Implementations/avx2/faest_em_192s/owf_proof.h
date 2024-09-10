#ifndef OWF_PROOF_H
#define OWF_PROOF_H

#if defined(OWF_AES_CTR)

// Number of applications of the round function per encryption (need OWF_ROUNDS + 1 round keys).
#define OWF_ROUNDS AES_ROUNDS
// Block size of the cipher
#define OWF_BLOCK_SIZE 16
// Number of blocks encrypted in the OWF.
// (1 for 128 bit, 2 for 192/256 bit to compensate for the 128 bit block size)
#define OWF_BLOCKS ((SECURITY_PARAM + 127) / 128)

// Spacing in bytes of the sub_words operation in the key schedule.
#if SECURITY_PARAM == 256
#define OWF_KEY_SCHEDULE_PERIOD 16
#else
#define OWF_KEY_SCHEDULE_PERIOD (SECURITY_PARAM / 8)
#endif

// Number of S-boxes in the key schedule.
#define OWF_KEY_SCHEDULE_CONSTRAINTS \
	(4 * (((AES_ROUNDS + 1) * 16 - SECURITY_PARAM / 8 + \
	       OWF_KEY_SCHEDULE_PERIOD - 1) / OWF_KEY_SCHEDULE_PERIOD))
#define OWF_KEY_WITNESS_BITS (SECURITY_PARAM + 8 * OWF_KEY_SCHEDULE_CONSTRAINTS)

typedef block128 owf_block;
inline owf_block owf_block_xor(owf_block x, owf_block y) { return block128_xor(x, y); }
inline owf_block owf_block_set_low32(uint32_t x) { return block128_set_low32(x); }
inline bool owf_block_any_zeros(owf_block x) { return block128_any_zeros(x); }

#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR)

#define OWF_KEY_SCHEDULE_CONSTRAINTS 0
#define OWF_KEY_WITNESS_BITS SECURITY_PARAM
#define OWF_BLOCK_SIZE (SECURITY_PARAM / 8)
#define OWF_BLOCKS 1

#if SECURITY_PARAM == 128
#define OWF_ROUNDS AES_ROUNDS
#elif SECURITY_PARAM == 192
#define OWF_ROUNDS RIJNDAEL192_ROUNDS
#elif SECURITY_PARAM == 256
#define OWF_ROUNDS RIJNDAEL256_ROUNDS
#endif

typedef block_secpar owf_block;
inline owf_block owf_block_xor(owf_block x, owf_block y) { return block_secpar_xor(x, y); }
inline owf_block owf_block_set_low32(uint32_t x) { return block_secpar_set_low32(x); }
inline bool owf_block_any_zeros(owf_block x) { return block_secpar_any_zeros(x); }

#else

#error "Unsupported one-way function."
#endif


#define OWF_NUM_CONSTRAINTS (OWF_BLOCKS * OWF_BLOCK_SIZE * OWF_ROUNDS + OWF_KEY_SCHEDULE_CONSTRAINTS)
#define WITNESS_BITS (8 * OWF_BLOCKS * OWF_BLOCK_SIZE * (OWF_ROUNDS - 1) + OWF_KEY_WITNESS_BITS)

#include "aes.h"
#include "quicksilver.h"

struct public_key;
typedef struct public_key public_key;

void owf_constraints_prover(quicksilver_state* state, const public_key* pk);
void owf_constraints_verifier(quicksilver_state* state, const public_key* pk);

#endif
