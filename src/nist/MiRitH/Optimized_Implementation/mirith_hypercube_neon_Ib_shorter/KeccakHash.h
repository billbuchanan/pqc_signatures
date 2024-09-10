#ifndef KECCAK
#define KECCAK

/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

Implementation by Gilles Van Assche and Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

//#ifndef _align_h_
//#define _align_h_

/* on Mac OS-X and possibly others, ALIGN(x) is defined in param.h, and -Werror chokes on the redef. */
#ifdef ALIGN
#undef ALIGN
#endif

#if defined(__GNUC__)
#define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#define ALIGN(x) __declspec(align(x))
#elif defined(__ARMCC_VERSION)
#define ALIGN(x) __align(x)
#else
#define ALIGN(x)
#endif

//#endif

/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 20/12/2007
 Changes for ARM 9/9/2010
*/

//#ifndef _BRG_ENDIAN_H
//#define _BRG_ENDIAN_H

#define IS_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */
#define IS_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */

/* Now attempt to set the define for platform byte order using any  */
/* of the four forms SYMBOL, _SYMBOL, __SYMBOL & __SYMBOL__, which  */
/* seem to encompass most endian symbol definitions                 */

#if defined( BIG_ENDIAN ) && defined( LITTLE_ENDIAN )
#  if defined( BYTE_ORDER ) && BYTE_ORDER == BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( BYTE_ORDER ) && BYTE_ORDER == LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( _BIG_ENDIAN ) && defined( _LITTLE_ENDIAN )
#  if defined( _BYTE_ORDER ) && _BYTE_ORDER == _BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( _BYTE_ORDER ) && _BYTE_ORDER == _LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( _BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( _LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( __BIG_ENDIAN ) && defined( __LITTLE_ENDIAN )
#  if defined( __BYTE_ORDER ) && __BYTE_ORDER == __BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( __BYTE_ORDER ) && __BYTE_ORDER == __LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( __BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( __LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( __BIG_ENDIAN__ ) && defined( __LITTLE_ENDIAN__ )
#  if defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __BIG_ENDIAN__
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __LITTLE_ENDIAN__
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( __BIG_ENDIAN__ )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( __LITTLE_ENDIAN__ )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

/*  if the platform byte order could not be determined, then try to */
/*  set this define using common machine defines                    */
#if !defined(PLATFORM_BYTE_ORDER)

#if   defined( __alpha__ ) || defined( __alpha ) || defined( i386 )       || \
      defined( __i386__ )  || defined( _M_I86 )  || defined( _M_IX86 )    || \
      defined( __OS2__ )   || defined( sun386 )  || defined( __TURBOC__ ) || \
      defined( vax )       || defined( vms )     || defined( VMS )        || \
      defined( __VMS )     || defined( _M_X64 )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN

#elif defined( AMIGA )    || defined( applec )    || defined( __AS400__ )  || \
      defined( _CRAY )    || defined( __hppa )    || defined( __hp9000 )   || \
      defined( ibm370 )   || defined( mc68000 )   || defined( m68k )       || \
      defined( __MRC__ )  || defined( __MVS__ )   || defined( __MWERKS__ ) || \
      defined( sparc )    || defined( __sparc)    || defined( SYMANTEC_C ) || \
      defined( __VOS__ )  || defined( __TIGCC__ ) || defined( __TANDEM )   || \
      defined( THINK_C )  || defined( __VMCMS__ ) || defined( _AIX )       || \
      defined( __s390__ ) || defined( __s390x__ ) || defined( __zarch__ )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN

#elif defined(__arm__)
# ifdef __BIG_ENDIAN
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
# else
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
# endif
#elif 1     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#else
#  error Please edit lines 132 or 134 in brg_endian.h to set the platform byte order
#endif

#endif

//#endif


/* File generated by ToTargetConfigFile.xsl */
#define XKCP_has_Sponge_Keccak
#define XKCP_has_FIPS202
//#define XKCP_has_SP800_185
//#define XKCP_has_Duplex_Keccak
//#define XKCP_has_PRG_Keccak
//#define XKCP_has_Ketje
//#define XKCP_has_Keyak
//#define XKCP_has_KangarooTwelve
//#define XKCP_has_Kravatte
//#define XKCP_has_Xoofff
//#define XKCP_has_Xoodyak
//#define XKCP_has_KeccakP200
//#define XKCP_has_KeccakP400
//#define XKCP_has_KeccakP800
#define XKCP_has_KeccakP1600
//#define XKCP_has_KeccakP1600times2
//#define XKCP_has_KeccakP1600times4
//#define XKCP_has_KeccakP1600times8
//#define XKCP_has_Xoodoo
//#define XKCP_has_Xoodootimes4
//#define XKCP_has_Xoodootimes8
//#define XKCP_has_Xoodootimes16



/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

The Keccak-p permutations, designed by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche.

Implementation by Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/

---

Please refer to SnP-documentation.h for more details.
*/

//#ifndef _KeccakP_1600_SnP_h_
//#define _KeccakP_1600_SnP_h_

#include <stddef.h>
#define KeccakP1600_implementation_config "all rounds unrolled"
#define KeccakP1600_implementation      "generic 64-bit optimized implementation (" KeccakP1600_implementation_config ")"
#define KeccakP1600_stateSizeInBytes    200
#define KeccakP1600_stateAlignment      8
//#define KeccakF1600_FastLoop_supported
//#define KeccakP1600_12rounds_FastLoop_supported
#define KeccakP1600_fullUnrolling

#define KeccakP1600_StaticInitialize()
void KeccakP1600_Initialize(void *state);
void KeccakP1600_AddByte(void *state, unsigned char data, unsigned int offset);
void KeccakP1600_AddBytes(void *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600_OverwriteBytes(void *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600_OverwriteWithZeroes(void *state, unsigned int byteCount);
void KeccakP1600_Permute_Nrounds(void *state, unsigned int nrounds);
void KeccakP1600_Permute_12rounds(void *state);
void KeccakP1600_Permute_24rounds(void *state);
void KeccakP1600_ExtractBytes(const void *state, unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600_ExtractAndAddBytes(const void *state, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length);

//#endif






/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

Keccak, designed by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche.

Implementation by the designers, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

//#ifndef _KeccakSponge_h_
//#define _KeccakSponge_h_

/* For the documentation, please follow the link: */
/* #include "KeccakSponge-documentation.h" */
#include <string.h>

#define XKCP_DeclareSpongeStructure(prefix, size, alignment) \
    ALIGN(alignment) typedef struct prefix##_SpongeInstanceStruct { \
        unsigned char state[size]; \
        unsigned int rate; \
        unsigned int byteIOIndex; \
        int squeezing; \
    } prefix##_SpongeInstance;

#define XKCP_DeclareSpongeFunctions(prefix) \
    int prefix##_Sponge(unsigned int rate, unsigned int capacity, const unsigned char *input, size_t inputByteLen, unsigned char suffix, unsigned char *output, size_t outputByteLen); \
    int prefix##_SpongeInitialize(prefix##_SpongeInstance *spongeInstance, unsigned int rate, unsigned int capacity); \
    int prefix##_SpongeAbsorb(prefix##_SpongeInstance *spongeInstance, const unsigned char *data, size_t dataByteLen); \
    int prefix##_SpongeAbsorbLastFewBits(prefix##_SpongeInstance *spongeInstance, unsigned char delimitedData); \
    int prefix##_SpongeSqueeze(prefix##_SpongeInstance *spongeInstance, unsigned char *data, size_t dataByteLen);

#ifdef XKCP_has_KeccakP1600
	//#include "KeccakP-1600-SnP.h"
    XKCP_DeclareSpongeStructure(KeccakWidth1600, KeccakP1600_stateSizeInBytes, KeccakP1600_stateAlignment)
    XKCP_DeclareSpongeFunctions(KeccakWidth1600)
    #define XKCP_has_Sponge_Keccak_width1600
#endif

#ifdef XKCP_has_KeccakP1600
    //#include "KeccakP-1600-SnP.h"
    XKCP_DeclareSpongeStructure(KeccakWidth1600_12rounds, KeccakP1600_stateSizeInBytes, KeccakP1600_stateAlignment)
    XKCP_DeclareSpongeFunctions(KeccakWidth1600_12rounds)
#endif
//#endif

/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

Keccak, designed by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche.

Implementation by the designers, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

//#ifndef _KeccakHashInterface_h_
//#define _KeccakHashInterface_h_

//#include "config.h"
//#ifdef XKCP_has_KeccakP1600

#include <stdint.h>
#include <string.h>
//#include "KeccakSponge.h"

#ifndef _Keccak_BitTypes_
#define _Keccak_BitTypes_
typedef uint8_t BitSequence;

typedef size_t BitLength;
#endif

typedef enum { KECCAK_SUCCESS = 0, KECCAK_FAIL = 1, KECCAK_BAD_HASHLEN = 2 } HashReturn;

typedef struct {
    KeccakWidth1600_SpongeInstance sponge;
    unsigned int fixedOutputLength;
    unsigned char delimitedSuffix;
} Keccak_HashInstance;

/**
  * Function to initialize the Keccak[r, c] sponge function instance used in sequential hashing mode.
  * @param  hashInstance    Pointer to the hash instance to be initialized.
  * @param  rate        The value of the rate r.
  * @param  capacity    The value of the capacity c.
  * @param  hashbitlen  The desired number of output bits,
  *                     or 0 for an arbitrarily-long output.
  * @param  delimitedSuffix Bits that will be automatically appended to the end
  *                         of the input message, as in domain separation.
  *                         This is a byte containing from 0 to 7 bits
  *                         formatted like the @a delimitedData parameter of
  *                         the Keccak_SpongeAbsorbLastFewBits() function.
  * @pre    One must have r+c=1600 and the rate a multiple of 8 bits in this implementation.
  * @return KECCAK_SUCCESS if successful, KECCAK_FAIL otherwise.
  */
HashReturn Keccak_HashInitialize(Keccak_HashInstance *hashInstance, unsigned int rate, unsigned int capacity, unsigned int hashbitlen, unsigned char delimitedSuffix);

/** Macro to initialize a SHAKE128 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHAKE128(hashInstance)        Keccak_HashInitialize(hashInstance, 1344,  256,   0, 0x1F)

/** Macro to initialize a SHAKE256 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHAKE256(hashInstance)        Keccak_HashInitialize(hashInstance, 1088,  512,   0, 0x1F)

/** Macro to initialize a SHA3-224 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHA3_224(hashInstance)        Keccak_HashInitialize(hashInstance, 1152,  448, 224, 0x06)

/** Macro to initialize a SHA3-256 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHA3_256(hashInstance)        Keccak_HashInitialize(hashInstance, 1088,  512, 256, 0x06)

/** Macro to initialize a SHA3-384 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHA3_384(hashInstance)        Keccak_HashInitialize(hashInstance,  832,  768, 384, 0x06)

/** Macro to initialize a SHA3-512 instance as specified in the FIPS 202 standard.
  */
#define Keccak_HashInitialize_SHA3_512(hashInstance)        Keccak_HashInitialize(hashInstance,  576, 1024, 512, 0x06)

/**
  * Function to give input data to be absorbed.
  * @param  hashInstance    Pointer to the hash instance initialized by Keccak_HashInitialize().
  * @param  data        Pointer to the input data.
  *                     When @a databitLen is not a multiple of 8, the last bits of data must be
  *                     in the least significant bits of the last byte (little-endian convention).
  *                     In this case, the (8 - @a databitLen mod 8) most significant bits
  *                     of the last byte are ignored.
  * @param  databitLen  The number of input bits provided in the input data.
  * @pre    In the previous call to Keccak_HashUpdate(), databitlen was a multiple of 8.
  * @return KECCAK_SUCCESS if successful, KECCAK_FAIL otherwise.
  */
HashReturn Keccak_HashUpdate(Keccak_HashInstance *hashInstance, const BitSequence *data, BitLength databitlen);

/**
  * Function to call after all input blocks have been input and to get
  * output bits if the length was specified when calling Keccak_HashInitialize().
  * @param  hashInstance    Pointer to the hash instance initialized by Keccak_HashInitialize().
  * If @a hashbitlen was not 0 in the call to Keccak_HashInitialize(), the number of
  *     output bits is equal to @a hashbitlen.
  * If @a hashbitlen was 0 in the call to Keccak_HashInitialize(), the output bits
  *     must be extracted using the Keccak_HashSqueeze() function.
  * @param  hashval     Pointer to the buffer where to store the output data.
  * @return KECCAK_SUCCESS if successful, KECCAK_FAIL otherwise.
  */
HashReturn Keccak_HashFinal(Keccak_HashInstance *hashInstance, BitSequence *hashval);

 /**
  * Function to squeeze output data.
  * @param  hashInstance    Pointer to the hash instance initialized by Keccak_HashInitialize().
  * @param  data        Pointer to the buffer where to store the output data.
  * @param  databitlen  The number of output bits desired (must be a multiple of 8).
  * @pre    Keccak_HashFinal() must have been already called.
  * @pre    @a databitlen is a multiple of 8.
  * @return KECCAK_SUCCESS if successful, KECCAK_FAIL otherwise.
  */
HashReturn Keccak_HashSqueeze(Keccak_HashInstance *hashInstance, BitSequence *data, BitLength databitlen);

//#else
//#error This requires an implementation of Keccak-p[1600]
//#endif
//#endif







/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

Keccak, designed by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche.

Implementation by Gilles Van Assche, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

//#ifndef _SimpleFIPS202_h_
//#define _SimpleFIPS202_h_
//#ifdef XKCP_has_KeccakP1600

#include <string.h>

/** Implementation of the SHAKE128 extendable output function (XOF) [FIPS 202].
  * @param  output          Pointer to the output buffer.
  * @param  outputByteLen   The desired number of output bytes.
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHAKE128(unsigned char *output, size_t outputByteLen, const unsigned char *input, size_t inputByteLen);

/** Implementation of the SHAKE256 extendable output function (XOF) [FIPS 202].
  * @param  output          Pointer to the output buffer.
  * @param  outputByteLen   The desired number of output bytes.
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHAKE256(unsigned char *output, size_t outputByteLen, const unsigned char *input, size_t inputByteLen);

/** Implementation of SHA3-224 [FIPS 202].
  * @param  output          Pointer to the output buffer (28 bytes).
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHA3_224(unsigned char *output, const unsigned char *input, size_t inputByteLen);

/** Implementation of SHA3-256 [FIPS 202].
  * @param  output          Pointer to the output buffer (32 bytes).
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHA3_256(unsigned char *output, const unsigned char *input, size_t inputByteLen);

/** Implementation of SHA3-384 [FIPS 202].
  * @param  output          Pointer to the output buffer (48 bytes).
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHA3_384(unsigned char *output, const unsigned char *input, size_t inputByteLen);

/** Implementation of SHA3-512 [FIPS 202].
  * @param  output          Pointer to the output buffer (64 bytes).
  * @param  input           Pointer to the input message.
  * @param  inputByteLen    The length of the input message in bytes.
  * @return 0 if successful, 1 otherwise.
  */
int SHA3_512(unsigned char *output, const unsigned char *input, size_t inputByteLen);

//#else
//#error This requires an implementation of Keccak-p[1600]
//#endif
//#endif



#endif //define KECCAK
