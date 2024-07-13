/*
 *  SPDX-License-Identifier: MIT
 */

#ifndef RANDOMNESS_H
#define RANDOMNESS_H

#include <stddef.h>
#include <stdint.h>

int rand_bytes(uint8_t* dst, size_t num_bytes);

#endif
