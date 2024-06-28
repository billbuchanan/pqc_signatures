# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
# sieve.py
#
# Sieving algorithms to identify smooth integers.

import sys, time
import ctypes
from math import floor, log, log2, ceil
from primes.parse import read_primes


def sieve(T, b, logB, primes):
    """Pure python sieve to find smooth numbers
    Arguments: 
    T: the starting integer for the sieve,
    b: the length of the interval that will be sieved [T, T+b),
    logB: the sieve identifies 2**logB-smooth integers,
    primes: the list of primes that are less than 2**logB.
    
    Returns a bytearray containing only 0x00 or 0x01 indicating the 
    smooth integers are in the positions with 0x01 in the interval.
    """
    numbers = [1 for j in range(b)]  # Start with 1s in all positions.
    logTpb = log(T+b)

    for p in primes:
        # Compute the max exponent to be considered for this prime.
        # I.e. such that p^a = R.
        exponent = floor(logTpb/log(p))
        # Iterate through the possible exponents
        q = 1
        for _ in range(exponent):
            # Update to the corresponding prime power
            q *= p
            # Determine the offset for sieving
            j = (-T) % q
            # Sieve
            while j < b:
                numbers[j] = numbers[j]*p
                j += q

    # Create all-zero byte array to mark the smooth numbers.
    positions = bytearray(b)
    # Mark the 2**logB-smooth numbers in the interval.
    for j in range(b):
        if numbers[j]==T+j:
            positions[j] = 1
    
    return positions


def log_sieve(T, b, logB, primes, log_primes):
    """Pure python sieve to find smooth numbers
    Arguments: 
    T: the starting integer for the sieve,
    b: the length of the interval that will be sieved [T, T+b),
    logB: the sieve identifies 2**logB-smooth integers,
    primes: the list of primes that are less than 2**logB,
    log_primes: rounded logarithms of the primes.
    
    Returns a bytearray containing only 0x00 or 0x01 indicating the 
    smooth integers are in the positions with 0x01 in the interval.

    This algorithm is approximate as it works with rounded logs.
    """
    numbers = bytearray(b)  # Start with 0s in all positions.
    numbers = memoryview(numbers)  
    log2T = round(log2(T))   
    log2Tpb = round(log2(T+b))
    # Determine bounds b[i] such that integers in [T+b[i], T+b[i+1]] have 
    # the same rounded log2 value.
    num_bounds = [0]+[ceil(2**(l + 0.5))-T for l in range(log2T, log2Tpb)]+[b]
    # Starting threshold to determine smoothness (for first interval)
    threshold = log2T - 0.75*logB

    for k,p in enumerate(primes):
        # Compute the max exponent to be considered for this prime.
        # I.e. such that p^a = R.
        exponent = floor(log2Tpb/log_primes[k])
        # Iterate through the possible exponents
        q = 1
        for _ in range(exponent):
            # Update to the corresponding prime power
            q *= p
            # Determine the offset for sieving
            j = (-T) % q
            # Sieve
            while j < b:
                numbers[j] += log_primes[k]
                j += q

    # Create all-zero byte array to mark the smooth numbers.
    positions = bytearray(b)
    # Mark the 2^logB-smooth numbers in the interval.
    for i in range(len(num_bounds)-1): 
        for j in range(num_bounds[i], num_bounds[i+1]):
            if numbers[j] > threshold:
                positions[j] = 1
        # Increase threshold (implicitly the rounded log2(T+j) value)
        threshold += 1
    
    return positions


def c_log_sieve_64(T, b, logB, np, c_primes, c_log_primes, c_log_positions):
    '''Sieve to find smooth numbers calling a faster C function.
    Arguments: 
    T: the starting integer for the sieve,
    b: the length of the interval that will be sieved [T, T+b),
    logB: the sieve identifies 2**logB-smooth integers,
    np: the number of primes less than 2**logB,
    c_primes: pointer to the list of primes less than 2**logB,
    c_log_primes: pointer to their rounded logarithms,
    c_log_positions: pointer to bytearray for the result.

    This code calls the 64-bit version of the C code, which 
    requires that T+b is less than 2**64.
    '''
    libsieve = ctypes.CDLL("c/libsieve.so")
    libsieve.log_sieve(ctypes.c_uint64(T), ctypes.c_uint(b), 
                       ctypes.c_uint(logB), ctypes.c_uint(np), 
                       c_primes, c_log_primes, c_log_positions)


def c_log_sieve_128(T, b, logB, np, c_primes, c_log_primes, c_log_positions):
    '''Sieve to find smooth numbers calling a faster C function.
    Arguments: 
    T: the starting integer for the sieve,
    b: the length of the interval that will be sieved [T, T+b),
    logB: the sieve identifies 2**logB-smooth integers,
    np: the number of primes less than 2**logB,
    c_primes: pointer to the list of primes less than 2**logB,
    c_log_primes: pointer to their rounded logarithms,
    c_log_positions: pointer to bytearray for the result.

    This code calls the 128-bit version of the C code, which 
    requires that T+b is less than 2**127. It uses __int128 
    data types. 
    '''
    log2T = round(log2(T))
    log2Tpb = round(log2(T+b))

    libsieve = ctypes.CDLL("c/libsieve128.so") 
    T0 = T % 2**64
    T1 = int((T-T0)/2**64)
    Tpt = (ctypes.c_uint64 * 2)(*[T0,T1])
    libsieve.log_sieve_128(ctypes.byref(Tpt), ctypes.c_uint(log2T), 
                       ctypes.c_uint(b), ctypes.c_uint(log2Tpb), 
                       ctypes.c_uint(logB), ctypes.c_uint(np), 
                       c_primes, c_log_primes, c_log_positions)
    
def main(args):
    # import cProfile, pstats, io

    # Left bound of the interval to be sieved (included).
    T = args[1]

    # Length of the batches to be sieved at one time.
    b = args[2]

    # Log of the smoothness bound, currently only allowing powers of 2.
    logB = args[3]
    
    # Read a precomputed table of all primes less than B=2**logB.
    primes, log_primes = read_primes(logB)
    np = len(primes)
     
    # #profiling
    # pr = cProfile.Profile()
    # pr.enable()
    
    print(f'Sieving from {T} to {T+b}...')
    start_interval_time = time.time()

    positions = sieve(T, b, logB, primes)

    end_interval_time = time.time()
    print(positions[:100])
    print(f'Interval [{T}, {T+b-1}], '
          + f'time: {round(end_interval_time - start_interval_time, 3)}s\n')

    # Now sieve with the approximate version using rounded logs.
    print(f'Sieving from {T} to {T+b} with logs...')
    start_interval_time = time.time()

    log_positions = log_sieve(T, b, logB, primes, log_primes)

    end_interval_time = time.time()
    print(log_positions[:100])
    print(f'Interval [{T}, {T+b-1}], '
          + f'time: {round(end_interval_time - start_interval_time, 3)}s')

    print(f'\nResults equal? {positions == log_positions}')
    diff = [positions[i] - log_positions[i] for i in range(b)]
    print(f'Wrong: {sum([abs(diff[i]) for i in range(len(diff))])}/{b}\n')

    # Use the C implementation of the log sieve
    use_c = args[4]
    if use_c == 64 or use_c == 128:
        print(f'Sieving from {T} to {T+b} with logs using C implementation...')
        # Start with 0s in all positions.
        numbers = bytearray(b)
        numbers = memoryview(numbers)
        # Prepare prime data and result byte array to pass to C.
        c_primes = (ctypes.c_int * np)(*primes)
        c_log_primes = (ctypes.c_char * np)(*log_primes)
        c_numbers = (ctypes.c_char * b)(*numbers)
    
        start_interval_time = time.time()   

        if use_c == 64:
            c_log_sieve_64(T, b, logB, np, ctypes.byref(c_primes), 
                           ctypes.byref(c_log_primes), 
                           ctypes.byref(c_numbers))
        elif use_c == 128:
            c_log_sieve_128(T, b, logB, np, ctypes.byref(c_primes), 
                           ctypes.byref(c_log_primes), 
                           ctypes.byref(c_numbers))

        end_interval_time = time.time()

        c_log_positions = bytearray(c_numbers)
        print(c_log_positions[0:100])
        print(f'Interval [{T}, {T+b-1}], '
              + f'time: {round(end_interval_time - start_interval_time, 3)}s')

        print(f'c result equal to python result: {c_log_positions == log_positions}')
        diff = [c_log_positions[i] - log_positions[i] for i in range(b)]
        print(f'Wrong: {sum([abs(diff[i]) for i in range(len(diff))])}/{b}')

    # pr.disable()
    # s = io.StringIO()
    # sortby = 'cumulative'
    # ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
    # ps.print_stats()
    # print(s.getvalue())
 

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("T", type=int, help="start of the sieving interval")
    parser.add_argument("b", type=int, 
                        help="length b of the sieving interval")
    parser.add_argument("logB", type=int, 
                        help="logarithm of the smoothness bound B")
    parser.add_argument("-c", "--use_c", type=int, default=0, 
                        help="use the 64- or 128-bit C code (USE_C = 64/128)")
    args = parser.parse_args()

    filename = sys.argv[0]
    
    main([filename, args.T, args.b, args.logB, args.use_c])
    