#include "field.h"

/* Modular multiplication 32 by 32 to 33bits */
uint64_t multiplicationModuloP(const uint64_t a, const uint64_t b)
{
  uint64_t r=a*b;
  r=r-PRIME*(r>>LOG_Q);
  return r;
}

/* Modular reduction 61 to 32bits */
uint64_t reductionModuloP(const uint64_t a)
{
  uint64_t r=a;
  r=r-PRIME*(r>>LOG_Q);
  r=r-PRIME*(r>>LOG_Q);
  return r;
}

/* Modular reduction 32 to [0,P[ */
uint64_t reductionStrict(const uint64_t a)
{
  uint64_t r=a;
  if (r>=PRIME)
    return (r-PRIME);
  return r;
}

/* Modular inversion of a field element */
static uint64_t inversionModuloP(const uint64_t a)
{
  uint64_t b=a;
  int i;
  for (i=LOG_Q-2;i>=0;i--)
    {
      b=reductionModuloP(multiplicationModuloP(b,b));
      if ((PRIME-2)&1lu<<i)
	b=reductionModuloP(multiplicationModuloP(b,a));
    }
  return b;
}

/* Modular inversion of a set of fields element */
void setInversionModuloP(uint64_t *set)
{
  uint64_t mul[N];
  uint64_t inv0;
  int i;

  mul[0]=set[0];
  for (i=1;i<N;i++)
    mul[i]=reductionModuloP(multiplicationModuloP(mul[i-1], set[i]));
  inv0=inversionModuloP(mul[N-1]);
  for (i=N-1;i>0;i--)
    {
      mul[i]=reductionModuloP(multiplicationModuloP(mul[i-1], inv0));
      inv0=reductionModuloP(multiplicationModuloP(inv0, set[i]));
      set[i]=mul[i];
    }
  set[0]=inv0;
}
