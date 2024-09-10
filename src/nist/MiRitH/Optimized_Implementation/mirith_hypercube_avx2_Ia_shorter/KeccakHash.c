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

#include <string.h>
#include "KeccakHash.h"

#ifdef XKCP_has_KeccakP1600
    //#include "KeccakP-1600-SnP.h"

    #define prefix KeccakWidth1600
    #define SnP KeccakP1600
    #define SnP_width 1600
    #define SnP_Permute KeccakP1600_Permute_24rounds
    #if defined(KeccakF1600_FastLoop_supported)
        #define SnP_FastLoop_Absorb KeccakF1600_FastLoop_Absorb
    #endif
        #include "KeccakSponge.inc"
    #undef prefix
    #undef SnP
    #undef SnP_width
    #undef SnP_Permute
    #undef SnP_FastLoop_Absorb
#endif

#ifdef XKCP_has_KeccakP1600
    //#include "KeccakP-1600-SnP.h"

    #define prefix KeccakWidth1600_12rounds
    #define SnP KeccakP1600
    #define SnP_width 1600
    #define SnP_Permute KeccakP1600_Permute_12rounds
    #if defined(KeccakP1600_12rounds_FastLoop_supported)
        #define SnP_FastLoop_Absorb KeccakP1600_12rounds_FastLoop_Absorb
    #endif
        #include "KeccakSponge.inc"
    #undef prefix
    #undef SnP
    #undef SnP_width
    #undef SnP_Permute
    #undef SnP_FastLoop_Absorb
#endif

/* ---------------------------------------------------------------- */

HashReturn Keccak_HashInitialize(Keccak_HashInstance *instance, unsigned int rate, unsigned int capacity, unsigned int hashbitlen, unsigned char delimitedSuffix)
{
    HashReturn result;

    if (delimitedSuffix == 0)
        return KECCAK_FAIL;
    result = (HashReturn)KeccakWidth1600_SpongeInitialize(&instance->sponge, rate, capacity);
    if (result != KECCAK_SUCCESS)
        return result;
    instance->fixedOutputLength = hashbitlen;
    instance->delimitedSuffix = delimitedSuffix;
    return KECCAK_SUCCESS;
}

/* ---------------------------------------------------------------- */

HashReturn Keccak_HashUpdate(Keccak_HashInstance *instance, const BitSequence *data, BitLength databitlen)
{
    if ((databitlen % 8) == 0)
        return (HashReturn)KeccakWidth1600_SpongeAbsorb(&instance->sponge, data, databitlen/8);
    else {
        HashReturn ret = (HashReturn)KeccakWidth1600_SpongeAbsorb(&instance->sponge, data, databitlen/8);
        if (ret == KECCAK_SUCCESS) {
            /* The last partial byte is assumed to be aligned on the least significant bits */
            unsigned char lastByte = data[databitlen/8];
            /* Concatenate the last few bits provided here with those of the suffix */
            unsigned short delimitedLastBytes = (unsigned short)((unsigned short)(lastByte & ((1 << (databitlen % 8)) - 1)) | ((unsigned short)instance->delimitedSuffix << (databitlen % 8)));
            if ((delimitedLastBytes & 0xFF00) == 0x0000) {
                instance->delimitedSuffix = delimitedLastBytes & 0xFF;
            }
            else {
                unsigned char oneByte[1];
                oneByte[0] = delimitedLastBytes & 0xFF;
                ret = (HashReturn)KeccakWidth1600_SpongeAbsorb(&instance->sponge, oneByte, 1);
                instance->delimitedSuffix = (delimitedLastBytes >> 8) & 0xFF;
            }
        }
        return ret;
    }
}

/* ---------------------------------------------------------------- */

HashReturn Keccak_HashFinal(Keccak_HashInstance *instance, BitSequence *hashval)
{
    HashReturn ret = (HashReturn)KeccakWidth1600_SpongeAbsorbLastFewBits(&instance->sponge, instance->delimitedSuffix);
    if (ret == KECCAK_SUCCESS)
        return (HashReturn)KeccakWidth1600_SpongeSqueeze(&instance->sponge, hashval, instance->fixedOutputLength/8);
    else
        return ret;
}

/* ---------------------------------------------------------------- */

HashReturn Keccak_HashSqueeze(Keccak_HashInstance *instance, BitSequence *data, BitLength databitlen)
{
    if ((databitlen % 8) != 0)
        return KECCAK_FAIL;
    return (HashReturn)KeccakWidth1600_SpongeSqueeze(&instance->sponge, data, databitlen/8);
}


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

//#include "KeccakSponge.h"
//#include "SimpleFIPS202.h"

int SHAKE128(unsigned char *output, size_t outputByteLen, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge(1344, 256, input, inputByteLen, 0x1F, output, outputByteLen);
}

int SHAKE256(unsigned char *output, size_t outputByteLen, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge(1088, 512, input, inputByteLen, 0x1F, output, outputByteLen);
}

int SHA3_224(unsigned char *output, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge(1152, 448, input, inputByteLen, 0x06, output, 224/8);
}

int SHA3_256(unsigned char *output, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge(1088, 512, input, inputByteLen, 0x06, output, 256/8);
}

int SHA3_384(unsigned char *output, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge( 832, 768, input, inputByteLen, 0x06, output, 384/8);
}

int SHA3_512(unsigned char *output, const unsigned char *input, size_t inputByteLen)
{
    return KeccakWidth1600_Sponge(576, 1024, input, inputByteLen, 0x06, output, 512/8);
}
