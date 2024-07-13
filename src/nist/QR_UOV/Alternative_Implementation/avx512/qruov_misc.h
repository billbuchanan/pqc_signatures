#pragma once
#include "qruov_config.h"

#include <inttypes.h>
#include <x86intrin.h>

/*
  QRUOV_security_strength_category // 1/3/5
  QRUOV_q                          // 7/31/127
  QRUOV_v                          // v
  QRUOV_m                          // m
  QRUOV_L                          // L    : f = x^L - fc * x^fe - fc0
  QRUOV_fc                         // fc
  QRUOV_fe                         // fe
  QRUOV_fc0                        // fc0
  QRUOV_PLATFORM                   // ref/portable64/avx2/avx512
*/

#ifndef QRUOV_security_strength_category
#  error "QRUOV_security_strength_category is not defined."
#endif

#ifndef QRUOV_q
#  error "QRUOV_q is not defined."
#endif

#ifndef QRUOV_v
#  error "QRUOV_v is not defined."
#endif

#ifndef QRUOV_m
#  error "QRUOV_m is not defined."
#endif

#ifndef QRUOV_L
#  error "QRUOV_L is not defined."
#endif

#ifndef QRUOV_fc
#  error "QRUOV_fc is not defined."
#endif

#ifndef QRUOV_fe
#  error "QRUOV_fe is not defined."
#endif

#ifndef QRUOV_fc0
#  define QRUOV_fc0 1
#endif

#ifndef QRUOV_PLATFORM
#  error "QRUOV_PLATFORM is not defined."
#endif

// QRUOV_q == ((1 << QRUOV_ceil_log_2_q) - 1)

#if   (QRUOV_q == 7)
#  define QRUOV_ceil_log_2_q 3
#elif (QRUOV_q == 31)
#  define QRUOV_ceil_log_2_q 5
#elif (QRUOV_q == 127)
#  define QRUOV_ceil_log_2_q 7
#else
#  error "Unsupported: QRUOV_q == " # QRUOV_q
#endif

#if   (QRUOV_security_strength_category == 1)
#  define QRUOV_SEED_LEN 32                   // 32 Byte == 256 bit
#elif (QRUOV_security_strength_category == 3)
#  define QRUOV_SEED_LEN 48                   // 48 Byte == 384 bit
#elif (QRUOV_security_strength_category == 5)
#  define QRUOV_SEED_LEN 64                   // 64 Byte == 512 bit
#else
#  error "Unsupported: QRUOV_security_strength_category == " # QRUOV_security_strength_category
#endif

#define QRUOV_SALT_LEN      (QRUOV_SEED_LEN>>1)

#define QRUOV_n             ((QRUOV_v)+(QRUOV_m))
#define QRUOV_N             ((QRUOV_n)/(QRUOV_L))
#define QRUOV_V             ((QRUOV_v)/(QRUOV_L))
#define QRUOV_M             ((QRUOV_m)/(QRUOV_L))

#define QRUOV_perm(i)       ((i<=(QRUOV_fe-1))?((QRUOV_fe-1)-i):(QRUOV_L+(QRUOV_fe-1)-i))

#define QRUOV_STR_INDIR(x)  #x
#define QRUOV_STR(x)        QRUOV_STR_INDIR(x)

#define QRUOV_ALGNAME                                 \
  "qruov" QRUOV_STR(QRUOV_security_strength_category) \
  "q"     QRUOV_STR(QRUOV_q)                          \
  "L"     QRUOV_STR(QRUOV_L)                          \
  "v"     QRUOV_STR(QRUOV_v)                          \
  "m"     QRUOV_STR(QRUOV_m)                          \
          QRUOV_STR(QRUOV_PLATFORM)

#ifndef QRUOV_PARAM_DESCRIPTION
#  define QRUOV_PARAM_DESCRIPTION                           \
  "QR-UOV ("                                                \
    "cat=" QRUOV_STR(QRUOV_security_strength_category) ", " \
    "q="   QRUOV_STR(QRUOV_q)                          ", " \
    "L="   QRUOV_STR(QRUOV_L)                          ", " \
    "v="   QRUOV_STR(QRUOV_v)                          ", " \
    "m="   QRUOV_STR(QRUOV_m)                               \
  ")"
#endif

#define BITS2BYTES(BITS)      (((BITS)>>3)+(((BITS)&7)?1:0))

#define TYPEDEF_STRUCT(TYPE_NAME, BODY)             \
typedef struct TYPE_NAME ## _t {                    \
  BODY                                              \
} TYPE_NAME ## _s, * TYPE_NAME ## _p, TYPE_NAME [1]

#define ERROR_ABORT(MESSAGE) { \
  fprintf(stderr, "runtime error: %s in file: %s, line: %d.\n",MESSAGE,__FILE__,__LINE__); \
  abort(); \
}

#define num_bytes(n,type)                   (n * sizeof(type))
#define upper_num_elements(bytes,type)      ((bytes/sizeof(type)) + ((bytes % sizeof(type))?1:0))
#define upper_num_bytes(bytes,type)         (upper_num_elements(bytes,type)*sizeof(type))
#define aligned_number(n, src_t, dst_t)     (upper_num_elements(upper_num_bytes(num_bytes(n,src_t),dst_t),src_t))

#include "Fql.h"
#define QRUOV_aligned   __attribute__((aligned(sizeof(__m256i))))
#define aligned_m       aligned_number(QRUOV_m, Fq , __m256i)
#define aligned_M       aligned_number(QRUOV_M, Fql, __m256i)
#define aligned_V       aligned_number(QRUOV_V, Fql, __m256i)
