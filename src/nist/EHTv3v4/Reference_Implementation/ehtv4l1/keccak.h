#ifndef keccak_h
#define keccak_h

#include <stddef.h>

void FIPS202_SHAKE256(const unsigned char *input, unsigned int inputByteLen, unsigned char *output, int outputByteLen);

#endif
