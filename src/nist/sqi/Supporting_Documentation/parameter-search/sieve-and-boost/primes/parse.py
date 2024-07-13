# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
# parse.py

import csv
from math import log

def read_values_csv(filename, type):
    '''Read contents of a csv file into a list of values.'''
    values = []
    with open(filename, 'rt') as file: 
        rows = csv.reader(file)
        for row in rows:
            values += [type(value_str) for value_str in row]

    return values

def read_primes(logB):
    '''Read a precomputed table of all primes less than B=2**logB.'''
    primes = read_values_csv('primes/primes_upto_2pow24.csv', int)
    if logB > 24:
        primes += read_values_csv(
                            'primes/primes_from_2pow24_to_2pow25.csv', int)
    ## If the files with primes larger than 2**25 are generated, the 
    ## respective lines below can then be used. 
    # if logB > 25:
    #     primes += read_values_csv(
    #                       'primes/primes_from_2pow25_to_2pow26.csv', int)
    # if logB > 26:
    #     primes += read_values_csv(
    #                       'primes/primes_from_2pow26_to_2pow27.csv', int)
    # if logB > 27:
    #     primes += read_values_csv(
    #                       'primes/primes_from_2pow27_to_2pow28.csv', int)
    # if logB > 28:
    #     primes += read_values_csv(
    #                       'primes/primes_from_2pow28_to_2pow29.csv', int)
    # if logB > 29:
    #     primes += read_values_csv(
    #                       'primes/primes_from_2pow29_to_2pow30.csv', int)
    
    # This currently only works with smoothness bounds at most 2**25.
    if logB > 25:
        raise RuntimeError('logB can be at most 25.')

    # These are the numbers of primes up to 2**1, 2**2, 2**3,..., 2**30.
    NrPrimesUpToB = [1, 2, 4, 6, 11, 18, 31, 54, 97, 172, 309, 564, 1028, 
                     1900, 3512, 6542, 12251, 23000, 43390, 82025, 155611,
                     295947, 564163, 1077871, 2063689, 3957809, 7603553,
                     14630843, 28192750, 54400028]
    # Pick the set of primes corresponding to the chosen B.
    np = NrPrimesUpToB[logB-1]
    primes = primes[0:np]
    # Compute the rounded logarithms of these primes
    log_primes = [round(log(p,2)) for p in primes]  

    return primes, log_primes