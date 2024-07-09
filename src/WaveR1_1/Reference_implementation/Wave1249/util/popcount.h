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

#ifndef POPCOUNT_H
#define POPCOUNT_H

// Source: Hamming wikipedia

#define m1 0x5555555555555555
#define m2 0x3333333333333333
#define m4 0x0f0f0f0f0f0f0f0f
#define h01 0x0101010101010101

static inline int popcount(uint64_t x) {
  x -= (x >> 1) & m1;              // put count of each 2 bits into those 2 bits
  x = (x & m2) + ((x >> 2) & m2);  // put count of each 4 bits into those 4 bits
  x = (x + (x >> 4)) & m4;         // put count of each 8 bits into those 8 bits
  return (x * h01) >>
         56;  // returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}

// Alternative for machines with slow multiplication
static inline int popcount_alt(uint64_t x) {
  x -= (x >> 1) & m1;              // put count of each 2 bits into those 2 bits
  x = (x & m2) + ((x >> 2) & m2);  // put count of each 4 bits into those 4 bits
  x = (x + (x >> 4)) & m4;         // put count of each 8 bits into those 8 bits
  x += x >> 8;   // put count of each 16 bits into their lowest 8 bits
  x += x >> 16;  // put count of each 32 bits into their lowest 8 bits
  x += x >> 32;  // put count of each 64 bits into their lowest 8 bits
  return x & 0x7f;
}

static inline uint64_t popcnt64(uint64_t x) {
  __asm__("popcnt %1, %0" : "=r"(x) : "0"(x));
  return x;
}

static inline uint64_t popcnt(const void* data, uint64_t size) {
  uint64_t i = 0;
  uint64_t cnt = 0;
  const uint8_t* ptr = (const uint8_t*)data;
#if defined(HAVE_POPCNT)
  /*
   * The user has compiled without -mpopcnt.
   * Unfortunately the MSVC compiler does not have
   * a POPCNT macro so we cannot get rid of the
   * runtime check for MSVC.
   */
#if !defined(__POPCNT__)
  if (cpuid & bit_POPCNT)
#endif
  {
    /* We use unaligned memory accesses here to improve performance */
    for (; i < size - size % 8; i += 8)
      cnt += popcnt64(*(const uint64_t*)(ptr + i));
    for (; i < size; i++) cnt += popcnt64(ptr[i]);

    return cnt;
  }
#endif

#if !defined(HAVE_POPCNT) || !defined(__POPCNT__)
  /*
   * Pure integer popcount algorithm.
   * We use unaligned memory accesses here to improve performance.
   */
  for (; i < size - size % 8; i += 8)
    cnt += popcount(*(const uint64_t*)(ptr + i));

  if (i < size) {
    uint64_t val = 0;
    size_t bytes = (size_t)(size - i);
    memcpy(&val, &ptr[i], bytes);
    cnt += popcount(val);
  }

  return cnt;
#endif
}
#endif  // POPCOUNT_H
