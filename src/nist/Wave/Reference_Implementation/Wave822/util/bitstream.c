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

#include "bitstream.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int bs_init(bitstream_t *bs, uint8_t *buf, size_t buf_len) {
  bs->data = buf;
  bs->buf_len = buf_len;
  bs->byte_pos = 0;
  bs->bit_pos = 0;

  return 0;
}

int bs_write(bitstream_t *bs, uint32_t data, uint32_t data_len) {
  //  printf("len: %i\n", data_len);
  if (bs->byte_pos * 8 + bs->bit_pos + data_len > bs->buf_len * 8) {
    // fprintf(stderr, "write exceeds buffer!\n");
    // exit(-1);
    return -1;
  }

  if (bs->bit_pos + data_len < 8) {
    bs->data[bs->byte_pos] |= data << bs->bit_pos;

    bs->bit_pos += data_len;

    if (bs->bit_pos > 7) {
      bs->bit_pos = 0;
      bs->byte_pos += 1;
    }

    return 0;
  }

  if (bs->bit_pos > 0) {
    // bs->data[bs->byte_pos] |= (data >> (data_len - bs->bit_pos)) & 0xFF;
    bs->data[bs->byte_pos] |= (data << bs->bit_pos) & 0xFF;

    data >>= 8 - bs->bit_pos;
    data_len -= 8 - bs->bit_pos;

    bs->bit_pos = 0;
    bs->byte_pos += 1;
  }

  while (data_len >= 8) {
    bs->data[bs->byte_pos] = data & 0xFF;

    data >>= 8;
    data_len -= 8;

    bs->byte_pos += 1;
  }

  if (data_len > 0) {
    bs->data[bs->byte_pos] = data;  //<< (8 - data_len);

    bs->bit_pos = data_len;
  }

  return 0;
}

uint32_t bs_read(bitstream_t *bs, uint32_t data_len) {
  if (bs->byte_pos * 8 + bs->bit_pos + data_len > bs->buf_len * 8) return -1;

  uint32_t data = 0;

  //  printf("len: %i  ", data_len);

  if (bs->bit_pos + data_len < 8) {
    data = (bs->data[bs->byte_pos] >> bs->bit_pos) & ((1 << data_len) - 1);

    bs->bit_pos += data_len;

    if (bs->bit_pos > 7) {
      bs->bit_pos = 0;
      bs->byte_pos += 1;
    }

    //    printf("%b\n", data);

    return data;
  }

  uint32_t off = 0;

  if (bs->bit_pos > 0) {
    data = bs->data[bs->byte_pos] >> bs->bit_pos;  //& ((1 << bs->bit_pos) - 1);

    off = 8 - bs->bit_pos;
    data_len -= 8 - bs->bit_pos;

    bs->bit_pos = 0;
    bs->byte_pos += 1;
  }

  while (data_len >= 8) {
    // data <<= 8;
    data |= bs->data[bs->byte_pos] << off;
    off += 8;
    bs->byte_pos += 1;
    data_len -= 8;
    bs->bit_pos = 0;
  }

  if (data_len > 0) {
    // data <<= data_len;
    data |= (bs->data[bs->byte_pos] & ((1 << data_len) - 1)) << off;

    bs->bit_pos = data_len;
  }

  //  printf("%b\n", data);

  return data;
}
