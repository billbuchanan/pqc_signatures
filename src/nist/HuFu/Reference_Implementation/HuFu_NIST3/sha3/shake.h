#ifndef _SHAKE_H_
#define _SHAKE_H_

#include <stdio.h>
#include <stdint.h>
typedef struct
{
	union
	{
		uint64_t A[25];
		uint8_t dbuf[200];
	} st;
	uint64_t dptr;
} inner_shake256_context;

typedef struct
{
	union
	{
		uint8_t d[512]; /* MUST be 512, exactly */
		uint64_t dummy_u64;
	} buf;
	size_t ptr;
	union
	{
		uint8_t d[256];
		uint64_t dummy_u64;
	} state;
	int type;
} prng;
void get_seed(size_t len, prng *rng, uint8_t seed[32]);
void prng_init(prng *p, inner_shake256_context *src);
void prng_get_bytes(prng *p, void *dst, size_t len);
void testrandbytes();
#endif