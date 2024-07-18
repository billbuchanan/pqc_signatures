/******************************************************************************
 * Tools for FGLM algorithm
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Jean-Charles Faugère
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vox_params.h"

#if (VOX_Q_BITS <= 8)
typedef uint8_t GFq;
typedef uint16_t GFq_2;
#define SIZE_GFQ 1
#elif (VOX_Q_BITS <= 16)
typedef uint16_t GFq;
typedef uint32_t GFq_2;
#define SIZE_GFQ 2
#else
#error "Unsupported parameters"
#endif

typedef uint32_t GFq_4;


static inline uint32_t NORMAL(uint32_t x) {
  /* Barrett inverse 2^32 / Q */
#if   (VOX_Q == 251)
#define VOX_QINV  UINT32_C(17111423)
#elif (VOX_Q == 1021)
#define VOX_QINV  UINT32_C(4206628)
#elif (VOX_Q == 4093)
#define VOX_QINV  UINT32_C(1049344)
#else
#error "Unsupported parameters"
#endif
  /* Compiler should be able to optimize this as a high multiplication
   * on 32-bit platforms */
  uint32_t m = (uint32_t) (((uint64_t) x * (uint64_t) VOX_QINV) >> 32);
  m = x - VOX_Q * m;
  /* Result is currently in [0, 2*Q) -> perform conditional subtraction */
  m -= VOX_Q;
  uint32_t mask = - (m >> 31);
  m += VOX_Q & mask;
  return m;
}

static inline GFq_2 mul_GFq_2(GFq_2 a,GFq_2 b) {
  const GFq_4 _tmp=a*b;
  return (GFq_2)NORMAL(_tmp);
}
static inline GFq mul_GFq(GFq a,GFq b) {
  const GFq_2 c=a*b;
  return (GFq)(NORMAL(c));
}
static inline GFq_2 add_GFq_2(GFq_2 a,GFq_2 b)
{
  const GFq_4 _tmp=a+b;
  if (_tmp>=VOX_Q)
    return (GFq_2)(_tmp-VOX_Q);
  else
    return _tmp;
}
static inline GFq_2 neg_GFq_2(GFq_2 a) { if (a) return VOX_Q-a; return 0;}
#define lin_GFq_2(reg1,idx,reg2,reg3) (reg1)[(idx)]=add_GFq_2((reg1)[(idx)],mul_GFq_2(reg2,reg3))

#include "tables/invmod_table.h"
GFq_2 inv_GFq_2(GFq_2 q)
{
  return invmod_table[q];
}

#include <immintrin.h>
typedef __m256i const *m256_ptr;
/* pointer to a memory location that can hold constant integer values; the address must be 32-byte aligned */
#if SIZE_GFQ == 2
static GFq main_comp_internal(GFq* a,GFq* b,const uint32_t n)
{
  __m256i A=_mm256_setzero_si256();
  for(int32_t i=n-16;i>=0;i-=16)
    {
      __builtin_prefetch(a+i-16);
      __builtin_prefetch(b+i-16);

      __m256i x=_mm256_load_si256((m256_ptr)(a+i));  /* <a0,a1,a2,a3,...,a16> */
      __m256i y=_mm256_load_si256((m256_ptr)(b+i));  /* <b0,b1,b2,b3,...,b16> */
      y=_mm256_madd_epi16(x,y);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a16*b16> */
      A=_mm256_add_epi32(A,y);
    }
  {
    __m128i l = _mm256_extracti128_si256(A, 0);
    __m128i h = _mm256_extracti128_si256(A, 1);
    l = _mm_add_epi32(l, h);
    l = _mm_hadd_epi32(l, l);
    uint32_t z=_mm_extract_epi32(l, 0) + _mm_extract_epi32(l, 1);
    return NORMAL(z);
  }
}
#else /*  SIZE_GFQ == 2 */
static GFq main_comp_internal(GFq* a,GFq* b,const uint32_t n)
{
  const __m256i izero = _mm256_setzero_si256();
  __m256i A=izero;
  uint32_t i=0;
  __m256i x=_mm256_load_si256((m256_ptr)(a+i));  /* <a0,a1,a2,a3,...,a15> */
  __m256i y=_mm256_load_si256((m256_ptr)(b+i));  /* <b0,b1,b2,b3,...,b15> */
  __m256i X=_mm256_load_si256((m256_ptr)(a+32));
  __m256i Y=_mm256_load_si256((m256_ptr)(b+32));
  {
    __m256i xh = _mm256_unpackhi_epi8(x, izero);
    __m256i yh = _mm256_unpackhi_epi8(y, izero);
    yh=_mm256_madd_epi16(xh,yh);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
    A=_mm256_add_epi32(A,yh);
    __m256i xl = _mm256_unpacklo_epi8(x, izero);
    __m256i yl = _mm256_unpacklo_epi8(y, izero);
    yl=_mm256_madd_epi16(xl,yl);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
    A=_mm256_add_epi32(A,yl);
  }
  for(i=32;i<n-32;i+=32)
    {
      x=X;
      y=Y;
      X=_mm256_load_si256((m256_ptr)(a+i+32));  /* <a0,a1,a2,a3,...,a15> */
      Y=_mm256_load_si256((m256_ptr)(b+i+32));  /* <b0,b1,b2,b3,...,b15> */
      {
	__m256i xh = _mm256_unpackhi_epi8(x, izero);
	__m256i yh = _mm256_unpackhi_epi8(y, izero);
	yh=_mm256_madd_epi16(xh,yh);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
	A=_mm256_add_epi32(A,yh);
	__m256i xl = _mm256_unpacklo_epi8(x, izero);
	__m256i yl = _mm256_unpacklo_epi8(y, izero);
	yl=_mm256_madd_epi16(xl,yl);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
	A=_mm256_add_epi32(A,yl);
      }
    }
  x=X;
  y=Y;
  {
    __m256i xh = _mm256_unpackhi_epi8(x, izero);
    __m256i yh = _mm256_unpackhi_epi8(y, izero);
    yh=_mm256_madd_epi16(xh,yh);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
    A=_mm256_add_epi32(A,yh);
    __m256i xl = _mm256_unpacklo_epi8(x, izero);
    __m256i yl = _mm256_unpacklo_epi8(y, izero);
    yl=_mm256_madd_epi16(xl,yl);            /* <a0*b0+a1*b1,a2*b2+a3*b3,a4*b4+a5*b5,a6*b6+a7*b7> */
    A=_mm256_add_epi32(A,yl);
  }
  {
    __m128i l = _mm256_extracti128_si256(A, 0);
    __m128i h = _mm256_extracti128_si256(A, 1);
    l = _mm_add_epi32(l, h);
    l = _mm_hadd_epi32(l, l);
    uint32_t z=_mm_extract_epi32(l, 0) + _mm_extract_epi32(l, 1);
    return NORMAL(z);
  }
}
#endif /*  SIZE_GFQ == 2 */
