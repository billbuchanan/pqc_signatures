#include <assert.h>
#include <string.h>

#include "api.h"
#include "faest.h"
#include "randomness.h"

static_assert(CRYPTO_PUBLICKEYBYTES == FAEST_PUBLIC_KEY_BYTES, "");
static_assert(CRYPTO_SECRETKEYBYTES == FAEST_SECRET_KEY_BYTES, "");
static_assert(CRYPTO_BYTES == FAEST_SIGNATURE_BYTES, "");

int crypto_sign_keypair(unsigned char* pk, unsigned char* sk)
{
	do
	{
		rand_bytes(sk, FAEST_SECRET_KEY_BYTES);
	} while (!faest_pubkey(pk, sk));
	return 0;
}

int crypto_sign(
	unsigned char *sm, unsigned long long *smlen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *sk)
{
	*smlen = mlen + FAEST_SIGNATURE_BYTES;
	memmove(sm, m, mlen);

	uint8_t random_seed[SECURITY_PARAM / 8];
	rand_bytes(random_seed, sizeof(random_seed));
	faest_sign(sm + mlen, sm, mlen, sk, random_seed, sizeof(random_seed));
	return 0;
}

int crypto_sign_open(
	unsigned char *m, unsigned long long *mlen,
	const unsigned char *sm, unsigned long long smlen,
	const unsigned char *pk)
{
	unsigned long long m_length = smlen - FAEST_SIGNATURE_BYTES;
	if (!faest_verify(sm + m_length, sm, m_length, pk))
		return -1;

	*mlen = m_length;
	memmove(m, sm, m_length);
	return 0;
}
