// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
/******************************************************************************
* Sieving algorithm using the sieve of Eratosthenes                           *
******************************************************************************/
#include "test_extras.h"
#include "test_sieve.h"

bool log_sieve(digit_t T, unsigned int b, unsigned int logB, unsigned int np, 
               unsigned int* primes, unsigned char* log_primes, 
               unsigned char* numbers)
{
    unsigned int *num_bounds, i, j, exponent;
    digit_t q, k;
    sdigit_t t, kint;
    unsigned int log2T = (unsigned int)round(log2((double)T));
    unsigned int log2Tpb = (unsigned int)round(log2((double)T+b));
    double threshold;

    // Start with 0s in all positions.
    for (i = 0; i < b; i++) {
        numbers[i] = 0;
    }
    
    num_bounds = calloc(log2Tpb-log2T+2, sizeof(unsigned int));
    if(num_bounds == NULL) {
        return false;
    }
    
    // Determine bounds b[i] such that integers in [T+b[i], 
    // T+b[i+1]] have the same rounded log2 value.
    for (i = log2T, j = 1; i < log2Tpb; i++, j++) {
        t = (sdigit_t)ceil(pow(2, i + 0.5)) - T;
        num_bounds[j] = (unsigned int)t;
    }
    num_bounds[j] = b;

    // Starting threshold to determine smoothness (for first interval).
    threshold = log2T - 0.75*logB;

    // Iterate through the primes
    for (i = 0; i < np; i++) {
        // Compute the max exponent to be considered for this prime, 
        // i.e. such that p^a = R
        exponent = (unsigned int)(log2Tpb/log_primes[i]);
        // Iterate through the possible exponents.
        q = 1;
        for (j = 0; j < exponent; j++) {
            // Take the corresponding prime power.
            q *= primes[i];
            // Determine the offset for sieving.
            kint = -(sdigit_t)(T % q);
            if (kint < 0)
                k = q + kint;
            else
                k = (digit_t)kint;
            // Sieve
            while (k < b) {
                numbers[k] += log_primes[i];
                k += q;
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
    int MAX_ARGSplus1 = 4;       // Format: "test_sieve -h [T] [b] [logB]"
    digit_t T = 0;
    unsigned int b = 0, logB = 0, np, *primes, value, i;  
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
        T = strtoull(argv[1], NULL, 10);
        b = strtoul(argv[2], NULL, 10);
        logB = strtoul(argv[3], NULL, 10);
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
    file = fopen("C:\\...", "r");
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
    
    // Run the sieving function
    printf("\nSieving from {T} to {T+b}... \n");
    
    current_time = time(NULL);
    cycles1 = cpucycles();
    if (!log_sieve(T, b, logB, np, primes, log_primes, positions))  // Sieving.
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
        printf("\n test_sieve -h [T] [b] [logB] \n");
        printf("\n T    : start of the sieving interval");
        printf("\n b    : length b of the sieving interval");
        printf("\n logB : logarithm of the smoothness bound B");
        printf("\n -h   : this help\n\n");
    }
    return true;
}
