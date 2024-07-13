#ifndef QUICKSILVER_H
#define QUICKSILVER_H

#include "polynomials.h"
#include "universal_hash.h"
#include "util.h"

#define QUICKSILVER_CHALLENGE_BYTES ((3 * SECURITY_PARAM + 64) / 8)
#define QUICKSILVER_PROOF_BYTES (SECURITY_PARAM / 8)
#define QUICKSILVER_CHECK_BYTES (SECURITY_PARAM / 8)

typedef struct
{
	poly_secpar_vec mac;
	poly1_vec value;
} quicksilver_vec_gf2;

typedef struct
{
	poly_secpar_vec mac;
	poly_secpar_vec value;
} quicksilver_vec_gfsecpar;

typedef struct
{
	bool verifier;
	poly_secpar_vec delta; // All components are equal
	poly_secpar_vec deltaSq; // Ditto

	hasher_gfsecpar_key key_secpar;
	hasher_gfsecpar_state state_secpar_const;
	hasher_gfsecpar_state state_secpar_linear;

	hasher_gfsecpar_64_key key_64;
	hasher_gfsecpar_64_state state_64_const;
	hasher_gfsecpar_64_state state_64_linear;

	poly_secpar_vec hash_combination[2];

	const uint8_t* witness;
	const block_secpar* macs;
} quicksilver_state;

// Initialize a prover's quicksilver_state. challenge must have length QUICKSILVER_CHALLENGE_BYTES.
void quicksilver_init_prover(
	quicksilver_state* state, const uint8_t* witness, const block_secpar* macs,
	size_t num_constraints, const uint8_t* challenge);

// Initialize a verifier's quicksilver_state. challenge must have length
// QUICKSILVER_CHALLENGE_BYTES.
void quicksilver_init_verifier(
	quicksilver_state* state, const block_secpar* macs, size_t num_constraints,
	block_secpar delta, const uint8_t* challenge);

inline quicksilver_vec_gf2 quicksilver_get_witness_vec(const quicksilver_state* state, size_t index)
{
	quicksilver_vec_gf2 out;
	if (!state->verifier)
	{
		uint16_t tmp;

		// This won't overflow the bounds of witness because the are extra masking bits at the end,
		// which won't get accessed through this function.
		memcpy(&tmp, &state->witness[index / 8], sizeof(tmp));
		out.value = poly1_load(tmp, index % 8);
	}
	out.mac = poly_secpar_load(&state->macs[index]);
	return out;
}

inline poly_secpar_vec quicksilver_get_delta(const quicksilver_state* state) {
    assert(state->verifier);
    return state->delta;
}

inline quicksilver_vec_gf2 quicksilver_add_gf2(const quicksilver_state* state, quicksilver_vec_gf2 x, quicksilver_vec_gf2 y)
{
	quicksilver_vec_gf2 out;
	if (!state->verifier)
		out.value = x.value ^ y.value;
	out.mac = poly_secpar_add(x.mac, y.mac);
	return out;
}

inline quicksilver_vec_gfsecpar quicksilver_add_gfsecpar(const quicksilver_state* state, quicksilver_vec_gfsecpar x, quicksilver_vec_gfsecpar y)
{
	quicksilver_vec_gfsecpar out;
	if (!state->verifier)
		out.value = poly_secpar_add(x.value, y.value);
	out.mac = poly_secpar_add(x.mac, y.mac);
	return out;
}

inline quicksilver_vec_gf2 quicksilver_zero_gf2()
{
	quicksilver_vec_gf2 out;
	out.value = 0;
	out.mac = poly_secpar_set_zero();
	return out;
}

inline quicksilver_vec_gfsecpar quicksilver_zero_gfsecpar()
{
	quicksilver_vec_gfsecpar out;
	out.value = poly_secpar_set_zero();
	out.mac = poly_secpar_set_zero();
	return out;
}

inline quicksilver_vec_gf2 quicksilver_one_gf2(const quicksilver_state* state)
{
	quicksilver_vec_gf2 out;
	if (state->verifier)
		out.mac = state->delta;
	else
	{
		out.mac = poly_secpar_set_zero();
		out.value = poly1_set_all(0xff);
	}
	return out;
}

inline quicksilver_vec_gfsecpar quicksilver_one_gfsecpar(const quicksilver_state* state)
{
	quicksilver_vec_gfsecpar out;
	if (state->verifier)
		out.mac = state->delta;
	else
	{
		out.mac = poly_secpar_set_zero();
		out.value = poly_secpar_set_low32(1);
	}
	return out;
}

inline quicksilver_vec_gf2 quicksilver_const_gf2(const quicksilver_state* state, poly1_vec c)
{
	quicksilver_vec_gf2 out;
	if (state->verifier)
		out.mac = poly1xsecpar_mul(c, state->delta);
	else
	{
		out.mac = poly_secpar_set_zero();
		out.value = c;
	}
	return out;
}

inline quicksilver_vec_gfsecpar quicksilver_const_gfsecpar(const quicksilver_state* state, poly_secpar_vec c)
{
	quicksilver_vec_gfsecpar out;
	if (state->verifier)
		out.mac = poly_2secpar_reduce_secpar(poly_secpar_mul(state->delta, c));
	else
	{
		out.mac = poly_secpar_set_zero();
		out.value = c;
	}
	return out;
}

inline quicksilver_vec_gf2 quicksilver_mul_const_gf2(const quicksilver_state* state, quicksilver_vec_gf2 x, poly1_vec c)
{
	x.mac = poly1xsecpar_mul(c, x.mac);
	if (!state->verifier)
		x.value &= c;
	return x;
}

inline quicksilver_vec_gfsecpar quicksilver_mul_const(const quicksilver_state* state, quicksilver_vec_gfsecpar x, poly_secpar_vec c)
{
	x.mac = poly_2secpar_reduce_secpar(poly_secpar_mul(c, x.mac));
	if (!state->verifier)
		x.value = poly_2secpar_reduce_secpar(poly_secpar_mul(c, x.value));
	return x;
}

inline quicksilver_vec_gfsecpar quicksilver_combine_8_bits(const quicksilver_state* state, const quicksilver_vec_gf2* qs_bits)
{
	quicksilver_vec_gfsecpar out;

	poly_secpar_vec macs[8];
	for (size_t i = 0; i < 8; ++i)
		macs[i] = qs_bits[i].mac;
	out.mac = poly_secpar_from_8_poly_secpar(macs);

	if (!state->verifier)
	{
		poly1_vec bits[8];
		for (size_t i = 0; i < 8; ++i)
			bits[i] = qs_bits[i].value;
		out.value = poly_secpar_from_8_poly1(bits);
	}

	return out;
}

// load 8 consecutive bits from s into QS GF(2) values, then combine them into a GF(2^secpar) value
// in the GF(2^8) subfield
inline quicksilver_vec_gfsecpar quicksilver_const_8_bits(const quicksilver_state* state, const void* s)
{
    quicksilver_vec_gf2 input_bits[8];
    for (size_t bit_j = 0; bit_j < 8; ++bit_j) {
        input_bits[bit_j] = quicksilver_const_gf2(state, poly1_load(*(uint8_t*)s, bit_j));
    }
    return quicksilver_combine_8_bits(state, input_bits);
}

// load 8 consecutive bits from the witness and combine them into a GF(2^secpar) value in the
// GF(2^8) subfield
inline quicksilver_vec_gfsecpar quicksilver_get_witness_8_bits(const quicksilver_state* state, size_t bit_index)
{
    quicksilver_vec_gf2 input_bits[8];
    for (size_t bit_j = 0; bit_j < 8; ++bit_j) {
        input_bits[bit_j] = quicksilver_get_witness_vec(state, bit_index + bit_j);
    }
    return quicksilver_combine_8_bits(state, input_bits);
}

// Add a constraint of the form x*y == 1.
inline void quicksilver_add_product_constraints(quicksilver_state* state, quicksilver_vec_gfsecpar x, quicksilver_vec_gfsecpar y)
{
	if (state->verifier)
	{
		poly_secpar_vec term = poly_secpar_add(poly_2secpar_reduce_secpar(poly_secpar_mul(x.mac, y.mac)), state->deltaSq);
		hasher_gfsecpar_update(&state->key_secpar, &state->state_secpar_const, term);
		hasher_gfsecpar_64_update(&state->key_64, &state->state_64_const, term);
	}
	else
	{
		// Use Karatsuba to save a multiplication.
		poly_secpar_vec x0_y0 = poly_2secpar_reduce_secpar(poly_secpar_mul(x.mac, y.mac));
		poly_secpar_vec x1_y1 = poly_2secpar_reduce_secpar(poly_secpar_mul(poly_secpar_add(x.value, x.mac), poly_secpar_add(y.value, y.mac)));

		// Assume that the constraint is valid, so x.value * y.value = 1.
		poly_secpar_vec xinf_yinf = poly_secpar_set_low32(1);
		assert(poly_secpar_eq(poly_2secpar_reduce_secpar(poly_secpar_mul(x.value, y.value)), xinf_yinf));

		poly_secpar_vec lin_term = poly_secpar_add(poly_secpar_add(x0_y0, xinf_yinf), x1_y1);

		hasher_gfsecpar_update(&state->key_secpar, &state->state_secpar_const, x0_y0);
		hasher_gfsecpar_update(&state->key_secpar, &state->state_secpar_linear, lin_term);
		hasher_gfsecpar_64_update(&state->key_64, &state->state_64_const, x0_y0);
		hasher_gfsecpar_64_update(&state->key_64, &state->state_64_linear, lin_term);
	}
}

void quicksilver_prove(const quicksilver_state* restrict state, size_t witness_bits,
                       uint8_t* restrict proof, uint8_t* restrict check);
void quicksilver_verify(const quicksilver_state* restrict state, size_t witness_bits,
                        const uint8_t* restrict proof, uint8_t* restrict check);

#endif
