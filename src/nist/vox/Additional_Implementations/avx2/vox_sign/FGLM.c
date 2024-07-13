/******************************************************************************
 * FGLM algorithm to change monomial ordering
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Jean-Charles Faugère, Robin Larrieu
 *****************************************************************************/
#include "FGLM.h"
#include "FGLM_gfq.h"
#include <alloca.h>

#define PRIME VOX_Q
#define ALLOCA(T,id,n) T* id=(T*)(alloca(sizeof(T)*(n)))

#ifndef __cplusplus
#define and &&
#define or ||
#define not !
#endif /* ndef __cplusplus */

static void* realign_ptr(void* x)
{
  uint64_t ptr=(uint64_t)(x);
  ptr=ptr+32-(ptr%32);
  return (void*)(ptr);
}

static void init_vector(GFq* u, int32_t n)
{
  for(int32_t i=0;i<n-1;i++)
    u[i]=0;
  u[n-1]=1;
}
static void update_samples(GFq_2 **PTs,int32_t k,const GFq *v, const int32_t *e_i,
                           const int32_t n,const int32_t nb)
{
  (void) n;
  for(int32_t i=0;i<nb-1;i++)
    if (e_i[i]>=0)
      PTs[i][k]=v[e_i[i]];
}

static uint32_t usefuls(const int32_t* nosp)
{
  int32_t k=0;
  while (1)
    {
      const int32_t i=nosp[2*k];
      if (i<0)
	return k;
      else
	k++;
    }
}
static void counters(uint32_t *pos, const int32_t *nosp)
{
  int32_t k=0;
  while (1)
    {
      const int32_t i=nosp[2*k];
      if (i<0)
	break;
      else
	{
	  pos[k]=i;
	  k++;
	}
    }
}

static void main_comp(GFq *y0, GFq *x0, const int32_t n,
                      const uint32_t *pos, const uint32_t imax, GFq **cfs)
{
  for(uint32_t j=0;j<imax;j++)
    y0[pos[j]]=main_comp_internal(cfs[j],x0,n);
}

static int32_t pol_degree(GFq_2* a,const int32_t n)
{
  for(int32_t i=n;i>=0;i--)
    if (a[i])
      return i;
  return -1;
}

static void mulop(GFq_2* A,int32_t* rdA,
                  GFq_2* B,int32_t* rdB,
                  GFq_2* U,int32_t* rdU,
                  GFq_2* V,int32_t* rdV)
{
  int32_t dA=rdA[0];
  int32_t dU=rdU[0];
  int32_t dQ=-1,dR=-1;
  int32_t dB=rdB[0];
  int32_t dV=rdV[0];

  ALLOCA(GFq_2,Rl,dA+1);
  ALLOCA(GFq_2,Ul,dV+dA-dB+1);
  ALLOCA(GFq_2,Q,dA-dB+1);
  GFq_2 ilB;
  if (dB<0)
    fprintf(stderr,"B=0 !!\n");

  ilB=inv_GFq_2(B[dB]);
  for(int32_t i=0;i<=dA;i++)
    Rl[i]=A[i];
  for(int32_t i=0;i<=dU;i++)
    Ul[i]=U[i];
  for(int32_t i=dU+1;i<=dV+dA-dB;i++)
    Ul[i]=0;
  for(int32_t i=0;i<=dA-dB;i++)
    Q[i]=0;

  for(int32_t i=dA;i>=dB;i--)
    {
      if (Rl[i])
	{
	  Q[i-dB]=mul_GFq_2(Rl[i],ilB);
	    GFq_2 a=neg_GFq_2(Q[i-dB]);
	  if (dQ<0)
	    dQ=i-dB;
	  for(int32_t j=0;j<=dB;j++)
	    lin_GFq_2(Rl,i-j,a,B[dB-j]);
	  for(int32_t j=0;j<=dV;j++)
	    lin_GFq_2(Ul,i-j+dV-dB,a,V[dV-j]);
	}
    }

  for(int32_t i=0;i<=dB;i++)
    A[i]=B[i];
  rdA[0]=dB;

  for(int32_t i=0;i<=dV;i++)
    U[i]=V[i];
  rdU[0]=dV;

  for(int32_t i=dB-1;i>=0;i--)
    {
      B[i]=Rl[i];
      if ((dR<0) and Rl[i])
	dR=i;
    }
  rdB[0]=dR;

  dU=-1;
  for(int32_t i=dA-dB+dV;i>=0;i--)
    {
      V[i]=Ul[i];
      if ((dU<0) and Ul[i])
	dU=i;
    }
  rdV[0]=dU;
}

static void fraction(GFq_2* U3, int32_t* dU3,
                     GFq_2* V3,int32_t* dV3,
                     int32_t l)
{
  int32_t du3=dU3[0];
  int32_t dv3=dV3[0];
  int32_t du2=0;
  int32_t dv2=0;
  ALLOCA(GFq_2,u3,du3+1);
  ALLOCA(GFq_2,v3,du3+1);
  ALLOCA(GFq_2,u2,du3+1);
  ALLOCA(GFq_2,v2,du3+1);
  v2[0]=1;
  u2[0]=0;

  for(int32_t i=0;i<=du3;i++)
    u3[i]=U3[i];
  for(int32_t i=0;i<=dv3;i++)
    v3[i]=V3[i];
  for(int32_t i=dv3+1;i<=du3;i++)
    v3[i]=0;

  while(dv3>=l)
    mulop(u3,&du3,v3,&dv3,u2,&du2,v2,&dv2);

  *dU3=dv3;
  for(int32_t i=0;i<=dv3;i++)
    U3[i]=v3[i];

  *dV3=dv2;
  for(int32_t i=0;i<=dv2;i++)
    V3[i]=v2[i];
}

void apply2v(GFq *y0, GFq *x0, int32_t n, const uint32_t *pos,
             const int32_t *sparse, GFq **cfs_tab, const uint32_t nb)
{
  int32_t l=0;
  while(1)
    {
      const int32_t i=sparse[l];
      const int32_t j=sparse[l+1];
      if (i>=0)
	{
	  y0[i]=x0[j];
	  l+=2;
	}
      else
	break;
    }

  main_comp(y0,x0,n,pos,nb,cfs_tab);
}

static int32_t mul_poly_mod(GFq_2* R, const GFq_2* A, int32_t dA, const GFq_2 *B, int32_t dB, int32_t D)
{
  int32_t d;
  int32_t dg=-1;
  for(d=0;d<D;d++)
    {
      /* a optimiser modulo p */
      int32_t i;
      GFq_2 al=0;
      for(i=0;i<=d;i++)
	if ((i<=dA) and ((d-i)<=dB))
	  lin_GFq_2(&al,0,A[i],B[d-i]);

      R[d]=al;
      if (R[d])
	dg=d;
    }
    return dg;
}
static void rev(GFq_2* P, const GFq_2* rP, const int32_t d)
{
  for(int32_t i=d;i>=0;i--)
    P[i]=rP[d-i];
}

#include "tables/zetas.h"
/* write i = zeta^i1 for zeta a primitive element modulo VOX_Q, and
 * use a precomputed table of all zeta^j. With this, we can save modular
 * reductions compared to Horner's method, which geatly improves
 * the exhaustive search in find_a_root */
#if 0
static uint32_t Evaluate(const GFq_2 *M, const int dM, int i) {
  int i1 = zetas_idx[i];
  int i2 = i1;
  uint32_t acc = M[0];
  for (int j=1; j<=dM; j++) {
    acc += M[j] * ((uint32_t) zetas[i2]);
    i2 += i1;
    if (i2 >= VOX_Q-1)
      i2 -= VOX_Q-1;
  }
  return acc % VOX_Q;
}
#endif /*  0 */

static GFq_2 EvaluateR(const GFq_2 *M, int i,const int dM)
{
  int i1 = zetas_idx[i];
  int i2 = i1;
  uint32_t acc = M[dM];
  for (int j=1; j<=dM; j++) {
    acc += M[dM-j] * ((uint32_t) zetas[i2]);
    i2 += i1;
    if (i2 >= VOX_Q-1)
      i2 -= VOX_Q-1;
  }
  return NORMAL(acc);
}


static void Evaluate01(GFq* u0,GFq* u1,const GFq_2 *M, int i,const int dM)
{
  const int i1 = zetas_idx[i];
  int i2 = i1;
  int j=0;
  uint32_t acc0 = M[j];
  uint32_t acc1 = M[j+1];
  for (j+=2; j<dM; j+=2)
    {
      const uint32_t z = ((uint32_t) zetas[i2]);
      acc0 += M[j]   * z;
      acc1 += M[j+1] * z;
      i2 += i1;
      if (i2 >= VOX_Q-1)
	i2 -= VOX_Q-1;
    }
  /* In our use case dM % 2 == 0 (dm is 64,128, or 256) so j == dM */
  acc0 += M[j]   * ((uint32_t) zetas[i2]);
  u0[0] = NORMAL(acc0);
  u1[0] = NORMAL(acc1);
}

static int find_a_root(GFq *sol, const GFq_2 *M, const int mod, const int dM, const Fq hint)
{
  (void) mod;
  GFq_2 out = hint + VOX_Q;
  int found = 0;
  if (M[0] == 0) {
    out = (hint==0) ? 0 : VOX_Q;
    found = 1;
  }
  for (uint32_t a = 1; a<=(VOX_Q-1)/2; a++) {
    uint32_t a2= NORMAL(a*a);
    GFq u0,u1;
    Evaluate01(&u0,&u1,M,a2,dM);

    a2 = NORMAL(u1*a);
    u1 = (a2==0) ? 0 : VOX_Q-a2;

    if (u0 == u1) { /* a is root */
      GFq_2 root = a;
      if (root < hint)
        root += VOX_Q;
      if (root < out)
        out = root;
      found = 1;
    }
    if (u0 == a2) { /* -a is root */
      GFq_2 root = VOX_Q-a;
      if (root < hint)
        root += VOX_Q;
      if (root < out)
        out = root;
      found = 1;
    }
  }
  *sol = (out < VOX_Q) ? out : (out-VOX_Q);
  return found;
}

uint32_t W_FGLM_new(int32_t n, const int32_t *sparse, const int32_t *nosp,
                    int32_t nvars, const int32_t *i_e, Fq** cfs, const Fq hint)
{
  const int32_t n0=n;
  const int32_t posxn=i_e[nvars-1];
  int32_t d0=n0;
  int32_t m=2*d0;
  int32_t i,k;

  ALLOCA(GFq,X,n+32);
  ALLOCA(GFq,Y,n+32);
  ALLOCA(GFq,PT,m+1);
  ALLOCA(GFq_2*,PTs,nvars);

  X=(GFq*)(realign_ptr(X));
  Y=(GFq*)(realign_ptr(Y));
  init_vector(Y,n);
  for(i=0;i<nvars-1;i++)
    PTs[i]=(GFq_2*)(malloc(sizeof(GFq_2)*(n+1)+32));

  PT[0]=Y[posxn];
  update_samples(PTs,0,Y,i_e,n,nvars);

  const uint32_t nbd=usefuls(nosp);
  uint32_t posnz[nbd];
  counters(posnz,nosp);
  i=1;
  apply2v(X,Y,n,posnz,sparse,cfs,nbd);
  PT[i]=X[posxn];
  update_samples(PTs,i,X,i_e,n,nvars);

  for(i++;i<m;i++)
    {
      apply2v(Y,X,n,posnz,sparse,cfs,nbd);
      PT[i]=Y[posxn];
      if (i<=n)
	update_samples(PTs,i,Y,i_e,n,nvars);
      i++;
      apply2v(X,Y,n,posnz,sparse,cfs,nbd);
      PT[i]=X[posxn];
      if (i<=n)
	update_samples(PTs,i,X,i_e,n,nvars);
    }

  {
    int32_t drevM=2*d0-1;
    int32_t dM=2*d0;
    int32_t drevG=2*d0;

    ALLOCA(GFq_2,revG,2*d0+1);
    ALLOCA(GFq_2,revM,2*d0);
    ALLOCA(GFq_2,M,d0+1);

    for(i=0;i<drevG;i++)
      revG[i]=0;
    revG[i]=1; // 2*d0
    for(i=0;i<=drevM;i++)
      revM[i]=PT[i];
    drevM=pol_degree(revM,m-1);
    fraction(revG,&drevG,revM,&drevM,d0);
    if (drevM != d0)
      {
	for(i=0;i<nvars-1;i++)
	  free(PTs[i]);
	return 0;
      }
    rev(M,revM,d0);
    dM=pol_degree(M,drevM);
    GFq sol;
    if (!find_a_root(&sol,M,PRIME,dM,hint))
      {
	for(i=0;i<nvars-1;i++)
	  free(PTs[i]);
	return 0;
      }

    GFq *SOL=cfs[0];
    SOL[nvars-1]=sol;
    GFq Gi=EvaluateR(revG,sol,d0-1);
    if (!Gi)
      {
	for(i=0;i<nvars-1;i++)
	  free(PTs[i]);
	return 0;
      }
    Gi=inv_GFq_2(Gi);

    ALLOCA(GFq_2,revGx,d0);
    for(k=0;k<nvars-1;k++)
      {
	mul_poly_mod(revGx,PTs[k],d0-1,revM,drevM,d0);
	SOL[k]=mul_GFq(EvaluateR(revGx,sol,d0-1),Gi);
      }
    for(i=0;i<nvars-1;i++)
      free(PTs[i]);
    return nvars;
  }
}
