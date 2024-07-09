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

#include "tritstream.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int ts_init(tritstream_t *ts, uint8_t *buf, size_t buf_len) {
  ts->data = buf;
  ts->buf_len = buf_len;
  ts->byte_pos = 0;
  ts->factor = 1;

  return 0;
}

int ts_write(tritstream_t *ts, uint8_t data) {
  //  printf("len: %i\n", data_len);
  if (ts->byte_pos + ((ts->factor * 3) > 245 ? 1 : 0) >= ts->buf_len) {
    fprintf(stderr, "write exceeds buffer!\n");
    exit(-1);
    // return -1;
  }

  ts->data[ts->byte_pos] += data * ts->factor;

  ts->factor *= 3;

  if (ts->factor > 240) {
    ts->byte_pos++;
    ts->factor = 1;
  }

  return 0;
}

uint8_t ts_read(tritstream_t *ts) {
  if (ts->byte_pos + ((ts->factor * 3) > 245 ? 1 : 0) >= ts->buf_len) return -1;

  uint8_t data = (ts->data[ts->byte_pos] / ts->factor) % 3;

  ts->factor *= 3;

  if (ts->factor > 240) {
    ts->byte_pos++;
    ts->factor = 1;
  }

  return data;
  return data;
}
