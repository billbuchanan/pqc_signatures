/******************************************************************************
 * Resolution of a quadratic system in T variables
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Jean-Charles Faugère, Robin Larrieu
 *****************************************************************************/
#include "F4F5.h"
#include "vox_arith.h"
#include "FGLM.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdint.h>

#if 0
#include <assert.h>
#define VOX_ASSERT assert
#else
#define VOX_ASSERT(x)
#endif

#if   (VOX_T == 6)
#define NB_MONOM_HIGH 20
#include "tables/F4_6vars.h"
#include "tables/FGLM_6vars.h"
#elif (VOX_T == 7)
#define NB_MONOM_HIGH 35
#include "tables/F4_7vars.h"
#include "tables/FGLM_7vars.h"
#elif (VOX_T == 8)
#define NB_MONOM_HIGH 70
#include "tables/F4_8vars.h"
#include "tables/FGLM_8vars.h"
#else
#error "Invalid parameter"
#endif

#define MAX_ROW  256
#define POW2_T   (1 << VOX_T)

/*
 * Barrett reduction of any 32-bit integer modulo Q
 */
static uint32_t Fq_reduce(uint32_t x) {
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
  VOX_ASSERT(x % VOX_Q == m);
  return m;
}

static inline Fq Fq_neg(Fq a) {
  uint16_t tmp = (uint16_t) (- ((uint16_t) a));
  uint16_t mask = - (tmp >> 15); /* 0 if a == 0, 0xFFFF otherwise */
  return (Fq) (tmp + (VOX_Q & mask));
}

typedef struct {
  uint32_t *coeffs;
  int rows, cols;
} Fq_mat;

static void Fq_mat_init(Fq_mat *dst, unsigned int r, unsigned int c) {
  dst->coeffs = calloc(r*c, sizeof(uint32_t));
  dst->rows = r;
  dst->cols = c;
}

static void Fq_mat_empty(Fq_mat *dst) {
  dst->coeffs = NULL;
  dst->rows = 0;
  dst->cols = 0;
}

static void Fq_mat_clear(Fq_mat *dst) {
  /* free(NULL) is defined and does nothing, so we may safely call Fq_mat_clear
   * multiple times. This will simplify error handling. */
  free(dst->coeffs);
  Fq_mat_empty(dst);
}

#define FQ_MAT_ENTRY(a, i, j)   (a).coeffs[(i)*((a).cols) + (j)]
#define FQ_MAT_ROW(a, i, j)    &(a).coeffs[(i)*((a).cols)]


static void do_pivot_dense(Fq_mat *pB, const int16_t *pTab1, const int e0,
                           int j, int n, int m, uint8_t *use)
{
  uint32_t *rowj = pB->coeffs + j*(pB->cols) + e0;
  if (use[j]) {
    use[j]=0;
    for (int i=0; i<m; i++)
      rowj[i] = Fq_reduce(rowj[i]);
  }
  for (int i = 0; i < n; i++) {
    const int i1=pTab1[i];
    uint32_t *rowi = pB->coeffs + i1*(pB->cols);
    uint32_t x = rowi[j];
    // FIXME: not constant time !!
    if (x) {
      use[i1]=1;
      x = VOX_Q-x;
      rowi+=e0;
      for (int k = 0; k < m; k++)
        rowi[k] += x*rowj[k];
    }
  }
}


void do_pivot_sparse(Fq_mat *pB, const int16_t *pTab1, const int16_t *pTab2,
                     int j, int n, int m, uint8_t *use)
{
  uint32_t *rowj = pB->coeffs + j*(pB->cols);
  if (use[j]) {
    use[j]=0;
    for (int i=0; i<m; i++)
      rowj[pTab2[i]] = Fq_reduce(rowj[pTab2[i]]);
  }
  for (int i = 0; i < n; i++) {
    const int16_t i1 = pTab1[i];
    uint32_t *rowi = pB->coeffs + i1*(pB->cols);
    uint32_t x = rowi[j];
    // FIXME: not constant time !!
    if (x) {
      use[i1]=1;
      x = VOX_Q-x;
      for (int k = 0; k < m; k++) {
        const int16_t l = pTab2[k];
        rowi[l] += x*rowj[l];
      }
    }
  }
}

static void* realign_ptr(void* x) {
  uintptr_t ptr = (uintptr_t)(x);
  ptr = ptr+32 - (ptr%32);
  return (void*)(ptr);
}


#define eqns_ncols  (((VOX_T+1)*(VOX_T+2))/2)

int Solve_MQ(Fq sol[VOX_T], const Fq eqns[VOX_T*eqns_ncols], const Fq hint) {
  const int16_t *pTable, *pTab1, *pTab2;
  Fq_mat A, B, C, *pA, *pB, *pC;
  uint8_t use[MAX_ROW];
  int Tag, i, j, k, l, s1, s2;
  int ret = 0;

  Fq_mat_init(&A,  VOX_T, eqns_ncols);
  for (i=0; i<VOX_T*eqns_ncols; i++)
    A.coeffs[i] = eqns[i];
  /* Initialize B,C to empty matrices to prevent double free in case of early abort */
  Fq_mat_empty(&B);
  Fq_mat_empty(&C);

  pA = &A;
  pB = &B;
  pC = &C;
  pTable = F4_table;
  /* Set these only to prevent -Wmaybe-uninitalized */
  s1 = 0;
  s2 = 0;
  pTab1 = pTable;
  pTab2 = pTable;

  if (rref_block_strict(A.coeffs, eqns_ncols, 0, 0, VOX_T, eqns_ncols) != VOX_T)
    goto _end; /* singular system => abort */

  /* Start main F4 loop */
  do {
    Tag = *pTable++;
    switch (Tag) {
      case INIT_B:
        i = *pTable++;
        j = *pTable++;
        Fq_mat_init(pB, i, j);
        VOX_ASSERT(i<MAX_ROW);
        for(k=0; k<i; k++)
          use[k]=0;
        break;
      case COPY_A:
        i = *pTable++;
        s1 = *pTable++;
        pTab1 = pTable;
        pTable += s1;
        break;
      case COPY_A_B:
        j = *pTable++;
        s2 = *pTable++;
        pTab2 = pTable;
        pTable += s2;
        VOX_ASSERT(s1 == s2);
        for (k = 0; k < s1; k++)
          FQ_MAT_ENTRY(*pB, j, pTab2[k]) = FQ_MAT_ENTRY(*pA, i, pTab1[k]);
        break;
      case PIVOT_DENSE:
        j  = *pTable++;
        s1 = *pTable++;
        k  = *pTable++;
        s2 = *pTable++;
        pTab1 = pTable;
        pTable += s1;
        do_pivot_dense(pB, pTab1, k, j, s1, s2, use);
        break;
      case PIVOT_SPARSE:
        j = *pTable++;
        s1 = *pTable++;
        s2 = *pTable++;
        pTab1 = pTable;
        pTable += s1;
        pTab2 = pTable;
        pTable += s2;
        do_pivot_sparse(pB, pTab1, pTab2, j, s1, s2, use);
        break;
      case RREF:
        i = *pTable++;
        j = *pTable++;
        k = *pTable++;
        l = *pTable++;
        for (s1=i; s1<k; s1++) {
          for (s2=j; s2<l; s2++)
            FQ_MAT_ENTRY(*pB, s1, s2) = Fq_reduce(FQ_MAT_ENTRY(*pB, s1, s2));
        }
        if (rref_block_strict(pB->coeffs, pB->cols, i, j, k, l) != k)
          goto _end; /* singular system => abort */
        break;
      case INIT_C:
        i = *pTable++;
        j = *pTable++;
        Fq_mat_init(pC, i, j);
        break;
      case COPY_B_C:
        i  = *pTable++;
        s1 = *pTable++;
        j  = *pTable++;
        s2 = *pTable++;
        pTab1 = pTable;
        pTable += s1;
        pTab2 = pTable;
        pTable += s2;
        VOX_ASSERT(s1 == s2);
        if (use[j]) {
          use[j]=0;
          for (k = 0; k < s1; k++)
            FQ_MAT_ENTRY(*pC, i, pTab1[k]) = Fq_reduce(FQ_MAT_ENTRY(*pB, j, pTab2[k]));
        }
        else {
          for (k = 0; k < s1; k++)
            FQ_MAT_ENTRY(*pC, i, pTab1[k]) = FQ_MAT_ENTRY(*pB, j, pTab2[k]);
        }
        break;
      case COPY_A_C:
        i  = *pTable++;
        s1 = *pTable++;
        j  = *pTable++;
        s2 = *pTable++;
        pTab1 = pTable;
        pTable += s1;
        pTab2 = pTable;
        pTable += s2;
        VOX_ASSERT(s1 == s2);
        for (k = 0; k < s1; k++)
            FQ_MAT_ENTRY(*pC, i, pTab1[k]) = FQ_MAT_ENTRY(*pA, j, pTab2[k]);
        break;
      case END_LOOP:
        { /* (A,B,C) <- (C,A,B) */
          Fq_mat *pTemp = pA; pA = pC; pC = pB, pB = pTemp;
        }
        Fq_mat_clear(pB);
        Fq_mat_clear(pC);
        break;
      case END:
      default:
        break;
    }
  } while (Tag != END);
  /* At this point, pB, pC are already cleared */
  /* Conversion to FGLM input format */
  Fq *cfs[NB_MONOM_HIGH];
  Fq *heap = calloc(32 + NB_MONOM_HIGH*POW2_T*sizeof(Fq), 1);
  Fq *heapa = realign_ptr(heap);
  for (i=0; i<NB_MONOM_HIGH; i++) {
    cfs[i] = heapa;
    heapa += POW2_T;
    for (j=0; j<POW2_T; j++)
      cfs[i][POW2_T-1-j] = Fq_neg(FQ_MAT_ENTRY(*pA, NB_MONOM_HIGH-1-i, I2[j]));
  }
  Fq_mat_clear(pA);

  ret = W_FGLM_new(POW2_T, sparse, nosp, VOX_T, i_e, cfs, hint);
  if (ret == VOX_T) {
    ret = 1;
    for (i=0; i<VOX_T; i++)
      sol[i] = cfs[0][i];
  }
  else
    ret = 0;
  free(heap);

  _end:
  /* clear matrices used in F4 in case of early abort
   * (otherwise, these do nothing) */
  Fq_mat_clear(&A);
  Fq_mat_clear(&B);
  Fq_mat_clear(&C);
  return ret;
}
