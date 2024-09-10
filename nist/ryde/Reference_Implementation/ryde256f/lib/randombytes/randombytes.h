//
//  rng.h
//
//  Created by Bassham, Lawrence E (Fed) on 8/29/17.
//  Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
//

#ifndef RANDOMBYTES_h
#define RANDOMBYTES_h

#include <stdio.h>

#define RNG_SUCCESS      0
#define RNG_BAD_MAXLEN  -1
#define RNG_BAD_OUTBUF  -2
#define RNG_BAD_REQ_LEN -3


typedef struct {
    unsigned char   Key[32];
    unsigned char   V[16];
    int             reseed_counter;
} AES256_CTR_DRBG_struct;

void randombytes_init(unsigned char *entropy_input, unsigned char *personalization_string, int security_strength);
int randombytes(unsigned char *x, unsigned long long xlen);

#endif 
