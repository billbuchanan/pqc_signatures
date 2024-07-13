/******************************************************************************
WAVE -- Code-Based Digital Signature Scheme
Copyright (c) 2023 The Wave Team
contact: wave-contact@inria.fr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef WAVE2_VF3_H
#define WAVE2_VF3_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prng/prng.h"
#include "types_f3.h"
#include "util/popcount.h"

/**
 * Initialize a vector of trits. All entries will be set to 0.
 * @param e vector to be initialized.
 * @param length length of the vector
 */
void vf3_init(vf3_e *e, size_t const length);

/**
 * Allocate a vector of trits.
 * This function calls vf3_init to initialize the elements to 0.
 * @param length length of the vector
 * @return valid pointer to a vf3_e
 */
vf3_e *vf3_alloc(size_t const length);

/**
 * Allocate an array of vectors (all the same length).
 * @param length length of each vector
 * @param array_length number of vectors
 * @return a pointer to the array of vectors
 */
vf3_e *vf3_array_alloc(size_t const length, size_t const array_length);

/**
 * Free an array of vectors.
 * @param e valid list of vectors
 * @param array_length number of elements in the list
 */
void vf3_array_free(vf3_e *e, size_t const array_length);

/**
 * Free a single vector
 * @param e valid pointer to a vector
 */
void vf3_free(vf3_e *e);

/**
 * Copy a vector.
 * @param dest destination (must be a valid pointer to a vector of sufficient
 * length)
 * @param src valid pointer to a vector
 */
void vf3_copy(vf3_e *dest, vf3_e const *src);

/**
 * Trim a vector, clearing leftover trits in the last allocated word
 * @param e valid pointer to a vector
 */
void vf3_trim(vf3_e *e);

/**
 * Check if two vectors are equal. Assumes x->size = y->size.
 * Vectors are trimmed before the check is performed.
 * @param x valid pointer to a vector
 * @param y valid pointer to a vector
 * @return 1 if the vectors are equal, 0 otherwise.
 */
int vf3_equal(vf3_e *x, vf3_e *y);

/**
 * Set all entries of a vector to a constant a.
 * @param e a valid pointer to a vector
 * @param a an integer in {0,1,2} specifying the trit to be copied into all
 * entries
 */
void vf3_vector_constant(vf3_e *e, uint8_t const a);

/**
 * Set all entries of a vector to zero.
 * @param e a valid pointer
 */
void vf3_set_to_zero(vf3_e *e);

/**
 * Get the trit (represented as an integer in {0,1,2}) in position pos from the
 * vector a.
 * @param pos position of the element
 * @param a valid pointer to a vector
 * @return element from a in the position pos
 */
uint8_t vf3_get_element(int pos, vf3_e const *a);

/**
 * Set the trit in position pos of the vector a to the value coeff (represented
 * as an integer in {0,1,2}).
 * @param pos position of the trit to be set
 * @param a valid pointer to a vector
 * @param coeff integer in {0,1,2} representing the trit to set in position pos
 * of a
 */
void vf3_set_coeff(size_t const pos, vf3_e *a, uint8_t const coeff);

/**
 * Add a trit a to the trit in position j in the vector x.
 * x_j = x_j + a
 * @param j position of the trit where a will be added
 * @param x a pointer to a valid vector
 * @param a an integer in {0,1,2} representing the trit to be added
 */
void vf3_add_coeff(int j, vf3_e *x, uint8_t const a);

/**
 * Convert 32 bits to 16 trits:
 * (Algorithm 13, used in random trit sampling)
 * @param x 32-bit unsigned integer
 * @return 16 trits packed into a 32-bit unsigned integer
 */
uint32_t convert32(uint32_t x);

/**
 * Convert 64 bits to 32 trits
 * (Algorithm 12, used in random trit sampling)
 * @param x 64-bit unsigned integer
 * @return 32 trits packed in a 64-bit unsigned integer
 */
uint64_t convert64(uint64_t x);

/**
 * Fill a vector with trits derived from an array of (typically random) 64-bit
 * words.
 * @param y a valid pointer to a vector
 * @param rnd array of 2*(y length) 64-bit words
 */
void vf3_trits_from_bits(vf3_e *y, uint64_t *rnd);

/**
 * Fill a vector with random trits.
 * @param e valid pointer to a vector
 */
void vf3_random(vf3_e *e);

/**
 * Fill a vector with random *nonzero* trits
 * @param e a valid pointer to a vector
 */
void vf3_random_non_zero(vf3_e *e);

/**
 * Concatenate a given vector with a zero vector (long enough to fill the result
 * vector).
 * @param res a valid pointer to a vector
 * @param a a valid pointer to a vector with size->a <= res->size
 */
void vf3_vector_cat_zero(vf3_e *res, vf3_e *const a);

/**
 * Concatenate two vectors a and b to produce a || b.
 * We assume res->size = a->size + b->size
 * @param res a || b
 * @param a a valid pointer to a vector
 * @param b a valid pointer to a vector
 */
void vf3_vector_cat(vf3_e *res, vf3_e *const a, vf3_e *const b);

/**
 * Copy a->size trits from x into a, trimming the result.
 * @param a a valid pointer to a vector
 * @param x a valid pointer to a vector at least as long as a
 */
void vf3_vector_split_zero(vf3_e *a, vf3_e *const x);

/**
 * Split a vector x into two vectors a and b (i.e. x = (a||b)).
 * Assumes x->size = a->size + b->size
 * @param a a valid pointer to a vector
 * @param b a valid pointer to a vector
 * @param x a valid pointer to a vector with x->size = a->size + b->size
 */
void vf3_vector_split(vf3_e *a, vf3_e *b, vf3_e *const x);

/**
 * Mask the ternary vector x with the binary vector mask
 * @param x a valid pointer to a vector of trits to be masked
 * @param mask a valid pointer to a vector of bits to be applied as a mask
 */
void vf3_vector_mask(vf3_e *x, vf2_e *mask);

/**
 * Unmask the ternary vector x with the binary vector mask
 * @param x a valid pointer to a vector of trits to be (un)masked
 * @param mask a valid pointer to a vector of bits to be applied as a negative
 * mask
 */
void vf3_vector_unmask(vf3_e *x, vf2_e *mask);

/**
 * Print a vector of trits.
 * @param a valid pointer to a vector
 */
void vf3_print(vf3_e *a);

/**
 * Read a ternary vector from a file (in internal format)
 * @param e a valid pointer to a vector
 * @param a stream open for reading
 * @return 0 on failure, e->size on success
 */
int vf3_read(vf3_e *e, FILE *f);

/**
 * Write a ternary vector to a file (in internal format)
 * @param e a valid pointer to a vector
 * @param f a stream open for writing
 * @return 0 on failure, e->size on success
 */
int vf3_write(vf3_e *e, FILE *f);

/**
 * Hamming weight of a ternary vector (i.e. number of nonzero trits)
 * @param e a valid pointer to a vector
 * @return number of nonzero trits in e
 */
static inline int vf3_hamming_weight(vf3_e *e) {
  int weight = 0;
  int length;
  int i;
  for (i = 0, length = e->size; length >= WORD_LENGTH;
       i++, length -= WORD_LENGTH) {
    weight += popcount(e->r0[i]);
  }
  if (length) {  // make sure we do not count stray bits in the last allocated
                 // word
    wave_word mask = (1UL << length) - 1;
    weight += popcount(e->r0[i] & mask);
  }
  return weight;
}

/**
 * Count the trits equal to 2 in a given vector.
 * @param e a valid pointer to a vector
 * @return number of trits equal to 2 in e
 */
static inline size_t vf3_number_of_coordinates_equal_to_two(vf3_e *e) {
  int weight = 0;
  int length;
  int i;
  for (i = 0, length = e->size; length >= WORD_LENGTH;
       i++, length -= WORD_LENGTH) {
    weight += popcount(e->r1[i]);
  }
  if (length) {  // make sure we do not count stray bits in the last allocated
                 // word
    wave_word mask = (1UL << length) - 1;
    weight += popcount(e->r1[i] & mask);
  }
  return weight;
}

/**
 * Compute the sum of vectors a = a + b in-place (overwriting a with the
 * result). Assumes a->size = b->size.
 * @param a a valid pointer to a vector (which will be modified)
 * @param b a valid pointer to a vector
 */
static inline void vf3_vector_add_inplace(vf3_e *a, vf3_e const *b) {
  for (int i = 0; i < a->alloc; i++) {
    wave_word tmp = a->r1[i] ^ b->r0[i];
    a->r1[i] = (a->r0[i] ^ b->r1[i]) & tmp;
    a->r0[i] = (a->r0[i] ^ b->r0[i]) | (tmp ^ b->r1[i]);
  }
}

/**
 * Compute the sum of vectors c = a + b.
 * Assumes a->size = b->size = c->size.
 * @param c a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param a a valid pointer to a vector
 * @param b a valid pointer to a vector
 */
static inline void vf3_vector_add(vf3_e *c, vf3_e const *a, vf3_e const *b) {
  for (int i = 0; i < a->alloc; i++) {
    wave_word tmp = a->r1[i] ^ b->r0[i];
    c->r1[i] = (a->r0[i] ^ b->r1[i]) & tmp;
    c->r0[i] = (a->r0[i] ^ b->r0[i]) | (tmp ^ b->r1[i]);
  }
}

/**
 * Compute the negative of a vector: a = -b.
 * Assumes a->size = b->size.
 * @param a a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param b a valid pointer to the vector to be negated
 */
static inline void vf3_vector_neg(vf3_e *a, vf3_e const *b) {
  // assume b->size = a->size
  memcpy(a->r0, b->r0, a->alloc * sizeof(wave_word));
  for (int i = 0; i < b->alloc; i++) {
    a->r1[i] = b->r1[i] ^ b->r0[i];
  }
}

/**
 * Negate a vector in-place.
 * @param b a valid pointer to a vector (which will be modified to hold its own
 * negative)
 */
static inline void vf3_vector_neg_inplace(vf3_e *b) {
  for (int i = 0; i < b->alloc; i++) {
    b->r1[i] = b->r1[i] ^ b->r0[i];
  }
}

/**
 * Compute the difference of two vectors: c = a - b.
 * Assumes a->size = b->size = c->size.
 * @param c a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param a a valid pointer to a vector
 * @param b a valid pointer to a vector
 */
static inline void vf3_vector_sub(vf3_e *c, const vf3_e *a, const vf3_e *b) {
  for (int i = 0; i < a->alloc; i++) {
    wave_word tmp = a->r0[i] ^ b->r0[i];
    c->r0[i] = tmp | ((a->r1[i] ^ b->r1[i]));
    c->r1[i] = (tmp ^ b->r1[i]) & (a->r1[i] ^ b->r0[i]);
  }
}

/**
 * Subtract one vector from another in-place: a = a - b.
 * Assumes a->size = b->size.
 * @param a a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param b a valid pointer to a vector
 */
static inline void vf3_vector_sub_inplace(vf3_e *a, const vf3_e *b) {
  for (int i = 0; i < a->alloc; i++) {
    wave_word tmp = a->r0[i] ^ b->r0[i];
    a->r0[i] = tmp | (a->r1[i] ^ b->r1[i]);
    a->r1[i] = (tmp ^ b->r1[i]) & (a->r1[i] ^ b->r0[i]);
  }
}

/**
 * Compute the component-wise product of vectors: res_i = x_i * y_i for all i.
 * Assumes res->size = x->size = y->size.
 * @param res a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param x a valid pointer to a vector
 * @param y a valid pointer to a vector
 */
static inline void vf3_vector_mul(vf3_e *res, const vf3_e *x, const vf3_e *y) {
  for (int i = 0; i < x->alloc; i++) {
    res->r0[i] = x->r0[i] & y->r0[i];
    res->r1[i] = (x->r1[i] ^ y->r1[i]) & res->r0[i];
  }
}

/**
 * Compute the component-wise product of vectors in-place: x_i = x_i * y_i for
 * all i. Assumes x->size = y->size.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param y a valid pointer to a vector
 */
static inline void vf3_vector_mul_inplace(vf3_e *x, const vf3_e *y) {
  for (int i = 0; i < x->alloc; i++) {
    x->r0[i] = x->r0[i] & y->r0[i];
    x->r1[i] = (x->r1[i] ^ y->r1[i]) & x->r0[i];
  }
}

/**
 * Multiply a vector by a scalar: res = coeff*x.
 * Assumes res->size = x->size.
 * @param res a valid pointer to a vector (which will be modified to hold the
 * resultt)
 * @param coeff an integer in {0,1,2} representing a single trit
 * @param x a valid pointer to a vector
 */
static inline void vf3_vector_scalarmul(vf3_e *res, const uint8_t coeff,
                                        const vf3_e *x) {
  wave_word mask0 = -(coeff > 0);
  wave_word mask1 = -(coeff > 1);
  for (int i = 0; i < x->alloc; i++) {
    res->r0[i] = x->r0[i] & mask0;
    res->r1[i] = (x->r1[i] ^ mask1) & res->r0[i];
  }
}

/**
 * Multiply a vector by a scalar in-place: x = coeff*x.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * resultt)
 * @param coeff an integer in {0,1,2} representing a single trit
 */
static inline void vf3_vector_scalarmul_inplace(vf3_e *x, const uint8_t coeff) {
  wave_word mask0 = -(coeff > 0);
  wave_word mask1 = -(coeff > 1);
  for (int i = 0; i < x->alloc; i++) {
    x->r0[i] = x->r0[i] & mask0;
    x->r1[i] = (x->r1[i] ^ mask1) &
               x->r0[i];  // watch out, x->r0[i] was modified, we mean that
  }
}

/**
 * Multiply a vector by a nonzero scalar in-place: x = coeff*x.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * resultt)
 * @param coeff an integer in {1,2} representing a single nonzero trit
 */
static inline void vf3_vector_scalarmul_nonzero_inplace(vf3_e *x,
                                                        const uint8_t coeff) {
  wave_word mask1 = -(coeff > 1);
  for (int i = 0; i < x->alloc; i++) {
    x->r1[i] = (x->r1[i] ^ mask1) & x->r0[i];
  }
}

/**
 * Add a multiple of a vector to another, in-place: x = x + coeff*y.
 * Assumes x->size = y->size.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param coeff an integer in {0,1,2} representing a single trit
 * @param y a valid pointer to a vector
 */
static inline void vf3_vector_add_multiple_inplace(vf3_e *x,
                                                   const uint8_t coeff,
                                                   const vf3_e *y) {
  wave_word mask0 = -(coeff > 0);
  wave_word mask1 = -(coeff > 1);
  for (int i = 0; i < x->alloc; i++) {
    wave_word y0 = y->r0[i] & mask0;
    wave_word y1 = (y->r1[i] ^ mask1) & y0;
    wave_word tmp = x->r1[i] ^ y0;
    x->r1[i] = (x->r0[i] ^ y1) & tmp;
    x->r0[i] = (x->r0[i] ^ y0) | (tmp ^ y1);
  }
}

/**
 * Add a multiple of a vector to another, in-place: x = x + coeff*y.
 * MAY IGNORE ALL INDICES < start (useful when these entries are known to be
 * zero).
 * @param x a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param coeff an integer in {0,1,2} representing a single trit
 */
static inline void vf3_vector_slice_add_multiple_inplace(vf3_e *x,
                                                         const uint8_t coeff,
                                                         const vf3_e *y,
                                                         size_t start) {
  wave_word mask0 = -(coeff > 0);
  wave_word mask1 = -(coeff > 1);
  for (int i = start / WORD_LENGTH; i < x->alloc; i++) {
    wave_word y0 = y->r0[i] & mask0;
    wave_word y1 = (y->r1[i] ^ mask1) & y0;
    wave_word tmp = x->r1[i] ^ y0;
    x->r1[i] = (x->r0[i] ^ y1) & tmp;
    x->r0[i] = (x->r0[i] ^ y0) | (tmp ^ y1);
  }
}

/**
 * Add the componentwise product of two vectors to another vector, in-place: x_i
 * = x_i + a_i*y_i for all i. Assumes x->size = a->size = y->size.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param a a valid pointer to a vector
 * @param y a valid pointer to a vector
 */
static inline void vf3_vector_add_mul_inplace(vf3_e *x, const vf3_e *a,
                                              const vf3_e *y) {
  for (int i = 0; i < x->alloc; i++) {
    wave_word y0 = y->r0[i] & a->r0[i];
    wave_word y1 = (y->r1[i] ^ a->r1[i]) & y0;
    wave_word tmp = x->r1[i] ^ y0;
    x->r1[i] = (x->r0[i] ^ y1) & tmp;
    x->r0[i] = (x->r0[i] ^ y0) | (tmp ^ y1);
  }
}

/**
 * Subtract the componentwise product of two vectors from another vector,
 * in-place: x_i = x_i - a_i*y_i for all i. Assumes x->size = a->size = y->size.
 * @param x a valid pointer to a vector (which will be modified to hold the
 * result)
 * @param a a valid pointer to a vector
 * @param y a valid pointer to a vector
 */
static inline void vf3_vector_sub_mul_inplace(vf3_e *x, const vf3_e *a,
                                              const vf3_e *y) {
  for (int i = 0; i < x->alloc; i++) {
    wave_word y0 = y->r0[i] & a->r0[i];
    wave_word y1 = (y->r1[i] ^ a->r1[i]) & y0;
    wave_word tmp = x->r0[i] ^ y0;
    x->r0[i] = tmp | (x->r1[i] ^ y1);
    x->r1[i] = (tmp ^ y1) & (x->r1[i] ^ y0);
  }
}

#endif  // WAVE2_VF3_H
