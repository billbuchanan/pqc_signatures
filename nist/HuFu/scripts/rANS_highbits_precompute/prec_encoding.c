/*
Copyright (c) 2023 Team HAETAE
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "rans_byte.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "prec_encoding_param.h"

#define SCALE (1u << SCALE_BITS)


static RansEncSymbol esyms_sig[M_SIG];
static uint16_t symbol_sig[SCALE] = {0};
static RansDecSymbol dsyms_sig[M_SIG];

void symbol_table(uint16_t *symbol, const uint32_t *freq, size_t alphabet_size) 
{
    int pos = 0;
    for (size_t sym=0; sym < alphabet_size; sym++) {
        for (uint32_t i=0; i < freq[sym]; i++)
            symbol[pos++] = sym;
    }
}

void cum_freq_table(uint32_t *cum_freq, const uint32_t *freq, size_t alphabet_size) {
    cum_freq[0] = 0;
    for (size_t i = 1; i < alphabet_size; i++) {
      cum_freq[i] = cum_freq[i-1] + freq[i-1];
    }
}

void encode_symbols(RansEncSymbol *esyms, const uint32_t *freq, size_t alphabet_size) {
    uint32_t cum_freq[alphabet_size];
    cum_freq_table(cum_freq, freq, alphabet_size);

    for (size_t i=0; i < alphabet_size; i++) 
    {
        RansEncSymbolInit(&esyms[i], cum_freq[i], freq[i], SCALE_BITS);
    }
}

void decode_symbols(RansDecSymbol *dsyms, uint16_t *symbol, const uint32_t *freq, size_t alphabet_size) {
    uint32_t cum_freq[alphabet_size];
    cum_freq_table(cum_freq, freq, alphabet_size);

    symbol_table(symbol, freq, alphabet_size);
    
    for (size_t i=0; i < alphabet_size; i++) 
    {
        RansDecSymbolInit(&dsyms[i], cum_freq[i], freq[i]);
    }
}

void precomputations_rans() {
    encode_symbols(esyms_sig, f_sig, M_SIG);
    decode_symbols(dsyms_sig, symbol_sig, f_sig, M_SIG);
}

void print_esym(RansEncSymbol x){
    printf("{%uu,%uu,%uu,%uu,%uu}", x.x_max, x.rcp_freq, x.bias, x.cmpl_freq, x.rcp_shift);
}

void print_dsym(RansDecSymbol x){
    printf("{%uu,%uu}", x.start, x.freq);
}

int main() {
    precomputations_rans();

    printf("// %s\n", TABLE_INFO);

    printf("#define M_SIG %uu \n", M_SIG);

    printf("static RansEncSymbol esyms_sig[M_SIG] = {");
    for(int i=0; i<M_SIG; i++) {
        print_esym(esyms_sig[i]);
        printf(",");
    }
    printf("};\n");

    printf("static RansDecSymbol dsyms_sig[M_SIG] = {");
    for(int i=0; i<M_SIG; i++) {
        print_dsym(dsyms_sig[i]);
        printf(",");
    }
    printf("};\n");

    printf("static uint16_t symbol_sig[SCALE] = {");
    for(int i=0; i<SCALE; i++) {
        printf("%uu,", symbol_sig[i]);
    }
    printf("};\n");
}
