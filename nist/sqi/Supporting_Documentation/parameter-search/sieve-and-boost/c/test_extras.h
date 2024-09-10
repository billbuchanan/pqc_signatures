// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#ifndef __TEST_EXTRAS_H__
#define __TEST_EXTRAS_H__
    
#include <stdint.h>
#include <time.h> 

#define PASSED    0
#define FAILURE   1
    
// Access system counter for benchmarking
int64_t cpucycles(void);

#endif