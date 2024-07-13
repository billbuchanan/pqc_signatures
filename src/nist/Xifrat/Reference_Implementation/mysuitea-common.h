/* DannyNiu/NJF, 2018-02-08. Public Domain. */

#ifndef MySuiteA_mysuitea_common_h
#define MySuiteA_mysuitea_common_h 1

#define static_assert _Static_assert

#include <limits.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define xglue(a,b) a##b
#define glue(a,b) xglue(a,b)

// Each primitive instance shall define
// 1. a function compatible with the type (IntPtr(*)(int)),
// 2. a function-like macro that evaluates to some integer types that's
//    large enough to hold pointers,
// that when evaluated in run-time or compile time, yields
// relevant information associated with particular primitive.
//
// Both the function and the function like macro takes a single
// argument `q' that is one of the constants enumerated below.
//
// The name of the function shall be the name of the primitive
// prefixed with a single "i"; the name of the macro shall be
// the name of the primitive prefixed with a single "c".

// <s>2020-11-21</s>:
// Per https://stackoverflow.com/q/64894785
// ``uintptr_t'' is changed to ``uintmax_t'', and the
// following static assertion is added.
///
// 2020-12-24:
// ``uintmax_t'' is, on my second thought, an overkill.
//
// What I really need, is a type to represent the byte addresses and ranges
// of the working memory space. Under mainstream memory models such as
// ILP32 and LP64, ``size_t'', ``uintptr_t'' would both work for me; and
// even with unconventional memory models, where function and objects
// doesn't share address space, it's exceptionally rare for code to exceed
// sizes as big as 2^32 bytes. (It's so much so that x86-64 and its ABI
// don't extend the "immediate" operand for CALL instruction to 64-bits for
// relatively positioned codes.)
///
// 2021-03-09:
// The name of the type is changed to the more "semantic" ``IntPtr'',
// allowing its name to be more meaningful and can be re-defined when
// need arises. It is intentional that the type is signed.

// Users of this library may change this type definition
// should their memory model require special treatment.
typedef intptr_t IntPtr;

static_assert(
    sizeof(IntPtr) >= sizeof(size_t) &&
    sizeof(IntPtr) >= sizeof(void (*)(void)),
    "Expectation on the compilation environment didn't hold!");

typedef struct CryptoParam CryptoParam_t;

typedef IntPtr (*iCryptoObj_t)(int q);
typedef IntPtr (*tCryptoObj_t)(const CryptoParam_t *P, int q);

struct CryptoParam {
    union {
        iCryptoObj_t info; // if param/aux is NULL,
        tCryptoObj_t template; // otherwise.
    };
    union {
        const CryptoParam_t *param;
        const void *aux;
    };
};

enum {
    // Applicable to
    // 1.) Primitives whose output length are fixed and constant. 
    //
    // - For hash functions, this is the length of the digest in bytes.
    //
    outBytes,

    // Applicable to
    // 1.) Fixed-length keyed or unkeyed permutations.
    // 2.) Iterated bufferred processing primitives. 
    //
    blockBytes,

    // Applicable to
    // 1.) All keyed primitives.
    //
    // - If positive, the primitive accepts only keys of fixed length;
    // - if negative, the primitive accepts keys of length up to
    //   the absolute value of this parameter;
    // - values with absolute values smaller than or equal to 4 have
    //   special meanings;
    // - a value of -1 specifies that the key may be of unlimited length;
    keyBytes,

    // Applicable to
    // 1.) All iterated keyed permutation with at least 1 iteration.
    //
    keyschedBytes,

    // Applicable to
    // 1.) Primitives reusing working variables for invocations.
    // 2.) Primitives saving working varibles for later resumption. 
    //
    contextBytes,

    // Applicable to
    // 1.) AEAD.
    //
    ivBytes, tagBytes, 

    // Block Cipher Interfaces //
    EncFunc, DecFunc, KschdFunc,

    // Permutation Interfaces //
    PermuteFunc,

    // Keyed Context Initialization Function (AEAD, HMAC, etc.) //
    KInitFunc,
    
    // Hash & XOF Functions //
    InitFunc,
    UpdateFunc, WriteFunc=UpdateFunc,
    FinalFunc, XofFinalFunc,
    ReadFunc,

    // AEAD Functions //
    AEncFunc, ADecFunc, 
    
    // Information macros evaluates to 0
    // for queries not applicable to them. 
};

// Aliases additions for PRNG/DRBG.
enum {
    seedBytes     = keyBytes,
    InstInitFunc  = KInitFunc,
    ReseedFunc    = WriteFunc,
    GenFunc       = ReadFunc,
};

typedef void (*EncFunc_t)(void const *in, void *out, void const *restrict w);
typedef void (*DecFunc_t)(void const *in, void *out, void const *restrict w);
typedef void (*KschdFunc_t)(void const *restrict key, void *restrict w);

typedef void (*PermuteFunc_t)(void const *in, void *out);

// Returns ``x'' on success, or ``NULL'' with invalid ``klen''.
typedef void *(*KInitFunc_t)(void *restrict x,
                             void const *restrict k,
                             size_t klen);
typedef void (*InitFunc_t)(void *restrict x);

typedef void (*UpdateFunc_t)(void *restrict x,
                             void const *restrict data, 
                             size_t len);
typedef UpdateFunc_t WriteFunc_t;

typedef void (*FinalFunc_t)(void *restrict x, void *restrict out, size_t t);
typedef void (*XFinalFunc_t)(void *restrict x);
typedef void (*ReadFunc_t)(void *restrict x,
                           void *restrict data,
                           size_t len);

// AEAD cipher taking data all-at-once.
typedef void (*AEncFunc_t)(void *restrict x,
                           void const *restrict iv,
                           size_t alen, void const *aad,
                           size_t len, void const *in, void *out,
                           size_t tlen, void *T);
// returns ``out'' on success and ``NULL'' on decryption failure.
typedef void *(*ADecFunc_t)(void *restrict x,
                            void const *restrict iv,
                            size_t alen, void const *aad,
                            size_t len, void const *in, void *out,
                            size_t tlen, void const *T);

// Alias additions for PRNG/DRBG.
typedef KInitFunc_t     InstInitFunc_t;
typedef WriteFunc_t     ReseedFunc_t;
typedef ReadFunc_t      GenFunc_t;

// Because `obj' can be an identifier naming a macro
// as well as a pointer to a function , we have to
// make sure that `obj' is not parenthesized so that
// macro expansion won't be suppressed.

#define OUT_BYTES(obj)      ((IntPtr)(obj(outBytes)))
#define BLOCK_BYTES(obj)    ((IntPtr)(obj(blockBytes)))
#define KEY_BYTES(obj)      ((IntPtr)(obj(keyBytes)))
#define KSCHD_BYTES(obj)    ((IntPtr)(obj(keyschedBytes)))
#define CTX_BYTES(obj)      ((IntPtr)(obj(contextBytes)))
#define IV_BYTES(obj)       ((IntPtr)(obj(ivBytes)))
#define TAG_BYTES(obj)      ((IntPtr)(obj(tagBytes)))

// In case C doesn't expand nested macro.
#define CTX_BYTES_1(obj)    ((IntPtr)(obj(contextBytes)))
#define CTX_BYTES_2(obj)    ((IntPtr)(obj(contextBytes)))
#define CTX_BYTES_3(obj)    ((IntPtr)(obj(contextBytes)))

#define ENC_FUNC(obj)       ((EncFunc_t)(obj(EncFunc)))
#define DEC_FUNC(obj)       ((DecFunc_t)(obj(DecFunc)))
#define KSCHD_FUNC(obj)     ((KschdFunc_t)(obj(KschdFunc)))

#define PERMUTE_FUNC(obj)   ((PermuteFunc_t)(obj(PermuteFunc)))

#define INIT_FUNC(obj)      ((InitFunc_t)(obj(InitFunc)))
#define UPDATE_FUNC(obj)    ((UpdateFunc_t)(obj(UpdateFunc)))
#define WRITE_FUNC(obj)     ((WriteFunc_t)(obj(WriteFunc)))
#define FINAL_FUNC(obj)     ((FinalFunc_t)(obj(FinalFunc)))
#define XFINAL_FUNC(obj)    ((XFinalFunc_t)(obj(XofFinalFunc)))
#define READ_FUNC(obj)      ((ReadFunc_t)(obj(ReadFunc)))

#define KINIT_FUNC(obj)     ((KInitFunc_t)(obj(KInitFunc)))

#define AENC_FUNC(obj)      ((AEncFunc_t)(obj(AEncFunc)))
#define ADEC_FUNC(obj)      ((ADecFunc_t)(obj(ADecFunc)))

// Aliases additions for PRNG/DRBG.
#define SEED_BYTES(obj)     ((IntPtr)(obj(seedBytes)))
#define INST_INIT_FUNC(obj) ((InstInitFunc_t)(obj(InstInitFunc)))
#define RESEED_FUNC(obj)    ((ReseedFunc_t)(obj(ReseedFunc)))
#define GEN_FUNC(obj)       ((GenFunc_t)(obj(GenFunc)))

#define ERASE_STATES(buf, len)                          \
    do {                                                \
        char volatile *ba = (void volatile *)(buf);     \
        size_t l = (size_t)(len), i;                    \
        for(i=0; i<l; i++) ba[i] = 0;                   \
    } while(false)
    
#endif /* MySuiteA_mysuitea_common_h */
