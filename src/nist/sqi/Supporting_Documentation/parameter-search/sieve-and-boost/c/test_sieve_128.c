// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/******************************************************************************
* Sieving algorithm using the sieve of Eratosthenes                           *
******************************************************************************/
#include "test_extras.h"
#include "test_sieve_128.h"

bool log_sieve_128(digit_t *T, unsigned int log2T, unsigned int b, 
                   unsigned int log2Tpb, unsigned int logB, unsigned int np, 
                   unsigned int* primes, unsigned char* log_primes, 
                   unsigned char* numbers)
{
    unsigned int *num_bounds, i, j, exponent;
    sdigit_t t;
    unsigned __int128 k, q = 0;
    __int128 kint;
    unsigned __int128 *Tpt = 0;
    double threshold;

    Tpt = T;
    
    // Start with 0s in all positions.
    for (i = 0; i < b; i++) {
        numbers[i] = 0;
    }
    
    num_bounds = calloc(log2Tpb-log2T+2, sizeof(unsigned int));
    if(num_bounds == NULL) {
        return false;
    }
    
    // Determine bounds b[i] such that integers in [T+b[i], T+b[i+1]] 
    // have the same rounded log2 value.
    for (i = log2T, j = 1; i < log2Tpb; i++, j++) {
        t = (sdigit_t)((__int128_t)ceil(pow(2, i + 0.5)) - *Tpt);
        num_bounds[j] = t;
    }
    num_bounds[j] = b;

    // Starting threshold to determine smoothness (for first interval).
    threshold = log2T - 0.75*logB;

    // Iterate through the primes
    for (i = 0; i < np; i++) {
        // Compute the max exponent to be considered for this prime, 
        // i.e. such that p^a = R.
        exponent = (unsigned int)(log2Tpb/log_primes[i]);
        // Iterate through the possible exponents.
        q = 1;
        for (j = 0; j < exponent; j++) {
            // Take the corresponding prime power.
            q *= (__uint128_t)(primes[i]);

            // Determine the offset for sieving.
            kint = -(__int128)(*Tpt % q);
            if (kint < 0) {
                k = (unsigned __int128)(q + kint);
            }
            else {
                k = (unsigned __int128)kint;
            }
            // Sieve.
            while (k < b) {
                numbers[k] += log_primes[i];
                k = (k + q);
            }
        }
    }

    // Create all-zero bitstring to mark the smooth numbers.
    // Mark the 2^logB-smooth numbers in the interval.
    for (i = 0; i < (log2Tpb-log2T+2-1); i++) { 
        for (j = num_bounds[i]; j < num_bounds[i+1]; j++) {
            if (numbers[j] > threshold)
                numbers[j] = true;  
            else
                numbers[j] = 0;              
        }    
        // Increase threshold (implicitly the rounded log2(T+j) value).
        threshold += 1;  
    }
    
    return true;
}


int main(int argc, char **argv)
{
    bool help_flag = false;
    int MAX_ARGSplus1 = 7; 
    // Format: "test_sieve_128 -h [T0] [T1] [log2T] [b] [log2Tpb] [logB]"
    digit_t T[2];
    unsigned int b = 0, logB = 0, np, *primes, value, i; 
    unsigned int log2T = 0, log2Tpb = 0; 
    unsigned char *positions, *log_primes;  
    // These are the numbers of primes up to 2^1, 2^2, 2^3,..., up to 2^25:
    unsigned int NrPrimesUpToB[25] = {1,2,4,6,11,18,31,54,97,172,309,564,1028,
                                      1900,3512,6542,12251,23000,43390,82025,
                                      155611,295947,564163,1077871,2063689};
    time_t calendar_time, current_time;
    unsigned long long cycles = 0, cycles1, cycles2;
    FILE* file;
    
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc > MAX_ARGSplus1 || argc == 1) {
        help_flag = true;
        goto help;
    }

    if (argv[1][0] != '-') {
        T[0] = (uint64_t)strtoull(argv[1], NULL, 10);
        T[1] = (uint64_t)strtoull(argv[2], NULL, 10);
        log2T = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);
        log2Tpb = strtoul(argv[5], NULL, 10);
        logB = strtoul(argv[6], NULL, 10);
    } else {
        if (argv[1][1] == 'h') {
            help_flag = true;
            goto help;
        }
    }
    
    // Currently only working with smoothness bounds at most 2**24.
    if (logB > 24) {
        printf("\nError: logB can be at most 24\n\n");
        return false;
    }
    
    // Read a precomputed table of all primes up to 2**24.
#if defined(__WINDOWS__)  // NOTE: modify path according to your system.
    file = fopen("C:\\...primes_upto_2pow24.csv", "r");
#elif defined(__LINUX__)
    file = fopen("../primes/primes_upto_2pow24.csv", "r");
#endif
    if (file == NULL) {
        printf("\nError: opening file\n\n");
        return false;
    }
    
    // Pick the set of primes corresponding to the chosen B.
    np = NrPrimesUpToB[logB-1];
    primes = calloc(np, sizeof(unsigned int));
    log_primes = calloc(np, sizeof(unsigned char));
    positions = calloc(b, sizeof(unsigned char));
    if (primes == NULL || log_primes == NULL || positions == NULL) {
        printf("\nError: memory not allocated\n\n");
        fclose(file);
        return false;
    }

    for (i = 0; i < np; i++) { 
        if (!fscanf(file, "%d%*[, \t\n]", &value)) {
            printf("\nError: scanning file\n\n");
            return false;
        } 
        primes[i] = value; 
        log_primes[i] = (unsigned char)round(log2((double)value));
    }
    
    // Run the sieving function.
    printf("\nSieving from {T} to {T+b}... \n");
    
    current_time = time(NULL);
    cycles1 = cpucycles();
    if (!log_sieve_128(T, log2T, b, log2Tpb, logB, np, primes, log_primes,
                       positions))  // Sieving.
        printf("\nError: memory not allocated or q became 0\n\n");
    calendar_time = time(NULL);
    cycles2 = cpucycles();
    cycles = cycles + (cycles2 - cycles1);
    calendar_time -= current_time;

    value = b;
    if (value > 512)
        value = 512;
    for (i = 0; i < value; i++)
        printf("%d", positions[i]);
    if (value < b)
        printf (".... ");
    printf("\n\nTotal time (one core)   : %ld sec", (long)calendar_time);
    printf("\nTotal cycles (one core) : %lld cycles\n\n", cycles);
    fclose(file);
    
help:
    if (help_flag) {
        printf("\n Usage:");
        printf("\n test_sieve -h [T0] [T1] [log2T] [b] [log2Tpb] [logB] \n");
        printf("\n T0      : start of the sieving interval (T0 = T mod 2^64)");
        printf("\n T1      : start of the sieving interval (T1 = T/2^64)");
        printf("\n log2T   : round(log_2(T))");
        printf("\n b       : length b of the sieving interval");
        printf("\n log2Tpb : round(log_2(T+b))");
        printf("\n logB    : logarithm of the smoothness bound B");
        printf("\n -h      : this help\n\n");
    }
    return true;
}
