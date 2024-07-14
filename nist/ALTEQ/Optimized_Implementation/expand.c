#include "expand.h"


/* Randomly generate a seed */
void randomSeed(uint8_t *seed)
{
  randombytes(seed, SEED_SIZE);
}

/* Init seed expander with a seed */
static void initExpander(aes256ctr_ctx *rng, const uint8_t *seed)
{
  uint8_t key[32];
  #if SEED_SIZE <= 32
  memset(key+SEED_SIZE, 0x00, 32-SEED_SIZE);
  #else
  #error SEED_SIZE IS BEYOND 32 BYTES
  #endif
  memcpy(key, seed, SEED_SIZE);
  aes256ctr_init(rng, key, 0lu);
}

/* Use seed expander to generate a set of 16 fields elements */
static void expand16Fields(uint64_t *set, aes256ctr_ctx *rng)
{
  uint8_t carr[64];
  aes256ctr_squeezeblocks(carr, 1, rng);
  decompressArray(set, carr, 16);
}

/* Expand a seed to a set of seeds */
void expandSeeds(uint8_t *seeds, const uint8_t *seed, const int nSeeds)
{
  uint8_t carr[64];
  int i=0;
  int k=64;
  aes256ctr_ctx rng[1];

  initExpander(rng, seed);

  while (i<nSeeds*SEED_SIZE)
    {
      if (k==64)
	{
	  k=0;
	  aes256ctr_squeezeblocks(carr, 1, rng);
	}
      seeds[i++]=carr[k++];
    }
}

/* Evaluate the number of bits required to represent all elements stricly inferior to a given value */
static uint64_t sizeOfValue(uint64_t val)
{
  int val_size=1;
  while ((1lu<<val_size)<val)
    val_size++;
  return val_size;
}

/* Use seed expander and a buffer of already generated random value to generate a random value of [0,max[ */
static uint64_t randomValue(aes256ctr_ctx *rng, uint8_t *buf64, uint64_t *buf64_ind, uint64_t *buf, uint64_t *buf_size, uint64_t max, uint64_t max_size)
{
  uint64_t r;
  do{
    while ((*buf_size)<max_size)
      {
	if ((*buf64_ind)==64)
	  {
	    aes256ctr_squeezeblocks(buf64, 1, rng);
	    (*buf64_ind)=0;
	  }
	(*buf)=((*buf)<<8)+(uint64_t)buf64[(*buf64_ind)++];
	(*buf_size)+=8;
      }
    r=(*buf)&((1lu<<max_size)-1lu);
    (*buf)=((*buf)>>max_size);
    (*buf_size)-=max_size;
  } while (r>=max);
  return r;
}

/* Expand a seed to a challenge of ROUND value with K value equal to C and ROUND-K value in [0,C[ */
void expandChallenge(uint64_t *chg_c, uint64_t *chg_nc, uint64_t *chg_val, const uint8_t *seed)
{
  aes256ctr_ctx rng[1];
  uint64_t buf=0;
  uint64_t buf_size=0;
  uint8_t buf64[64];
  uint64_t buf64_ind=64;
  int k0=0;
  int k1=0;
  int r,k,i;
  uint64_t chg[ROUND];
  uint64_t C_SIZE=sizeOfValue(C);


  initExpander(rng, seed);

#if ROUND-K<K
  memset(chg, 0x00, ROUND*8);
  /* Pick randomly ROUND-K coefficients of the challenge to be equal to C */
  for(k=0;k<ROUND-K;k++)
    {
      r=randomValue(rng, buf64, &buf64_ind, &buf, &buf_size, ROUND-k,sizeOfValue(ROUND-k));
      for (i=0;i<=r;i++)
	if (chg[i]==C)
	  r++;
      chg[r]=C;
    }
  /* Fix other coefficients of the challenge */
  for (i=0;i<ROUND;i++)
    if (chg[i]==0)
      chg[i]=randomValue(rng,  buf64, &buf64_ind, &buf, &buf_size, C, C_SIZE);
#else
  for(k=0;k<ROUND;k++)
    chg[k]=C;
  /* Pick randomly K coefficients of the challenge to be striclty inferior to C */
  for(k=0;k<K;k++)
    {
      r=randomValue(rng, buf64, &buf64_ind, &buf, &buf_size, ROUND-k,sizeOfValue(ROUND-k));
      for (i=0;i<=r;i++)
	if (chg[i]<C)
	  r++;
      chg[r]=randomValue(rng,  buf64, &buf64_ind, &buf, &buf_size, C, C_SIZE);
    }
#endif
  
  for (i=0;i<ROUND;i++)
    if (chg[i]<C)
      {
	chg_nc[k0]=i;
	chg_val[k0++]=chg[i];
      }
    else
      chg_c[k1++]=i;
}

/* Expand a seed to a atf */
void expandATF(uint64_t *atf, const uint8_t *seed)
{
  uint64_t set[16]; 
  int i=0;
  int j=16;
  aes256ctr_ctx rng[1]; 

  initExpander(rng, seed); 

  while (i<LEN)
    {
      if (j==16)
	{
	  j=0;
	  expand16Fields(set, rng); 
	}
      atf[i]=set[j++];
      if (atf[i]<PRIME)
	i++;
    }
}

/* Expand a seed to n matrix columns inversible */
void expandColumns(uint64_t *col, const uint8_t *seed)
{
  uint64_t set[16];
  int i=0;
  int j=0;
  int k=16;

  aes256ctr_ctx rng[1];  
  initExpander(rng, seed); 

  while (i<N)
    {
      if (k==16)
	{
	  k=0;
	  expand16Fields(set, rng);
	}
      col[i*N+j]=set[k++];
      if (col[i*N+j]<PRIME&&(i!=j||col[i*N+j]>0)&&(++j==N))
	{
	  j=0;
	  i++;
	}
    }
}
