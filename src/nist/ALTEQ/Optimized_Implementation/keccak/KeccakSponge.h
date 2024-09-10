/* File modified from the original XKCP package */
/* Modification is for AVX2 enabled. Note that faster implemetations exists, notably for AVX512 */

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

#ifndef _KeccakSponge_h_
#define _KeccakSponge_h_

/* For the documentation, please follow the link: */
/* #include "KeccakSponge-documentation.h" */

#include <string.h>
#include "align.h"
#include <stdint.h>

#define XKCP_DeclareSpongeStructure(prefix, size, alignment) \
    ALIGN(alignment) typedef struct prefix##_SpongeInstanceStruct { \
        uint8_t state[size]; \
        unsigned int rate; \
        unsigned int byteIOIndex; \
        int squeezing; \
    } prefix##_SpongeInstance;

#define XKCP_DeclareSpongeFunctions(prefix) \
    int prefix##_Sponge(unsigned int rate, unsigned int capacity, const uint8_t *input, size_t inputByteLen, uint8_t suffix, uint8_t *output, size_t outputByteLen); \
    int prefix##_SpongeInitialize(prefix##_SpongeInstance *spongeInstance, unsigned int rate, unsigned int capacity); \
    int prefix##_SpongeAbsorb(prefix##_SpongeInstance *spongeInstance, const uint8_t *data, size_t dataByteLen); \
    int prefix##_SpongeAbsorbLastFewBits(prefix##_SpongeInstance *spongeInstance, uint8_t delimitedData); \
    int prefix##_SpongeSqueeze(prefix##_SpongeInstance *spongeInstance, uint8_t *data, size_t dataByteLen);

#include "KeccakP-1600-SnP.h"
XKCP_DeclareSpongeStructure(KeccakWidth1600, KeccakP1600_stateSizeInBytes, KeccakP1600_stateAlignment)
XKCP_DeclareSpongeFunctions(KeccakWidth1600)
#define XKCP_has_Sponge_Keccak_width1600

#endif
