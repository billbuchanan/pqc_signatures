#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#include "config.h"
#include "quicksilver.h"
#include "vole_check.h"
#include "vole_params.h"
#include "vector_com.h"
#include "vole_commit.h"

#if defined(OWF_AES_CTR)
#define FAEST_IV_BYTES (OWF_BLOCKS * OWF_BLOCK_SIZE)
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR)
#define FAEST_IV_BYTES (SECURITY_PARAM / 8)
#endif

#define FAEST_SECRET_KEY_BYTES ((SECURITY_PARAM / 8) + FAEST_IV_BYTES)

#if defined(OWF_AES_CTR) && SECURITY_PARAM == 192
#define FAEST_PUBLIC_KEY_BYTES (32 + FAEST_IV_BYTES)
#else
#define FAEST_PUBLIC_KEY_BYTES FAEST_SECRET_KEY_BYTES
#endif

#define FAEST_SIGNATURE_BYTES ( \
	VOLE_COMMIT_SIZE + \
	VOLE_CHECK_PROOF_BYTES + \
	WITNESS_BITS / 8 + \
	QUICKSILVER_PROOF_BYTES + \
	VECTOR_OPEN_SIZE + \
	SECURITY_PARAM / 8 + \
	16)

// Random seed can be set to null for deterministic signatures.

bool faest_pubkey(uint8_t* pk_packed, const uint8_t* sk_packed);
bool faest_sign(
	uint8_t* signature, const uint8_t* msg, size_t msg_len, const uint8_t* sk_packed,
	const uint8_t* random_seed, size_t random_seed_len);
bool faest_verify(const uint8_t* signature, const uint8_t* msg, size_t msg_len,
                  const uint8_t* pk_packed);
