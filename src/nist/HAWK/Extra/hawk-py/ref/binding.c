#include "hawk_inner.h"
#include "hawk.h"
#include <stdio.h>
#include <stdlib.h>


void c_keygen(unsigned int logn,
    uint8_t *priv, uint8_t *pub, uint8_t *buf, unsigned int lbuf){

  // init the randomness source
	shake_context rng;
	shake_init(&rng, 256);
  shake_inject(&rng, buf,lbuf);
	shake_flip(&rng);

  // at leat 40 * 2^logn with some maring :)
	uint8_t tmp[(40*1024)+7];

  hawk_keygen(logn,priv,pub, (hawk_rng)&shake_extract, &rng,tmp,sizeof(tmp));
}

void c_sign(unsigned int logn,
    uint8_t *priv, uint8_t *msg, unsigned int lmsg, uint8_t *sig,uint8_t *buf, unsigned int lbuf){

  // init the randomness source
	shake_context rng;
	shake_init(&rng, 256);
  shake_inject(&rng, buf,lbuf);
	shake_flip(&rng);

	shake_context sc_data;
  hawk_sign_start(&sc_data);
  shake_inject(&sc_data,msg,lmsg);

  // at leat 40 * 2^logn with some maring :)
	uint8_t tmp[(6*1024)+7];

  hawk_sign_finish(logn, (hawk_rng)&shake_extract, &rng, sig, &sc_data, priv, tmp,sizeof(tmp));
}

int c_verify(unsigned int logn,
    uint8_t *pub, uint8_t *msg, unsigned int lmsg, uint8_t *sig){

	shake_context sc_data;
  hawk_sign_start(&sc_data);
  shake_inject(&sc_data,msg,lmsg);

  // at leat 40 * 2^logn with some maring :)
	uint8_t tmp[(10*1024)+7];
  return hawk_verify_finish(logn, sig, HAWK_SIG_SIZE(logn), &sc_data, pub, HAWK_PUBKEY_SIZE(logn), tmp,sizeof(tmp));
}

void c_keygen_unpacked(
	int8_t *restrict f, int8_t *restrict g,
	int8_t *restrict F, int8_t *restrict G,
	int16_t *restrict q00, int16_t *restrict q01,
  int32_t *restrict q11,
  int8_t *restrict seed,
	unsigned int logn, uint8_t *buf, unsigned int lbuf){

  // init the randomness source
	shake_context rng;
	shake_init(&rng, 256);
  shake_inject(&rng, buf,lbuf);
	shake_flip(&rng);


  // at leat 40 * 2^logn with some maring :)
	uint64_t tmp[(40*1024)+7];
  Hawk_keygen(logn,f,g,F,G,q00,q01,q11,seed,(hawk_rng)&shake_extract, &rng,tmp,sizeof(tmp));

	return;
}

int c_encode_public(unsigned int logn,
    uint8_t *buf2, 
    int16_t *q00,int16_t *q01){
  return encode_public(logn,buf2,HAWK_PUBKEY_SIZE(logn),q00,q01);
}

int c_encode_sig(unsigned int logn,
    uint8_t *sig,
    uint8_t *salt, unsigned int salt_len, int16_t *s1){
  return encode_sig(logn,sig,HAWK_SIG_SIZE(logn),salt,salt_len,s1);
}


void c_decode_public(unsigned int logn, 
      int16_t *q00, 
      int16_t *q01, 
      int8_t *buf2){

  int n = 1<<logn;
  int16_t q00q012[HAWK_DECODED_PUB_LENGTH(10)];
  
  hawk_decode_public_key(logn,q00q012,buf2,HAWK_PUBKEY_SIZE(logn));
  memcpy(q00,q00q012,n);
  memcpy(q01,q00q012 + n/2, 2*n);

}

void c_encode_private(unsigned int logn, 
    uint8_t *priv, uint8_t *seed, int8_t *F, int8_t *G, uint8_t *pub
  ){
  encode_private(logn,priv,seed,F,G,pub,HAWK_PUBKEY_SIZE(logn));
}
