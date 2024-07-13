#include "config.h"

#include "aes.h"
#include "faest_details.h"
#include "owf_proof.h"
#include "quicksilver.h"

#define NUM_COLS (OWF_BLOCK_SIZE / 4)
#define N_WD (SECURITY_PARAM / 32)
#define S_ENC (OWF_BLOCK_SIZE * OWF_ROUNDS)

#if defined(OWF_AES_CTR)

static ALWAYS_INLINE void key_sched_fwd(quicksilver_state* state, quicksilver_vec_gf2* output) {
    for (size_t bit_i = 0; bit_i < SECURITY_PARAM; ++bit_i) {
        output[bit_i] = quicksilver_get_witness_vec(state, bit_i);
    }
    // current index in the extended witness
    size_t i_wd = SECURITY_PARAM;
    for (size_t word_j = N_WD; word_j < 4 * (AES_ROUNDS + 1); ++word_j) {
        if (word_j % N_WD == 0 || (N_WD > 6 && word_j % N_WD == 4)) {
            for (size_t bit_i = 0; bit_i < 32; ++bit_i) {
                output[32 * word_j + bit_i] = quicksilver_get_witness_vec(state, i_wd + bit_i);
            }
            i_wd += 32;
        } else {
            for (size_t bit_i = 0; bit_i < 32; ++bit_i) {
                output[32 * word_j + bit_i] = quicksilver_add_gf2(state,
                    output[32 * (word_j - N_WD) + bit_i], output[32 * (word_j - 1) + bit_i]);
            }
        }
    }
}

static ALWAYS_INLINE void key_sched_lift_round_key_bits(quicksilver_state* state,
        const quicksilver_vec_gf2* round_key_bits, quicksilver_vec_gfsecpar* output) {
    for (size_t byte_i = 0; byte_i < OWF_BLOCK_SIZE * (OWF_ROUNDS + 1); ++byte_i) {
        output[byte_i] = quicksilver_combine_8_bits(state, &round_key_bits[8 * byte_i]);
    }
}

static ALWAYS_INLINE void key_sched_bkwd(quicksilver_state* state, const quicksilver_vec_gf2* round_key_bits,
        quicksilver_vec_gfsecpar* output) {
    size_t i_wd = 0; // bit index to the round key word we are currently handling
    size_t i_rcon = 0; // round constant index
    bool remove_rcon = true; // flag indicating if we need to remove the round constant from the
                             // next word
    const quicksilver_vec_gf2 qs_one = quicksilver_one_gf2(state);
    for (size_t sbox_j = 0; sbox_j < OWF_KEY_SCHEDULE_CONSTRAINTS; ++sbox_j) {
        // load the witness byte
        quicksilver_vec_gf2 sbox_out[8];
        for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
            sbox_out[bit_i] = quicksilver_get_witness_vec(state, SECURITY_PARAM + sbox_j * 8 + bit_i);
            // remove the byte that was xored in
            sbox_out[bit_i] = quicksilver_add_gf2(state, sbox_out[bit_i],
                    round_key_bits[i_wd + 8 * (sbox_j % 4) + bit_i]);
        }
        // (possibly) remove the round constant
        if (sbox_j % 4 == 0 && remove_rcon) {
            // remove the round constant from the first byte of every word coming through the sboxes
            for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
                if ((aes_round_constants[i_rcon] >> bit_i) & 1) {
                    sbox_out[bit_i] = quicksilver_add_gf2(state, sbox_out[bit_i], qs_one);
                }
            }
            ++i_rcon;
        }

        quicksilver_vec_gf2 inv_out[8];
        for (size_t i = 0; i < 8; ++i)
            inv_out[(i + 1) % 8] = sbox_out[i];
        for (size_t i = 0; i < 8; ++i)
            inv_out[(i + 3) % 8] = quicksilver_add_gf2(state, inv_out[(i + 3) % 8], sbox_out[i]);
        for (size_t i = 0; i < 8; ++i)
            inv_out[(i + 6) % 8] = quicksilver_add_gf2(state, inv_out[(i + 6) % 8], sbox_out[i]);
        inv_out[0] = quicksilver_add_gf2(state, inv_out[0], qs_one);
        inv_out[2] = quicksilver_add_gf2(state, inv_out[2], qs_one);

        // lift into a field element and store in the output buffer
        output[sbox_j] = quicksilver_combine_8_bits(state, inv_out);

        if (sbox_j % 4 == 3) {
            // increase i_wd to point to the next word
            if (SECURITY_PARAM == 192) {
                i_wd += 192;
            } else {
                i_wd += 128;
                if (SECURITY_PARAM == 256) {
                    remove_rcon = !remove_rcon;
                }
            }
        }
    }
}

static ALWAYS_INLINE void key_sched_constraints(quicksilver_state* state, quicksilver_vec_gf2* round_key_bits,
        quicksilver_vec_gfsecpar* round_key_bytes) {
    quicksilver_vec_gfsecpar key_schedule_inv_outs[OWF_KEY_SCHEDULE_CONSTRAINTS];
    key_sched_fwd(state, round_key_bits);
    key_sched_bkwd(state, round_key_bits, key_schedule_inv_outs);
    key_sched_lift_round_key_bits(state, round_key_bits, round_key_bytes);

    // byte index of the current word to read from the round keys
    size_t i_wd = 4 * (N_WD - 1);
    // for 256 bit we only rotate every second time
    bool rotate_word = true;
    quicksilver_vec_gfsecpar lhss[4];
    quicksilver_vec_gfsecpar rhss[4];
    for (size_t sboxwd_j = 0; sboxwd_j < OWF_KEY_SCHEDULE_CONSTRAINTS / 4; ++sboxwd_j) {
        if (rotate_word) {
            for (size_t row_k = 0; row_k < 4; ++row_k) {
                lhss[(row_k + 3) % 4] = round_key_bytes[i_wd + row_k];
                rhss[row_k] = key_schedule_inv_outs[4 * sboxwd_j + row_k];
            }
        } else {
            for (size_t row_k = 0; row_k < 4; ++row_k) {
                lhss[row_k] = round_key_bytes[i_wd + row_k];
                rhss[row_k] = key_schedule_inv_outs[4 * sboxwd_j + row_k];
            }
        }
        for (size_t row_k = 0; row_k < 4; ++row_k) {
            quicksilver_add_product_constraints(state, lhss[row_k], rhss[row_k]);
        }
        // increase i_wd to point to the next word
        if (SECURITY_PARAM == 192) {
            i_wd += 24;
        } else {
            i_wd += 16;
            if (SECURITY_PARAM == 256) {
                rotate_word = !rotate_word;
            }
        }
    }
}

#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR)

// load the round keys into quicksilver values and "bake" EM secret key into the first round key
static ALWAYS_INLINE void load_fixed_round_key(quicksilver_state* state, quicksilver_vec_gf2* round_key_bits,
        quicksilver_vec_gfsecpar* round_key_bytes, const rijndael_round_keys* fixed_key) {
    const uint8_t* rk_bytes = (const uint8_t*) fixed_key;

    for (size_t byte_j = 0; byte_j < OWF_BLOCK_SIZE; ++byte_j) {
        for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
            round_key_bits[8 * byte_j + bit_i] = quicksilver_add_gf2(state,
                    quicksilver_const_gf2(state, poly1_load(rk_bytes[byte_j], bit_i)),
                    quicksilver_get_witness_vec(state, 8 * byte_j + bit_i));
        }
        round_key_bytes[byte_j] = quicksilver_combine_8_bits(state, &round_key_bits[8 * byte_j]);
    }

    for (size_t byte_j = OWF_BLOCK_SIZE; byte_j < OWF_BLOCK_SIZE * (OWF_ROUNDS + 1); ++byte_j) {
        for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
            round_key_bits[8 * byte_j + bit_i] = quicksilver_const_gf2(state, poly1_load(rk_bytes[byte_j], bit_i));
        }
        round_key_bytes[byte_j] = quicksilver_combine_8_bits(state, &round_key_bits[8 * byte_j]);
    }
}

#else
#error "undefined OWF"
#endif

static ALWAYS_INLINE void enc_fwd(quicksilver_state* state, const quicksilver_vec_gfsecpar* round_key_bytes, size_t witness_bit_offset, owf_block in, quicksilver_vec_gfsecpar* output) {
    const uint8_t* in_bytes = (uint8_t*)&in;

    // first round: only add the round key
    for (size_t byte_i = 0; byte_i < OWF_BLOCK_SIZE; ++byte_i) {
        quicksilver_vec_gfsecpar input_byte = quicksilver_const_8_bits(state, &in_bytes[byte_i]);
        output[byte_i] = quicksilver_add_gfsecpar(state, input_byte, round_key_bytes[byte_i]);
    }

    const poly_secpar_vec c_two = poly_secpar_from_byte(0x02);
    const poly_secpar_vec c_three = poly_secpar_from_byte(0x03);

    size_t round_key_byte_offset = OWF_BLOCK_SIZE;
    size_t output_byte_offset = OWF_BLOCK_SIZE;
    for (size_t round_i = 1; round_i < OWF_ROUNDS; ++round_i) {
        for (size_t col_j = 0; col_j < NUM_COLS; ++col_j) {
            quicksilver_vec_gfsecpar col_wit_bytes[4];
            for (size_t row_k = 0; row_k < 4; ++row_k) {
                col_wit_bytes[row_k] = quicksilver_get_witness_8_bits(state, witness_bit_offset + row_k * 8);
            }
            for (size_t row_k = 0; row_k < 4; ++row_k) {
                output[output_byte_offset + row_k] =
                    quicksilver_add_gfsecpar(state,
                        quicksilver_mul_const(state, col_wit_bytes[row_k], c_two),
                        quicksilver_add_gfsecpar(state,
                            quicksilver_mul_const(state, col_wit_bytes[(row_k + 1) % 4], c_three),
                            quicksilver_add_gfsecpar(state, col_wit_bytes[(row_k + 2) % 4],
                                quicksilver_add_gfsecpar(state, col_wit_bytes[(row_k + 3) % 4],
                                                         round_key_bytes[round_key_byte_offset + row_k]))));
            }
            witness_bit_offset += 32;
            round_key_byte_offset += 4;
            output_byte_offset += 4;
        }
   }
}

static ALWAYS_INLINE void enc_bkwd(quicksilver_state* state, const quicksilver_vec_gf2* round_key_bits, size_t witness_bit_offset, owf_block out, quicksilver_vec_gfsecpar* output) {
    const uint8_t* out_bytes = (uint8_t*)&out;
    const size_t last_round_key_bit_offset = 8 * OWF_ROUNDS * OWF_BLOCK_SIZE;

    for (size_t round_i = 0; round_i < OWF_ROUNDS; ++round_i, witness_bit_offset += OWF_BLOCK_SIZE * 8) {
        for (size_t col_j = 0; col_j < NUM_COLS; ++col_j) {
            for (size_t row_k = 0; row_k < 4; ++row_k) {
                quicksilver_vec_gf2 witness_bits[8];
#if OWF_BLOCK_SIZE == 32
                size_t inv_shifted_index;
                if (row_k >= 2) {
                    inv_shifted_index = 4 * ((col_j + NUM_COLS - row_k - 1) % NUM_COLS) + row_k;
                } else {
                    inv_shifted_index = 4 * ((col_j + NUM_COLS - row_k) % NUM_COLS) + row_k;
                }
#else
                size_t inv_shifted_index = 4 * ((col_j + NUM_COLS - row_k) % NUM_COLS) + row_k;
#endif
                if (round_i < OWF_ROUNDS - 1) {
                    // read witness bits directly
                    for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
                        witness_bits[bit_i] = quicksilver_get_witness_vec(
                                state, witness_bit_offset + 8 * inv_shifted_index + bit_i);
                    }
                } else {
                    // compute witness bits from the last round key and the output
                    for (size_t bit_i = 0; bit_i < 8; ++bit_i) {
                        witness_bits[bit_i] = quicksilver_add_gf2(state,
                                quicksilver_const_gf2(state, poly1_load(out_bytes[inv_shifted_index], bit_i)),
                                round_key_bits[last_round_key_bit_offset + 8 * inv_shifted_index + bit_i]);
#if defined(OWF_RIJNDAEL_EVEN_MANSOUR)
                        witness_bits[bit_i] = quicksilver_add_gf2(state, witness_bits[bit_i],
                                quicksilver_get_witness_vec(state, 8 * inv_shifted_index + bit_i));
#endif
                    }
                }

                quicksilver_vec_gf2 qs_one = quicksilver_one_gf2(state);
                quicksilver_vec_gf2 inv_out[8];
                for (size_t i = 0; i < 8; ++i)
                    inv_out[(i + 1) % 8] = witness_bits[i];
                for (size_t i = 0; i < 8; ++i)
                    inv_out[(i + 3) % 8] = quicksilver_add_gf2(state, inv_out[(i + 3) % 8], witness_bits[i]);
                for (size_t i = 0; i < 8; ++i)
                    inv_out[(i + 6) % 8] = quicksilver_add_gf2(state, inv_out[(i + 6) % 8], witness_bits[i]);
                inv_out[0] = quicksilver_add_gf2(state, inv_out[0], qs_one);
                inv_out[2] = quicksilver_add_gf2(state, inv_out[2], qs_one);

                // lift into a field element and store in the output buffer
                output[round_i * OWF_BLOCK_SIZE + 4 * col_j + row_k] = quicksilver_combine_8_bits(state, inv_out);
            }
        }
    }
}

static ALWAYS_INLINE void enc_constraints(quicksilver_state* state, const quicksilver_vec_gf2* round_key_bits,
        const quicksilver_vec_gfsecpar* round_key_bytes, size_t block_num, owf_block in, owf_block out) {
    // compute the starting index of the witness bits corresponding to the s-boxes in this round of
    // encryption
#if defined(OWF_AES_CTR)
    const size_t witness_bit_offset = OWF_KEY_WITNESS_BITS + block_num * OWF_BLOCK_SIZE * 8 * (OWF_ROUNDS - 1);
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR)
    assert(block_num == 0);
    const size_t witness_bit_offset = SECURITY_PARAM;
#endif

    quicksilver_vec_gfsecpar inv_inputs[S_ENC];
    quicksilver_vec_gfsecpar inv_outputs[S_ENC];
    enc_fwd(state, round_key_bytes, witness_bit_offset, in, inv_inputs);
    enc_bkwd(state, round_key_bits, witness_bit_offset, out, inv_outputs);

    for (size_t sbox_j = 0; sbox_j < S_ENC; ++sbox_j) {
        quicksilver_add_product_constraints(state, inv_inputs[sbox_j], inv_outputs[sbox_j]);
    }
}

static ALWAYS_INLINE void owf_constraints(quicksilver_state* state, const public_key* pk)
{
    quicksilver_vec_gf2 round_key_bits[8 * OWF_BLOCK_SIZE * (OWF_ROUNDS + 1)];
    quicksilver_vec_gfsecpar round_key_bytes[OWF_BLOCK_SIZE * (OWF_ROUNDS + 1)];
#if defined(OWF_AES_CTR)
    key_sched_constraints(state, round_key_bits, round_key_bytes);
    for (size_t i = 0; i < OWF_BLOCKS; ++i) {
        enc_constraints(state, round_key_bits, round_key_bytes, i, pk->owf_input[i], pk->owf_output[i]);
    }
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR)
    load_fixed_round_key(state, round_key_bits, round_key_bytes, &pk->fixed_key);
    enc_constraints(state, round_key_bits, round_key_bytes, 0, owf_block_set_low32(0), pk->owf_output[0]);
#else
#error "unsupported OWF"
#endif
}

void owf_constraints_prover(quicksilver_state* state, const public_key* pk)
{
	assert(!state->verifier);
	state->verifier = false; // Let the compiler know that it is constant.
	owf_constraints(state, pk);
}

void owf_constraints_verifier(quicksilver_state* state, const public_key* pk)
{
	assert(state->verifier);
	state->verifier = true; // Let the compiler know that it is constant.
	owf_constraints(state, pk);
}


extern inline owf_block owf_block_xor(owf_block x, owf_block y);
extern inline owf_block owf_block_set_low32(uint32_t x);
extern inline bool owf_block_any_zeros(owf_block x);
extern inline owf_block owf_block_xor(owf_block x, owf_block y);
extern inline owf_block owf_block_set_low32(uint32_t x);
extern inline bool owf_block_any_zeros(owf_block x);
