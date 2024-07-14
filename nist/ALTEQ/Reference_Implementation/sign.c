#include <stdlib.h>
#include <string.h>

#include <mm_malloc.h>
#include <stdint.h>

#include "api.h"
#include "compress.h"
#include "expand.h"
#include "matrix.h"
#include "atf.h"

int crypto_sign_keypair(uint8_t  *pk, uint8_t  *sk)
{
  uint8_t * seeds = (uint8_t *)_mm_malloc(sizeof(uint8_t ) * SEED_SIZE*(C+1), 64);
  uint64_t* atfC  = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN            , 64);
  uint64_t* atf   = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN            , 64);
  uint64_t* cols  = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N            , 64);

  int i;

  memset(pk, 0x00, CRYPTO_PUBLICKEYBYTES);
  memset(sk, 0x00, CRYPTO_SECRETKEYBYTES);

  randomSeed(sk);
  
  /* Expanding Secret Key */
  expandSeeds(seeds, sk, C+1);

  /* Expanding ATF_C */
  expandATF(atfC, &(seeds[C*SEED_SIZE]));

  /* Keeping Seed for ATF_C in both key */
  memcpy(&(pk[C*ALT_SIZE]),&seeds[C*SEED_SIZE], SEED_SIZE);

  /* Generating C ATF from C secret N Columns Matrices such that acting on ATF_i with columns matrices i return atf_C */
  for(i=0; i<C; i++){
    expandColumns(cols, &(seeds[i*SEED_SIZE]));
    invertingOnATF(atf, atfC, cols);
    compressArray(&(pk[ALT_SIZE*i]), atf, LEN);
  }

  /* free */
  _mm_free(seeds);
  _mm_free(atfC);
  _mm_free(atf);
  _mm_free(cols);

  return 0;
}


int crypto_sign(uint8_t  *sm, uint64_t *smlen, const uint8_t  *m, uint64_t mlen, const uint8_t  *sk)
{

  uint8_t * hash     = (uint8_t *)_mm_malloc(sizeof(uint8_t ) * (ALT_SIZE*ROUND+2*SEED_SIZE), 64);
  uint8_t * seeds_sk = (uint8_t *)_mm_malloc(sizeof(uint8_t ) * SEED_SIZE*(C+1)             , 64);
  uint64_t* atfC     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN                         , 64);
  uint8_t * seeds    = (uint8_t *)_mm_malloc(sizeof(uint8_t ) * SEED_SIZE*ROUND             , 64);
  uint64_t* cols_rnd = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*ROUND                   , 64);
  uint64_t* atfs     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN*ROUND                   , 64);
  uint64_t* cols     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*C                       , 64);
  uint64_t* mat      = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N                         , 64);

  uint64_t cols_exp[C];
  uint64_t chg_c[ROUND-K];
  uint64_t chg_nc[K];
  uint64_t chg_val[K];

  int r, j, sm_index;

  if (!mlen){
    /* free and exit */
    _mm_free(hash);
    _mm_free(seeds_sk);
    _mm_free(atfC);
    _mm_free(seeds);
    _mm_free(cols_rnd);
    _mm_free(atfs);
    _mm_free(cols);
    _mm_free(mat);
    return -1;
  }

  sm_index=2*SEED_SIZE;
  memset(cols_exp, 0x00, C*8);
  memset(hash, 0x00, ALT_SIZE*ROUND+2*SEED_SIZE);
  memset(sm, 0x00, CRYPTO_BYTES);

  /* Expanding Secret Key  */
  expandSeeds(seeds_sk, sk, C+1);
  expandATF(atfC, &(seeds_sk[C*SEED_SIZE]));

  /* Creating ROUND random N Columns matrices */
  randomSeed(seeds);
  expandSeeds(seeds, seeds, ROUND);
  for(r=0; r<ROUND; r++)
    expandColumns(&(cols_rnd[r*N*N]), &(seeds[r*SEED_SIZE]));

  /* Acting independently on ATFC ROUND time */
  actingOnATFS(atfs, atfC, cols_rnd, ROUND);

  /* Creating Challenge from hash */
  hashArray(hash, m, mlen);
  for (r=0;r<ROUND;r++)
    compressArray(&(hash[2*SEED_SIZE+r*ALT_SIZE]),&(atfs[r*LEN]),LEN);
  hashArray(sm, hash, ALT_SIZE*ROUND+2*SEED_SIZE);
  expandChallenge(chg_c, chg_nc, chg_val, sm);

  /* Keeping Seeds for column matrices corresponding to challenge =C */
  for(r=0; r<ROUND-K; r++)
    for (j=0;j<SEED_SIZE;j++)
      sm[sm_index++]=seeds[chg_c[r]*SEED_SIZE+j];

  /* Expanding only necessary KxN Columns matrices from the CxN from Secret Key*/
  for(r=0; r<K; r++)
    if (cols_exp[chg_val[r]]++==0)
      expandColumns(&(cols[N*N*chg_val[r]]), &(seeds_sk[chg_val[r]*SEED_SIZE]));

  /* Construct K Matrices corresponding to challenge !=C */
  for(r=0; r<K; r++)
    {
      columnsMatrix(mat,  &(cols_rnd[N*N*chg_nc[r]]), &(cols[N*N*chg_val[r]]));
      compressArray(&(sm[sm_index]),mat,N*N);
      sm_index+=MAT_SIZE;
    }

  *smlen=CRYPTO_BYTES;
  *smlen+=mlen;
  memcpy(sm+CRYPTO_BYTES, m, mlen);

  /* free */
  _mm_free(hash);
  _mm_free(seeds_sk);
  _mm_free(atfC);
  _mm_free(seeds);
  _mm_free(cols_rnd);
  _mm_free(atfs);
  _mm_free(cols);
  _mm_free(mat);

  return 0;
}


int crypto_sign_open(uint8_t  *m, uint64_t *mlen, const uint8_t  *sm, uint64_t smlen, const uint8_t  *pk)
{
  int sm_index=2*SEED_SIZE;
  int correct=0;

  uint64_t* atfC     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN                         , 64);
  uint8_t * hash     = (uint8_t *)_mm_malloc(sizeof(uint8_t ) * (ALT_SIZE*ROUND+2*SEED_SIZE), 64);
  uint64_t* cols     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*(ROUND-K)               , 64);
  uint64_t* mats     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*K                       , 64);
  uint64_t* atfs     = (uint64_t*)_mm_malloc(sizeof(uint64_t) * LEN*ROUND                   , 64);

  uint8_t  chk[2*SEED_SIZE];
  uint64_t chg_c[ROUND-K];
  uint64_t chg_nc[K];
  uint64_t chg_val[K];

  int r, i;

  memset(chk, 0x00, 2*SEED_SIZE);
  memset(hash, 0x00, ALT_SIZE*ROUND+2*SEED_SIZE);

  *mlen=smlen-CRYPTO_BYTES;
  memcpy(m, sm+CRYPTO_BYTES, *mlen);

  /* Expanding ATF_C  */
  expandATF(atfC, &(pk[C*ALT_SIZE]));
  
  /* Expanding Challenge  */
  expandChallenge(chg_c, chg_nc, chg_val, sm);

  /* Expanding ROUND-K  N Columns Matrices correponding to challenge =C  */
  for(r=0; r<ROUND-K;r++)
    {
      expandColumns(&(cols[r*N*N]), &(sm[sm_index]));
      sm_index+=SEED_SIZE;
    }

  /* Acting independently on ATFC ROUND-K time */
  actingOnATFS(atfs, atfC, cols, ROUND-K);

  /* Preparing Hashing of ROUND-K ATF */
  hashArray(hash, m, *mlen);
  for(r=0; r<ROUND-K;r++)
    compressArray(&(hash[2*SEED_SIZE+chg_c[r]*ALT_SIZE]),&(atfs[r*LEN]),LEN);

  /* Extract matrix and corresponding ATF for challenge <C*/
  for(r=0; r<K; r++)
    {
      decompressArray(&(mats[r*N*N]), &(sm[sm_index]), N*N);
      decompressArray(&(atfs[r*LEN]), &(pk[ALT_SIZE*chg_val[r]]), LEN);
      sm_index+=MAT_SIZE;	    
    }

  /* Acting independently on K ATFs with K independent matrices using Tensor */
  actingOnATFSwTensor(atfs, atfs, mats);

  /* Hashing with the K last ATF */
  for(r=0; r<K; r++)
    compressArray(&(hash[2*SEED_SIZE+chg_nc[r]*ALT_SIZE]),&(atfs[r*LEN]),LEN);
  hashArray(chk, hash, ALT_SIZE*ROUND+2*SEED_SIZE);

  /* Verfication */
  for (i=0;i<2*SEED_SIZE;i++)
    correct |= (sm[i]!=chk[i]);

  /* free */
  _mm_free(atfC);
  _mm_free(hash);
  _mm_free(cols);
  _mm_free(mats);
  _mm_free(atfs);

  return correct;
}
