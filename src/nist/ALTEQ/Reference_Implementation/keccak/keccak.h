#ifndef _KECCAK_REF_H
#define _KECCAK_REF_H

void Keccak(unsigned int rate, unsigned int capacity, const uint8_t *input, uint64_t inputByteLen, uint8_t delimitedSuffix, uint8_t *output, uint64_t outputByteLen);

#endif
