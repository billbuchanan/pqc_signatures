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
#elif (VOX_Q_BITS <= 16)
typedef uint16_t GFq;
typedef uint32_t GFq_2;
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


static GFq main_comp_internal(GFq* a,GFq* b,const uint32_t n)
{
  GFq_4 A=0;
  for(uint32_t j=0;j<n;j++)
    A+=a[j]*b[j];
  return NORMAL(A);
}

