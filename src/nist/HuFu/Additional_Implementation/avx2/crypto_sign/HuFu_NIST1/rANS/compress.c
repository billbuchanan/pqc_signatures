#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "compress.h"
#include "rans_byte.h"
#include "highbits_compress_params_sec1.h"


uint16_t compress_sig(uint8_t buf[PARAM_SIG_ENCODE_MAX_BYTES], const int16_t sig[PARAM_SIG_SIZE])
{
    RansState rans;
    uint8_t *ptr_highbits = buf + PARAM_SIG_HIGHBIT_ENCODE_MAX_BYTES;
    uint8_t *ptr_lowbits = buf + PARAM_SIG_ENCODE_MAX_BYTES;
    uint16_t s;

    RansEncInit(&rans);

    for (size_t i = 0; i < PARAM_SIG_SIZE; i++)
    {
        if (sig[i] < -PARAM_TRUNC || sig[i] > PARAM_TRUNC)
        {
            return 0; // out of range
        }
    }

    for (size_t i = PARAM_SIG_SIZE; i > 0; i--)
    {
        if (sig[i - 1] < 0)
        {
            s = (uint16_t)(-sig[i - 1]);
            *(--ptr_lowbits) = 0x80 | (s & 0x7f);
        }
        else
        {
            s = (uint16_t)(sig[i - 1]);
            *(--ptr_lowbits) = s & 0x7f;
        }

        RansEncPutSymbol(&rans, &ptr_highbits, &esyms_sig[s >> 7]);
    }
    RansEncFlush(&rans, &ptr_highbits);

    const uint16_t buf_len = (uint16_t)(buf + PARAM_SIG_ENCODE_MAX_BYTES - ptr_highbits);
// printf("origin: %lu, compress: %u, avg:%4.2lf, table_size:%4.2lf KB, max_size: %d\n",PARAM_SIG_SIZE*sizeof(uint32_t),buf_len,(double)buf_len/PARAM_SIG_SIZE*8,(sizeof(esyms_sig)+sizeof(dsyms_sig)+sizeof(symbol_sig))/1024.0,PARAM_SIG_ENCODE_MAX_BYTES);
    return buf_len;
}

int decompress_sig(int16_t sig[PARAM_SIG_SIZE], const uint8_t *buf, const uint16_t buf_len)
{
    if (buf_len > PARAM_SIG_ENCODE_MAX_BYTES)
    {
        return 0; // invalid length
    }

    RansState rans;
    const uint8_t * const ptr_lowbits_start = buf + buf_len - PARAM_SIG_SIZE;
    const uint8_t *ptr_highbits = buf;
    const uint8_t *ptr_lowbits = ptr_lowbits_start;
    uint16_t s;

    RansDecInit(&rans, &ptr_highbits);

    for (size_t i = 0; i < PARAM_SIG_SIZE; i++)
    {
        if(ptr_highbits > ptr_lowbits_start){
            return 0; // out of range
        }

        s = symbol_sig[RansDecGet(&rans, SCALE_BITS)];
        sig[i] = (s << 7) | ((*(ptr_lowbits)) & 0x7f);

        if ((*(ptr_lowbits++)) & 0x80)
        {
            if (sig[i] == 0)
            {
                return 0; // -0
            }
            sig[i] = -sig[i];
        }

        if (sig[i] < -PARAM_TRUNC || sig[i] > PARAM_TRUNC)
        {
            return 0; // out of range
        }

        RansDecAdvanceSymbol(&rans, &ptr_highbits, &dsyms_sig[s], SCALE_BITS);
    }

    return 1; //success
}