#include <stdint.h>
#include "cpucycles.h"
#include "speed_print.h"

#include "../api.h"
#include "../expand.h" /* for the hash */
#include <mm_malloc.h>
#include <stdint.h> /* for predetermined number of bits per type */
#define NTESTS 1000

uint64_t t[NTESTS];

/* SMLEN doesn't matter much */
#define SMLEN 512
#define MLEN LAMBDA/4

int main(void)
{
  unsigned int i;
  uint8_t* pk = _mm_malloc(sizeof(uint8_t) * CRYPTO_PUBLICKEYBYTES, 64);
  uint8_t* sk = _mm_malloc(sizeof(uint8_t) * CRYPTO_SECRETKEYBYTES, 64);
  uint8_t* sm = _mm_malloc(sizeof(uint8_t) * (CRYPTO_BYTES+MLEN)  , 64);
  /* same message for all signing processes */
  /* uint8_t m[100]="this is just a long enough char array initiated with some stuff"; */
  /* use a different message per signing process, of size MLEN = LAMBDA*2 in bits */
  uint8_t* m = _mm_malloc(sizeof(uint8_t) * (NTESTS*MLEN), 64);
  uint64_t smlen = SMLEN, mlen=MLEN;

  /* initialize all messages to be signed to be somehow distinct, by hashing repeatedly */
  uint8_t *m_ptr = m;
  for (i = 0; i < NTESTS; i++){
    hashArray(m+(LAMBDA/4), m, LAMBDA/4);
    *m_ptr += LAMBDA/4;
  }
  m_ptr = m;


  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
    crypto_sign_keypair(pk,sk);
  }
  print_results("Keypair:", t, NTESTS);

  for(i = 0; i < NTESTS; ++i) {
    *m += LAMBDA/4; /* this simple operation should not impact clock cycles too much, comment if message is kept the same */
    t[i] = cpucycles();
    crypto_sign(sm, &smlen, m, mlen, sk);
  }
  print_results("Sign:", t, NTESTS);

  for(i = 0; i < NTESTS; ++i) {
    t[i] = cpucycles();
	crypto_sign_open(m, &mlen, sm, smlen, pk);
  }
  print_results("Verify:", t, NTESTS);

  /* free */
  _mm_free(pk); _mm_free(sk); _mm_free(sm);
  _mm_free(m); /* comment this line if you revert back to signing the same static message */

  return 0;
}
