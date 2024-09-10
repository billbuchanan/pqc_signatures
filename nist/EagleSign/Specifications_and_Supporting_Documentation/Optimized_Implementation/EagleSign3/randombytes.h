/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stddef.h>
#include <stdint.h>

void randombytes(uint8_t *out, size_t outlen);

#endif
