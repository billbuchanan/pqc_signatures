#pragma once

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "qruov_misc.h"

/* =====================================================================
   mgf
   ===================================================================== */

#ifdef QRUOV_HASH_LEGACY

#  if QRUOV_SEED_LEN == 32
#    define QRUOV_MGF_CORE EVP_sha256
/*
#    define HASH_CTX    SHA256_CTX
#    define HASH_Init   SHA256_Init
#    define HASH_Update SHA256_Update
#    define HASH_Final  SHA256_Final
*/
#  elif QRUOV_SEED_LEN == 48
#    define QRUOV_MGF_CORE EVP_sha384
/*
#    define HASH_CTX    SHA512_CTX
#    define HASH_Init   SHA384_Init
#    define HASH_Update SHA384_Update
#    define HASH_Final  SHA384_Final
*/
#  elif QRUOV_SEED_LEN == 64
#    define QRUOV_MGF_CORE EVP_sha512
/*
#    define HASH_CTX    SHA512_CTX
#    define HASH_Init   SHA512_Init
#    define HASH_Update SHA512_Update
#    define HASH_Final  SHA512_Final
*/
#  else
#    error "Unsupported: QRUOV_SEED_LEN == " # QRUOV_SEED_LEN
#  endif

/*
TYPEDEF_STRUCT (MGF_CTX,
  uint32_t counter ;
  HASH_CTX hash_ctx [1] ;
  uint32_t pool_bytes ;
  uint8_t  pool [QRUOV_SEED_LEN] ;
) ;
*/

#define QRUOV_MGF_POOLSIZE (QRUOV_SEED_LEN)

#else

/*
  SHAKE128: bsz = 168
  SHAKE256: bsz = 136
  unsigned char md [bsz+1]
*/

#  if   QRUOV_SEED_LEN == 32
#    define QRUOV_MGF_CORE  EVP_shake128
#    define QRUOV_SHAKE_BSZ 168
#  elif ( QRUOV_SEED_LEN == 48 ) || (QRUOV_SEED_LEN == 64)
#    define QRUOV_MGF_CORE  EVP_shake256
#    define QRUOV_SHAKE_BSZ 136
#  else
#    error "Unsupported: QRUOV_SEED_LEN == " # QRUOV_SEED_LEN
#  endif

#define QRUOV_MGF_POOLSIZE (QRUOV_SHAKE_BSZ + 1)

#endif

TYPEDEF_STRUCT (MGF_CTX,
  uint32_t counter    ;
  EVP_MD_CTX * mdctx  ;
  uint32_t pool_bytes ;
  uint8_t  pool [ QRUOV_MGF_POOLSIZE ] ;
) ;

extern MGF_CTX_s * MGF_init  (const uint8_t * seed, const size_t n0, MGF_CTX ctx) ;
extern MGF_CTX_s * MGF_update(const uint8_t * seed, const size_t n0, MGF_CTX ctx) ;
extern uint8_t   * MGF_yield (MGF_CTX ctx, uint8_t * dest, const size_t n1) ;
extern void MGF_final(MGF_CTX ctx) ;
extern void MGF_CTX_copy(MGF_CTX src, MGF_CTX dst) ;
