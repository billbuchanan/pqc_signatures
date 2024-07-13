#ifndef CONFIG_H_
#define CONFIG_H_

#define NDEBUG

/*
 *   This file contains all implementation options that do not affect testvectors
 *   params.h contains other options.
 */

//
// The following macros choose optimizations for basic linear algebra functions.
// It is currently defined from the makefile
//
//#define _BLAS_UINT64_
//#define _BLAS_SSE_
//#define _BLAS_AVX2_
//#define _BLAS_NEON_
//#define _BLAS_M4F_

// GF multiplication with multiplication tables
//#define _MUL_WITH_MULTAB_


//
// choose implementations for SHAKE256, AES128CTR, and randombytes() functions
//
#if !(defined(_UTILS_OPENSSL_)||defined(_UTILS_SUPERCOP_)||defined(_UTILS_PQM4_))
// default:
#define _UTILS_OPENSSL_

//#define _UTILS_SUPERCOP_
//#define _UTILS_PQM4_
#endif  // !(defined(_UTILS_OPENSSL_)||defined(_UTILS_SUPERCOP_)||defined(_UTILS_PQM4_))

//
// Options for AES128CTR
// The following macros will implement AES128CTR with native aes instructions.
//
// x86 aes instrucitons
//#define _UTILS_AESNI_
// arm aes instructions
//#define _UTILS_NEONAES_
// arm neon bitslice aes
//#define _UTILS_NEONBSAES_

#if ! (defined(_UTILS_PQM4_)||defined(_UTILS_AESNI_)||defined(_UTILS_NEONAES_)||defined(_UTILS_NEONBSAES_))

#if defined(_BLAS_AVX2_)
#define _UTILS_AESNI_
#elif defined(_BLAS_NEON_)&&defined(_APPLE_SILICON_)
#define _UTILS_NEONAES_
#elif defined(_BLAS_NEON_)
#define _UTILS_NEONBSAES_
#endif

#endif


//
// Options for randombytes()
//
//
// The following macro will implement randombytes() with utils/nistkat/rng.[hc]
// It uses AEC256CTR and AES256_ECB from openssl.
//
//#define _NIST_KAT_
//
// The following macro will implement randombytes() with C rand()
// Turn on only when there is no proper implementation
//
//#define _DEBUG_RANDOMBYTES_


#endif
