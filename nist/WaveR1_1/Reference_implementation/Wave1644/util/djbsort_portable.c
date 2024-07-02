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

#include <stdint.h>
#include <stdlib.h>

/* This file directly derives fron Dan Benstein's portable C code
 * found in the djbsort library available at https://sorting.cr.yp.to/
 * The "32" files are essentially a verbatim from the original library
 * (portable), while the "64" files are a straightforward
 * generalization from 32 bits type to 64 bits type
 */

/* from djbsort-20190516/h-internal/int32_minmax.c */
#define int32_MINMAX(a, b) \
  do {                     \
    int32_t ab = b ^ a;    \
    int32_t c = b - a;     \
    c ^= ab & (c ^ b);     \
    c >>= 31;              \
    c &= ab;               \
    a ^= c;                \
    b ^= c;                \
  } while (0)

/* from djbsort-20190516/int32/portable3/sort.c */
void int32_sort(int32_t *x, long long n) {
  long long top, p, q, r, i;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top; p > 0; p >>= 1) {
    for (i = 0; i < n - p; ++i)
      if (!(i & p)) int32_MINMAX(x[i], x[i + p]);
    i = 0;
    for (q = top; q > p; q >>= 1) {
      for (; i < n - q; ++i) {
        if (!(i & p)) {
          int32_t a = x[i + p];
          for (r = q; r > p; r >>= 1) int32_MINMAX(a, x[i + r]);
          x[i + p] = a;
        }
      }
    }
  }
}

/* from djbsort-20190516/int32/portable4/sort.c */
void int32_sort_4(int32_t *x, long long n) {
  long long top, p, q, r, i, j;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top; p >= 1; p >>= 1) {
    i = 0;
    while (i + 2 * p <= n) {
      for (j = i; j < i + p; ++j) int32_MINMAX(x[j], x[j + p]);
      i += 2 * p;
    }
    for (j = i; j < n - p; ++j) int32_MINMAX(x[j], x[j + p]);

    i = 0;
    j = 0;
    for (q = top; q > p; q >>= 1) {
      if (j != i)
        for (;;) {
          if (j == n - q) goto done;
          int32_t a = x[j + p];
          for (r = q; r > p; r >>= 1) int32_MINMAX(a, x[j + r]);
          x[j + p] = a;
          ++j;
          if (j == i + p) {
            i += 2 * p;
            break;
          }
        }
      while (i + p <= n - q) {
        for (j = i; j < i + p; ++j) {
          int32_t a = x[j + p];
          for (r = q; r > p; r >>= 1) int32_MINMAX(a, x[j + r]);
          x[j + p] = a;
        }
        i += 2 * p;
      }
      /* now i + p > n - q */
      j = i;
      while (j < n - q) {
        int32_t a = x[j + p];
        for (r = q; r > p; r >>= 1) int32_MINMAX(a, x[j + r]);
        x[j + p] = a;
        ++j;
      }

    done:;
    }
  }
}

/* from djbsort-20190516/uint32/useint32/sort.c */
void uint32_sort(uint32_t *x, long long n) {
  long long j;
  for (j = 0; j < n; ++j) x[j] ^= 0x80000000;
  int32_sort((int32_t *)x, n);
  for (j = 0; j < n; ++j) x[j] ^= 0x80000000;
}

/* from djbsort-20190516/h-internal/int32_minmax.c modified to 64 bits words */
#define int64_MINMAX(a, b) \
  do {                     \
    int64_t ab = b ^ a;    \
    int64_t c = b - a;     \
    c ^= ab & (c ^ b);     \
    c >>= 63;              \
    c &= ab;               \
    a ^= c;                \
    b ^= c;                \
  } while (0)

/* from djbsort-20190516/int32/portable3/sort.c modified to 64 bits words */
void int64_sort(int64_t *x, long long n) {
  long long top, p, q, r, i;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top; p > 0; p >>= 1) {
    for (i = 0; i < n - p; ++i)
      if (!(i & p)) int64_MINMAX(x[i], x[i + p]);
    i = 0;
    for (q = top; q > p; q >>= 1) {
      for (; i < n - q; ++i) {
        if (!(i & p)) {
          uint64_t a = x[i + p];
          for (r = q; r > p; r >>= 1) int64_MINMAX(a, x[i + r]);
          x[i + p] = a;
        }
      }
    }
  }
}

/* from djbsort-20190516/int32/portable4/sort.c modified to 64 bits words */
void int64_sort_4(int64_t *x, long long n) {
  long long top, p, q, r, i, j;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top; p >= 1; p >>= 1) {
    i = 0;
    while (i + 2 * p <= n) {
      for (j = i; j < i + p; ++j) int64_MINMAX(x[j], x[j + p]);
      i += 2 * p;
    }
    for (j = i; j < n - p; ++j) int64_MINMAX(x[j], x[j + p]);

    i = 0;
    j = 0;
    for (q = top; q > p; q >>= 1) {
      if (j != i)
        for (;;) {
          if (j == n - q) goto done;
          int64_t a = x[j + p];
          for (r = q; r > p; r >>= 1) int64_MINMAX(a, x[j + r]);
          x[j + p] = a;
          ++j;
          if (j == i + p) {
            i += 2 * p;
            break;
          }
        }
      while (i + p <= n - q) {
        for (j = i; j < i + p; ++j) {
          int64_t a = x[j + p];
          for (r = q; r > p; r >>= 1) int64_MINMAX(a, x[j + r]);
          x[j + p] = a;
        }
        i += 2 * p;
      }
      /* now i + p > n - q */
      j = i;
      while (j < n - q) {
        int64_t a = x[j + p];
        for (r = q; r > p; r >>= 1) int64_MINMAX(a, x[j + r]);
        x[j + p] = a;
        ++j;
      }

    done:;
    }
  }
}

/* from djbsort-20190516/uint32/useint32/sort.c modified to 64 bits words */
void uint64_sort(uint64_t *x, long long n) {
  long long j;
  for (j = 0; j < n; ++j) x[j] ^= 0x8000000000000000;
  int64_sort((int64_t *)x, n);
  for (j = 0; j < n; ++j) x[j] ^= 0x8000000000000000;
}
