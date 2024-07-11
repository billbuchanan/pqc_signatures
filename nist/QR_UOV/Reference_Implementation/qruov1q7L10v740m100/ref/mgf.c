#include <string.h>
#include "qruov.h"

MGF_CTX_s * MGF_init (const uint8_t * seed, const size_t n0, MGF_CTX ctx) {
  ctx->counter    = 0 ;
  ctx->pool_bytes = 0 ;

  ctx->mdctx = EVP_MD_CTX_new() ;
  EVP_DigestInit_ex(ctx->mdctx, QRUOV_MGF_CORE(), NULL);
  EVP_DigestUpdate(ctx->mdctx, seed, n0) ;
  return ctx ;
}

MGF_CTX_s * MGF_update (const uint8_t * seed, const size_t n0, MGF_CTX ctx) {
  EVP_DigestUpdate(ctx->mdctx, seed, n0) ;
  return ctx ;
}

#ifdef QRUOV_HASH_LEGACY

#define HASH_LEN QRUOV_SEED_LEN

static inline uint32_t MGF_htonl(uint32_t x) {
  uint8_t y[4] ;
  y[0] = (x>>24) & 0xFF ;
  y[1] = (x>>16) & 0xFF ;
  y[2] = (x>> 8) & 0xFF ;
  y[3] =  x      & 0xFF ;
  return * (uint32_t *) y ;
} ;

static void MGF_hash (MGF_CTX ctx) {
  uint32_t counter = MGF_htonl(ctx->counter) ;
  EVP_MD_CTX * mdctx  = EVP_MD_CTX_new() ;
  EVP_MD_CTX_copy_ex(mdctx, ctx->mdctx) ;
  EVP_DigestUpdate(mdctx, &counter, 4) ;
  EVP_DigestFinal_ex(mdctx, ctx->pool, NULL) ;
  EVP_MD_CTX_free(mdctx) ;
  ctx->pool_bytes = HASH_LEN ;
  ctx->counter++ ;
  return ;
}

uint8_t * MGF_yield (MGF_CTX ctx, uint8_t * dest, const size_t n1) {
  for(size_t i=0; i<n1; i++){
    if(ctx->pool_bytes == 0) MGF_hash (ctx) ;
    dest[i] = ctx->pool[ HASH_LEN - ctx->pool_bytes ] ;
    ctx->pool_bytes -- ;
  }
  return dest ;
}

void MGF_final (MGF_CTX ctx) {
  EVP_MD_CTX_free(ctx->mdctx) ;
  OPENSSL_cleanse(ctx->pool, HASH_LEN) ;
  return ;
}

#else

// from openssl-1.1.1t/crypto/sha/keccak1600.c
//      openssl-3.1.0/crypto/sha/keccak1600.c

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern size_t SHA3_absorb(uint64_t A[5][5], const unsigned char *inp, size_t len, size_t r);
extern void SHA3_squeeze(uint64_t A[5][5], unsigned char *out, size_t len, size_t r);
#ifdef __cplusplus
}
#endif /* __cplusplus */

// from openssl-1.1.1t/crypto/evp/m_sha3.c
// omit 'PROV_SHA3_METHOD meth;' in openssl-3.1.0/include/internal/sha3.h 

#define KECCAK1600_WIDTH 1600

typedef struct {
    uint64_t A[5][5];
    size_t block_size;          /* cached ctx->digest->block_size */
    size_t md_size;             /* output length, variable in XOF */
    size_t num;                 /* used bytes in below buffer */
    unsigned char buf[KECCAK1600_WIDTH / 8 - 32];
    unsigned char pad;
} KECCAK1600_CTX;

#  if OPENSSL_VERSION_NUMBER < 0x30000000L
// assume openssl-1.1.1t

// from openssl-1.1.1t/crypto/evp/evp_local.h

struct evp_md_ctx_st {
    const EVP_MD *digest;
    ENGINE *engine;             /* functional reference if 'digest' is
                                 * ENGINE-provided */
    unsigned long flags;
    void *md_data;
    /* Public key context for sign/verify */
    EVP_PKEY_CTX *pctx;
    /* Update function: usually copied from EVP_MD */
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);
} /* EVP_MD_CTX */ ;

// from openssl-1.1.1t/include/crypto/evp.h

struct evp_md_st {
    int type;
    int pkey_type;
    int md_size;
    unsigned long flags;
    int (*init) (EVP_MD_CTX *ctx);
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);
    int (*final) (EVP_MD_CTX *ctx, unsigned char *md);
    int (*copy) (EVP_MD_CTX *to, const EVP_MD_CTX *from);
    int (*cleanup) (EVP_MD_CTX *ctx);
    int block_size;
    int ctx_size;               /* how big does the ctx->md_data need to be */
    /* control function */
    int (*md_ctrl) (EVP_MD_CTX *ctx, int cmd, int p1, void *p2);
} /* EVP_MD */ ;

#  else
// assume openssl-3.1.0

// from openssl-3.1.0/crypto/evp/evp_local.h

struct evp_md_ctx_st {
    const EVP_MD *reqdigest;    /* The original requested digest */
    const EVP_MD *digest;
    ENGINE *engine;             /* functional reference if 'digest' is
                                 * ENGINE-provided */
    unsigned long flags;
    void *md_data;
    /* Public key context for sign/verify */
    EVP_PKEY_CTX *pctx;
    /* Update function: usually copied from EVP_MD */
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);

    /*
     * Opaque ctx returned from a providers digest algorithm implementation
     * OSSL_FUNC_digest_newctx()
     */
    void *algctx;
    EVP_MD *fetched_digest;
} /* EVP_MD_CTX */ ;

// from openssl-3.1.0/crypto/evp/evp_local.h

#include "refcount.h"
#include <openssl/core_names.h>

struct evp_md_st {
    /* nid */
    int type;

    /* Legacy structure members */
    int pkey_type;
    int md_size;
    unsigned long flags;
    int origin;
    int (*init) (EVP_MD_CTX *ctx);
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);
    int (*final) (EVP_MD_CTX *ctx, unsigned char *md);
    int (*copy) (EVP_MD_CTX *to, const EVP_MD_CTX *from);
    int (*cleanup) (EVP_MD_CTX *ctx);
    int block_size;
    int ctx_size;               /* how big does the ctx->md_data need to be */
    /* control function */
    int (*md_ctrl) (EVP_MD_CTX *ctx, int cmd, int p1, void *p2);

    /* New structure members */
    /* Above comment to be removed when legacy has gone */
    int name_id;
    char *type_name;
    const char *description;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;
    OSSL_FUNC_digest_newctx_fn *newctx;
    OSSL_FUNC_digest_init_fn *dinit;
    OSSL_FUNC_digest_update_fn *dupdate;
    OSSL_FUNC_digest_final_fn *dfinal;
    OSSL_FUNC_digest_digest_fn *digest;
    OSSL_FUNC_digest_freectx_fn *freectx;
    OSSL_FUNC_digest_dupctx_fn *dupctx;
    OSSL_FUNC_digest_get_params_fn *get_params;
    OSSL_FUNC_digest_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_digest_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_digest_gettable_params_fn *gettable_params;
    OSSL_FUNC_digest_settable_ctx_params_fn *settable_ctx_params;
    OSSL_FUNC_digest_gettable_ctx_params_fn *gettable_ctx_params;

} /* EVP_MD */ ;

#  endif

static int EVP_DigestFinalXOF_BEGIN_legacy(EVP_MD_CTX * ctx, unsigned char * md){
    int ret = 0;
    if (ctx->digest->flags & EVP_MD_FLAG_XOF
        && ctx->digest->md_ctrl(ctx, EVP_MD_CTRL_XOF_LEN, 1, NULL)) {
        ret = ctx->digest->final(ctx, md) ;
        KECCAK1600_CTX * kctx = (KECCAK1600_CTX *) ctx->md_data;
        size_t bsz = kctx->block_size;
        ctx->digest->md_ctrl(ctx, EVP_MD_CTRL_XOF_LEN, (int)bsz+1, NULL) ;
    } else {
        EVPerr(EVP_F_EVP_DIGESTFINALXOF, EVP_R_NOT_XOF_OR_INVALID_LENGTH);
    }
    return ret ;
}

#  if OPENSSL_VERSION_NUMBER < 0x30000000L

int EVP_DigestFinalXOF_BEGIN(EVP_MD_CTX * ctx, unsigned char * md){
    return EVP_DigestFinalXOF_BEGIN_legacy(ctx, md) ;
}

#define ctx2kctx(ctx) ((KECCAK1600_CTX *)ctx->md_data)

#  else

int EVP_DigestFinalXOF_BEGIN(EVP_MD_CTX * ctx, unsigned char * md){
    int ret = 0;

    OSSL_PARAM params[2];
    size_t i = 0;

    if (ctx->digest == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INVALID_NULL_ALGORITHM);
        return 0;
    }

    if (ctx->digest->prov == NULL)
        return EVP_DigestFinalXOF_BEGIN_legacy(ctx, md) ;

    if (ctx->digest->dfinal == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_FINAL_ERROR);
        return 0;
    }

    size_t size = 1 ;

    params[i++] = OSSL_PARAM_construct_size_t(OSSL_DIGEST_PARAM_XOFLEN, &size);
    params[i++] = OSSL_PARAM_construct_end();

    if (EVP_MD_CTX_set_params(ctx, params) > 0)
        ret = ctx->digest->dfinal(ctx->algctx, md, &size, size) ;

    KECCAK1600_CTX * kctx = (KECCAK1600_CTX *) ctx->algctx ;

    size_t bsz = kctx->block_size ;

    i = 0;
    size = bsz+1 ;
    params[i++] = OSSL_PARAM_construct_size_t(OSSL_DIGEST_PARAM_XOFLEN, &size);
    params[i++] = OSSL_PARAM_construct_end();
    EVP_MD_CTX_set_params(ctx, params) ;

    return ret;
}

#define ctx2kctx(ctx) \
    ((ctx->digest->prov == NULL)?((KECCAK1600_CTX *)ctx->md_data):((KECCAK1600_CTX *)ctx->algctx))

#  endif

int EVP_DigestFinalXOF_SQUEEZE(EVP_MD_CTX * ctx, unsigned char * md) {
    int ret = 0;
    if (ctx->digest->flags & EVP_MD_FLAG_XOF){
      KECCAK1600_CTX * kctx = ctx2kctx(ctx) ;
      size_t bsz = kctx->block_size;
      SHA3_squeeze(kctx->A, md, bsz+1, bsz) ;
      ret = 1 ;
    }else{
      EVPerr(EVP_F_EVP_DIGESTFINALXOF, EVP_R_NOT_XOF_OR_INVALID_LENGTH);
    }
    return ret ;
}

int EVP_DigestFinalXOF_END(EVP_MD_CTX *ctx) {
    int ret = 0;
    if (ctx->digest->flags & EVP_MD_FLAG_XOF){
      if (ctx->digest->cleanup != NULL) {
          ctx->digest->cleanup(ctx);
          EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
      }
      OPENSSL_cleanse(ctx->md_data, ctx->digest->ctx_size);
      ret = 1;
    }else{
      EVPerr(EVP_F_EVP_DIGESTFINALXOF, EVP_R_NOT_XOF_OR_INVALID_LENGTH);
    }
    return ret ;
}

uint8_t * MGF_yield (MGF_CTX ctx, uint8_t * dest, const size_t n1) {
  if(ctx->counter == 0){
    ctx->counter = 1 ;
    EVP_DigestFinalXOF_BEGIN(ctx->mdctx, ctx->pool) ;
  }
  for(size_t i=0; i<n1; i++){
    if(ctx->pool_bytes == 0){
      EVP_DigestFinalXOF_SQUEEZE(ctx->mdctx, ctx->pool) ;
      ctx->pool_bytes = QRUOV_SHAKE_BSZ ;
    }
    dest[i] = ctx->pool[ QRUOV_SHAKE_BSZ - ctx->pool_bytes ] ;
    ctx->pool_bytes -- ;
  }
  return dest ;
}

void MGF_final (MGF_CTX ctx) {
  EVP_DigestFinalXOF_END(ctx->mdctx) ;
  EVP_MD_CTX_free(ctx->mdctx);
  OPENSSL_cleanse(ctx->pool, QRUOV_SHAKE_BSZ+1);
  return ;
}

#endif

void MGF_CTX_copy (MGF_CTX src, MGF_CTX dst) {
  memcpy(dst, src, sizeof(MGF_CTX)) ;
  dst->mdctx = EVP_MD_CTX_new() ;
  EVP_MD_CTX_copy_ex(dst->mdctx, src->mdctx) ;
  return ;
}
