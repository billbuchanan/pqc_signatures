// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "test_extras.h"
#if defined(__WINDOWS__)
    #include <intrin.h>
    #include <windows.h>
#elif defined(__LINUX__)
    #include <unistd.h>
#endif 


int64_t cpucycles(void)
{ // Access system counter for benchmarking
#if defined(__WINDOWS__)
    return __rdtsc();
#elif defined(__LINUX__)
    unsigned int hi, lo;

    __asm volatile ("rdtsc\n\t" : "=a" (lo), "=d"(hi));
    return ((int64_t)lo) | (((int64_t)hi) << 32);
#else
    return 0;            
#endif
}