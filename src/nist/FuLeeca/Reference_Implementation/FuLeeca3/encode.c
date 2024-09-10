/*
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
 */

// Compression and decompression apdated from Falcon (falcon-sign.info)

#include "encode.h"

/*************************************************
* Name:        encode_sig
*
* Description: Compresses the coefficients of the input polynomial
*              by concatenating the sign bit with an uncompressed tail
*              and appending the head in 0^k encoding.
*
* Arguments:   - const s_coeff *in: pointer to input polynomial
*              - size_t *outlen:    size of the compressed signature
*              - uint8_t *out:      pointer to the compressed signature
**************************************************/
void encode_sig(uint8_t *out, size_t *outlen, const s_coeff *in)
{
    size_t i;
    size_t j = 0;
    uint32_t tmp;
    uint32_t tmp_len;
    int16_t t;
    uint16_t w;

    /* Check for proper ranges */
    for (i=0; i<N/2; i++) {
        if (in[i] < -(P-1)/2  || in[i] > (P-1)/2) {
            return;
        }
    }

    tmp = 0;
    tmp_len = 0;

    for (i=0; i<N/2; i++) {
        t = in[i]; 

        /* Store sign */
        tmp <<= 1;
        if (t < 0) {
            t = -t;
            tmp |= 1;
        }
        w = (uint16_t) t;

        /* Store the lower tail bits assuming they are close to random */
        tmp <<= COMP_TAIL;
        tmp |= w & ((1 << COMP_TAIL) - 1);
        tmp_len += COMP_TAIL + 1;
        w >>= COMP_TAIL;

        /* Now store the remaining upper part */
        tmp <<= (w + 1);
        tmp |= 1;
        tmp_len += w + 1;

        /* Fill the full bytes */
        while (tmp_len >= 8) {
            tmp_len -= 8;
            if (out != NULL) {
                if (j >= CRYPTO_BYTES - SALT_BYTES) {
                    return;
                }
                out[j] = (uint8_t)(tmp >> tmp_len);
            }
            j++;
        }
    }
    /* Fill the last partial byte */
    if (tmp_len > 0) {
        if (out != NULL) {
            if (j >= CRYPTO_BYTES - SALT_BYTES) {
                return;
            }
            out[j] = (uint8_t)(tmp << (8-tmp_len));
        }
        j++;
    }

    /* Padd to CRYTPO_BYTES */
    while (j < CRYPTO_BYTES - SALT_BYTES) {
        if (out != NULL) {
            out[j++] = 0u;
        }
    }
    *outlen = j;
    return;
}


/*************************************************
* Name:        decode_sig
*
* Description: De-compresses the signature
*
* Arguments:   - const uint8_t *in:     to compressed signature
*              - s_coeff *out:          pointer to de-compressed signature polynomial
**************************************************/
void decode_sig(s_coeff *out, const uint8_t *in)
{
    uint32_t tmp;
    uint32_t tmp_len;
    size_t j = 0;
    size_t i;
    uint32_t bits, s, m;

    tmp = 0;
    tmp_len = 0;

    for (i=0; i<N/2; i++) {

        if (j >= CRYPTO_BYTES - SALT_BYTES) {
            return;
        }

        /* Get first tail bits and sign */
        /* if tmp_len < tail + sign, we dont have sign and message available, so get another byte */
        while (tmp_len < COMP_TAIL + 1) {
            tmp = (tmp << 8) | (uint32_t) in[j++];
            tmp_len += 8;
        }

        tmp_len -= COMP_TAIL + 1;
        bits = tmp >> tmp_len;
        s = bits & (1 << COMP_TAIL);
        m = bits & ((1 << COMP_TAIL) - 1);

        /* Search for the 1 */ 
        for (;;) {
            if (tmp_len == 0) {
                if (j >= CRYPTO_BYTES - SALT_BYTES) {
                    return;
                }
                tmp = (tmp << 8) | (uint32_t) in[j++];
                tmp_len = 8;
            }
            tmp_len--;
            if (((tmp >> tmp_len) & 1) != 0) {
                break;
            }
            m += (1 << COMP_TAIL);
            if (m > (P-1) / 2) {
                return;
            }
        }
        out[i] = (s_coeff) m;
        if (s) {
            out[i] = (s_coeff) - out[i];
        }
    }
    return;
}
