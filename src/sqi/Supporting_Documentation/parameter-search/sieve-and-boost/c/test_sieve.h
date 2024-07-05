// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#ifndef __TEST_SIEVE_H__
#define __TEST_SIEVE_H__
    
#include <stdbool.h> 
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>  
#include <stdio.h>
#include <math.h>

#define digit_t uint64_t
#define sdigit_t int64_t
    
// Sieving function
bool log_sieve(digit_t T, unsigned int b, unsigned int logB, unsigned int np, 
               unsigned int* primes, unsigned char* log_primes, 
               unsigned char* positions);

#endif