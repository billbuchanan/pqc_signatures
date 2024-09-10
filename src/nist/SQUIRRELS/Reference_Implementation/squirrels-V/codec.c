/*
 * Encoding/decoding of keys and signatures.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2017-2019  Falcon Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@nccgroup.com>
 */

#include "inner.h"
#include <stdint.h>
#include <stdio.h>

/* see inner.h */
int Zf(ui32_encode)(uint8_t *out, const uint32_t *in, size_t len) {
  /*
   * We enforce little-endian representation.
   */
  for (size_t i = 0; i < len; i++) {
    int32_t n = in[i];

    out[4 * i] = n & 0xFF;
    out[4 * i + 1] = (n >> 8) & 0xFF;
    out[4 * i + 2] = (n >> 16) & 0xFF;
    out[4 * i + 3] = (n >> 24) & 0xFF;
  }

  return 4 * len;
}

/* see inner.h */
int Zf(i32_encode)(uint8_t *out, const int32_t *in, size_t len) {
  /*
   * We enforce little-endian representation.
   */
  for (size_t i = 0; i < len; i++) {
    int32_t n = in[i];
    int sign = n < 0;
    if (sign) {
      n += 1u << 31;
    }

    out[4 * i] = n & 0xFF;
    out[4 * i + 1] = (n >> 8) & 0xFF;
    out[4 * i + 2] = (n >> 16) & 0xFF;
    out[4 * i + 3] = ((n >> 24) & 0xFF) | (sign << 7);
  }

  return 4 * len;
}

/* see inner.h */
size_t Zf(trim_i8_encode)(void *out, size_t max_out_len, const int32_t *x,
                          unsigned bits) {
  size_t u, out_len;
  int minv, maxv;
  uint8_t *buf;
  uint32_t acc, mask;
  unsigned acc_len;

  maxv = (1 << (bits - 1)) - 1;
  minv = -maxv;
  for (u = 0; u < SQUIRRELS_D; u++) {
    if (x[u] < minv || x[u] > maxv) {
      return 0;
    }
  }
  out_len = ((SQUIRRELS_D * bits) + 7) >> 3;
  if (out == NULL) {
    return out_len;
  }
  if (out_len > max_out_len) {
    return 0;
  }
  buf = out;
  acc = 0;
  acc_len = 0;
  mask = ((uint32_t)1 << bits) - 1;
  for (u = 0; u < SQUIRRELS_D; u++) {
    acc = (acc << bits) | ((uint8_t)((int8_t)x[u]) & mask);
    acc_len += bits;
    while (acc_len >= 8) {
      acc_len -= 8;
      *buf++ = (uint8_t)(acc >> acc_len);
    }
  }
  if (acc_len > 0) {
    *buf++ = (uint8_t)(acc << (8 - acc_len));
  }
  return out_len;
}

/* see inner.h */
size_t Zf(trim_i8_decode)(int8_t *x, unsigned bits, void *in,
                          size_t max_in_len) {
  size_t in_len;
  const uint8_t *buf;
  size_t u;
  uint32_t acc, mask1, mask2;
  unsigned acc_len;

  in_len = ((SQUIRRELS_D * bits) + 7) >> 3;
  if (in_len > max_in_len) {
    return 0;
  }
  buf = in;
  u = 0;
  acc = 0;
  acc_len = 0;
  mask1 = ((uint32_t)1 << bits) - 1;
  mask2 = (uint32_t)1 << (bits - 1);
  while (u < SQUIRRELS_D) {
    acc = (acc << 8) | *buf++;
    acc_len += 8;
    while (acc_len >= bits && u < SQUIRRELS_D) {
      uint32_t w;

      acc_len -= bits;
      w = (acc >> acc_len) & mask1;
      w |= -(w & mask2);
      if (w == -mask2) {
        /*
         * The -2^(bits-1) value is forbidden.
         */
        return 0;
      }
      x[u++] = (int8_t) * (int32_t *)&w;
    }
  }
  if ((acc & (((uint32_t)1 << acc_len) - 1)) != 0) {
    /*
     * Extra bits in the last byte must be zero.
     */
    return 0;
  }
  return in_len;
}

/* see inner.h */
size_t Zf(comp_encode)(void *out, size_t max_out_len, const discrete_vector *x,
                       unsigned rate) {
  uint8_t *buf;
  size_t u, v;
  uint32_t acc;
  unsigned acc_len;

  buf = out;

  /*
   * Make sure that all values are within a small range.
   */
  int32_t maxval =
      (1 << rate) * 16; // After removing the lowest rate bits, the value must
                        // not be too big (< 16), so that our computations fit
                        // in a 32 bits integer Also assumes that rate <= 8
  for (u = 0; u < SQUIRRELS_D; u++) {
    if (x->coeffs[u] < -maxval || x->coeffs[u] > maxval) {
      return 0;
    }
  }

  acc = 0;
  acc_len = 0;
  v = 0;
  for (u = 0; u < SQUIRRELS_D; u++) {
    int t;
    unsigned w;

    /*
     * Get sign and absolute value of next integer; push the
     * sign bit.
     */
    acc <<= 1;
    t = x->coeffs[u];
    if (t < 0) {
      t = -t;
      acc |= 1;
    }
    w = (unsigned)t;

    /*
     * Push the low bits of the absolute value.
     */
    acc <<= rate;
    acc |= w & ((1u << rate) - 1u);
    w >>= rate;

    /*
     * We pushed exactly 8 bits.
     */
    acc_len += rate + 1u;

    /*
     * Push as many zeros as necessary, then a one. Since the
     * absolute value is bounded by an appropriate value, we can only range up
     * to 15 at this point, thus we will add at most 16 bits here. With the
     * rate+1 bits above and possibly up to 7 bits from previous iterations, we
     * may go up to 24+rate bits, which will fit in the accumulator, which is an
     * uint32_t.
     */
    acc <<= (w + 1);
    acc |= 1;
    acc_len += w + 1;

    /*
     * Produce all full bytes.
     */
    while (acc_len >= 8) {
      acc_len -= 8;
      if (buf != NULL) {
        if (v >= max_out_len) {
          return 0;
        }
        buf[v] = (uint8_t)(acc >> acc_len);
      }
      v++;
    }
  }

  /*
   * Flush remaining bits (if any).
   */
  if (acc_len > 0) {
    if (buf != NULL) {
      if (v >= max_out_len) {
        return 0;
      }
      buf[v] = (uint8_t)(acc << (8 - acc_len));
    }
    v++;
  }

  return v;
}

/* see inner.h */
size_t Zf(comp_decode)(discrete_vector *x, const void *in, size_t max_in_len,
                       unsigned rate) {
  const uint8_t *buf;
  size_t u, v;
  uint32_t acc;
  unsigned acc_len;

  buf = in;
  acc = 0;
  acc_len = 0;
  v = 0;

  unsigned maxval =
      (1 << rate) * 16; // After removing the lowest rate bits, the value must
                        // not be too big (< 16), so that our computations fit
                        // in a 32 bits integer Also assumes that rate <= 8

  for (u = 0; u < SQUIRRELS_D; u++) {
    unsigned b, s, m;

    /*
     * Get next eight bits: sign and low seven bits of the
     * absolute value.
     */
    if (acc_len < rate + 1) { // not enough bits left in the accumulator
      if (v >= max_in_len) {
        return 0;
      }

      acc = (acc << 8) | (uint32_t)buf[v++];
      acc_len += 8;
    }

    b = acc >> (acc_len - (rate + 1));
    acc_len -= rate + 1;
    s = b & (1u << rate);
    m = b & ((1u << rate) - 1u);

    /*
     * Get next bits until a 1 is reached.
     */
    for (;;) {
      if (acc_len == 0) {
        if (v >= max_in_len) {
          return 0;
        }
        acc = (acc << 8) | (uint32_t)buf[v++];
        acc_len = 8;
      }
      acc_len--;
      if (((acc >> acc_len) & 1) != 0) {
        break;
      }
      m += 1 << rate;
      if (m >= maxval) {
        return 0;
      }
    }

    /*
     * "-0" is forbidden.
     */
    if (s && m == 0) {
      return 0;
    }

    x->coeffs[u] = (int16_t)(s ? -(int)m : (int)m);
  }

  /*
   * Unused bits in the last byte must be zero.
   */
  if ((acc & ((1u << acc_len) - 1u)) != 0) {
    return 0;
  }

  return v;
}
