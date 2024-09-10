#include <stdlib.h>
#include <time.h>

#include <mm_malloc.h>

#include <stdio.h>

#include "../api.h"

#define NUM 1000

int main()
{

  clock_t start=0;
  clock_t set_time=0;
  clock_t sig_time=0;
  clock_t ver_time=0;

  int num_success=0;

  uint8_t* pk = _mm_malloc(sizeof(uint8_t) * CRYPTO_PUBLICKEYBYTES, 64);
  uint8_t* sk = _mm_malloc(sizeof(uint8_t) * CRYPTO_SECRETKEYBYTES, 64);
  uint8_t* sm = _mm_malloc(sizeof(uint8_t) * (CRYPTO_BYTES+100)   , 64);
  uint8_t m[100]="this is just a long enough char array initiated with some stuff";

  uint64_t smlen,mlen=10;

  int num;
  /*long i;*/
  for (num=0;num<NUM;num++)
    {
      /* Key generation procedure */
      start=clock();
      crypto_sign_keypair(pk,sk);
      set_time+=(clock()-start);
      /* Signature procedure */
      start = clock();
      crypto_sign(sm, &smlen, m, mlen, sk);
      sig_time+=(clock()-start);
      /*      for (i=0;i<CRYPTO_BYTES;i++)
	printf("%d ",(int)sm[i]);
	printf("\n");                            */
      /* Verify procedure */
      start=clock();
      num_success+= !(crypto_sign_open(m, &mlen, sm, smlen, pk));
      ver_time+=(clock()-start);

    }

  _mm_free(pk);
  _mm_free(sk);
  _mm_free(sm);

  printf("RESULT %5d %3d %2d %2d %3d %2d %3d %d %7d %7d %7d %7d %7ld %7ld %7ld %7ld %7.1f %7.1f %7.1f %7.1f %10.4f %10.4f %d\n",NUM,LAMBDA,N,LOG_Q,ROUND,K,C,num_success/NUM,CRYPTO_SECRETKEYBYTES, CRYPTO_PUBLICKEYBYTES, CRYPTO_BYTES,CRYPTO_PUBLICKEYBYTES+CRYPTO_BYTES,	(set_time+NUM-1)/NUM,(sig_time+NUM-1)/NUM,(ver_time+NUM-1)/NUM,(sig_time+ver_time+NUM-1)/NUM,	 1000000*set_time/(double)CLOCKS_PER_SEC/NUM,1000000*sig_time/(double)CLOCKS_PER_SEC/NUM,1000000*ver_time/(double)CLOCKS_PER_SEC/NUM, 1000000*(sig_time+ver_time)/(double)CLOCKS_PER_SEC/NUM,   (CRYPTO_PUBLICKEYBYTES+CRYPTO_BYTES)*(sig_time+ver_time)/(double)CLOCKS_PER_SEC/NUM,   (CRYPTO_BYTES)*(ver_time)/(double)CLOCKS_PER_SEC/NUM ,(int)CLOCKS_PER_SEC);

  return 0;
}

