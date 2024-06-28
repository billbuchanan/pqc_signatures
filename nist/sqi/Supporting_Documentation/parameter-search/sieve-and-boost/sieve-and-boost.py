# Licensed under the MIT license.
#
# Basic algorithm for finding SQIsign friendly primes, by trying
# p = 2x^n - 1 for smooth values of x, and certain choices of n
#
# Finding the smooth values of x is us done using the sieve from
# https://github.com/microsoft/twin-smooth-integers

import sys, time, datetime
import ctypes
from math import log, ceil, floor
from pathlib import Path
from primes.parse import read_primes
from sieve import sieve, c_log_sieve_64, c_log_sieve_128
from sage.all import is_pseudoprime, factor, Integer, prod, sqrt, round, prime_range, gcd
from sage.rings.generic import ProductTree


class BatchSmoothPartsExtractor:
    # Initialize with the list of primes to use
    def __init__(self, ps):
        self.z = prod(ps)

    # Given a list of xs, returns the largest part of the xs dividing some power of z
    def smoothparts(self, xs):
        rs = ProductTree(xs).remainders(self.z)
        ys = [gcd(x, pow(r, 999, x)) for r,x in zip(rs,xs)]
        return ys

# Takes in a list of smooth values (x_i).

# Returns list of primes of the form p_i = 2(2^pow2*3^pow3*x_i)^n - 1,
# if p satisfies the SQIsign constraints, with rough smoothness bound B
def boost_and_check(xs, pow2, pow3, n, B, S):
    ps = []
    p2m1 = []
    for x in xs:
        smooth = Integer((2**(pow2))*(3**(pow3))*Integer(x))
        p = 2*(smooth**n) - 1
        if is_pseudoprime(p):
            ps.append(p)
            p2m1.append(p**2 - 1)
    Ts = S.smoothparts(p2m1)
    pTs = []
    for i, T in enumerate(Ts):
        logT = round(log(T,2),4)
        p = ps[i]
        logp = round(log(p,2),4)
        if  logT > (5/4)*logp:
            pTs.append((p,T))
    return pTs


def sieve_and_boost(L, R, b, primes, log_primes, logB, proc_num, results_file,
              status_path, param_set):
    print(f'{proc_num}: Sieving from {L} to {R}...')

    # File name for logging last finished interval and prime stats.
    status_filename = status_path
    status_filename += f'/{L}_to_{R}'
    status_filename += f'_status_{proc_num}.txt'


    # Start on the left of the interval [L,R].
    T = L
    # Count the number of x values that produce twin smooth integers.
    num_x = 0
    # Count the number of x values that make p=2*f(x)-1 prime.
    num_primes = 0

    # Prepare prime data and result byte array to pass to C.
    np = len(primes)
    numbers = bytearray(b)
    numbers = memoryview(numbers)
    c_primes = (ctypes.c_int * np)(*primes)
    c_log_primes = (ctypes.c_char * np)(*log_primes)
    c_positions = (ctypes.c_char * b)(*numbers)

    # Count the number of sieve steps.
    sieve_count = 0

    b_ext = b

    # Needed for the boosting thing
    lowp2 = param_set["min_pow2"]
    highp2 = lowp2*2
    target_in = param_set["x_size"]
    ns = param_set["ns"]
    pow3dict = {}
    for pow2 in range(lowp2, highp2):
        pow3dict[pow2] = max(floor(log((target_in)/((R)*(2**pow2)), 3)), 0)

    targetCost = {
        "NIST-I": 1,
        "NIST-III" : 2.5,
        "NIST-V" : 5
    }

    boundDict = {}
    for pow2 in range(lowp2, highp2):
        for NIST_lvl, n in ns.items():
            boundDict[(pow2,n)] = int(round(((pow2*n)*targetCost[NIST_lvl])**2)) #Compute bound based on cost ≈ sqrt(B)/f

    while T < R:
        start_interval_time = time.time()
        sieve_count += 1

        if R-T < b:
            # At the end of the range, the interval might be shorter.
            # Extend the range by the maximum range occurring in the solutions
            # to overlap the intervals.
            b_ext = R - T

        #################################################
        c_log_sieve_64(T, b, logB, np, ctypes.byref(c_primes),
                    ctypes.byref(c_log_primes), ctypes.byref(c_positions))
        positions = bytearray(c_positions)
        #################################################
        after_sieving_time = time.time()

        # Run through the bitstring
        smooths = []
        for j in range(b):
            # Start at the next smooth number
            if positions[j]:
                #counter for smooth numbers
                num_x += 1
                #actual smooth number
                x = T + j
                smooths.append(x)

        for pow2 in range(lowp2, highp2):
            pow3 = pow3dict[pow2]
            for _, n in ns.items():
                B = boundDict[(pow2,n)]
                S = BatchSmoothPartsExtractor(prime_range(B)[1:])
                passed = boost_and_check(smooths, pow2, pow3, n, B, S)
                for p, Tpart in passed:
                    print(f'\n{proc_num} ', end='')
                    num_primes += 1
                    logp = log(p,2)
                    _, f= factor(p+1, limit=2)[0]
                    print(f'p = {p}')
                    print(f'logp = {logp}')
                    print(f'T = {factor(Tpart)}')
                    print(f'logT = {log(Tpart,2)}')
                    print(f'f = {f}')
                    biggestFac = factor(Tpart)[-1][0]
                    costmet = round(sqrt(biggestFac)/f,5)
                    print(f'costMetric = {costmet}')
                    # Write to file
                    with open(results_file, 'a', newline='') as sols_file:
                        sols_file.write(f'p = {p}\n')
                        sols_file.write(f'logp = {logp}\n')
                        sols_file.write(f'T = {factor(Tpart)}\n')
                        sols_file.write(f'logT = {log(Tpart,2)}\n')
                        sols_file.write(f'f = {f}\n')
                        sols_file.write(f'costMetric = {costmet}\n')

        with open(status_filename, 'w', newline='') as status_file:
            status_file.write(f'{proc_num}, {logB}, {L}, {R}, {T}, {T+b},'
                              + f' {num_x}, {num_primes}\n\n')
            status_file.write(f'Status file for process {proc_num} searching'
                              + f' for 2^{logB}-smooth numbers in the'
                              + f' range from {L} to {R}\n')
            status_file.write(f'Last finished sieve interval: [{T}, {T+b}],'
                              + f' {(T+b-L)/(R-L)*100} % done.\n')
            status_file.write(f'Number of smooth x values'
                              + f' integers in [{L}, {T+b}]: {num_x}\n')
            status_file.write(f'Number of x values that produce SQIsign-friendly prime 2*f(x)-1'
                              + f' in [{L}, {T+b}]: {num_primes}\n')

        end_interval_time = time.time()
        print(f'\n{proc_num}: Interval [{T}, {T+b-1}], {(T+b-L)/(R-L)*100} %, '
              + f'time: {round(end_interval_time - start_interval_time, 3)}s,'
              + f' spent on sieving:'
              + f' {round(after_sieving_time - start_interval_time, 3)}')
        sys.stdout.flush()

        T += b

    with open(status_filename, 'a', newline='') as status_file:
        status_file.write(f'Done!\n')


def main(args):
    import multiprocessing as mp
    # import cProfile, pstats, io

    # Different choices of polynomials for different security levels.
    param_sets = [{"min_pow2" : 70//4, "x_size" : floor(2**(384/4)), "ns" : {"NIST-III":4}},
    {"min_pow2" : 50//3, "x_size" : floor(2**(256/3)), "ns" : {"NIST-I":3, "NIST-V":6}},
    {"min_pow2" : 50//4, "x_size" : floor(2**(256/4)), "ns" : {"NIST-I":4, "NIST-III":6, "NIST-V":8}},
    {"min_pow2" : 50//6, "x_size" : floor(2**(256/6)), "ns" : {"NIST-I":6, "NIST-III":9, "NIST-V":12}},
    {"min_pow2" : 50//8, "x_size" : floor(2**(256/8)), "ns" : {"NIST-I":8, "NIST-III":12, "NIST-V":16}},
    {"min_pow2" : 50//12, "x_size" : floor(2**(256/12)), "ns" : {"NIST-I":12, "NIST-III":18, "NIST-V":24}}]
    # Choice
    param_set = param_sets[args[1]]
    # Size of smooth part which sieve is ran for finding.
    R = 2**args[2]
    L = 2**(args[2] - 1)
    if R > sqrt(param_set["x_size"]):
        print("Cannot use that big R")
        print("For this parameter set, R can be at most " + str(floor(0.5*log(param_set["x_size"],2))))
        exit()


    # Number of processes among which to divide up the full interval.
    num_proc = args[5]
    # Interval size (rounded up), the last process gets the rest.
    interval = ceil(R - L/num_proc)
    # Left and right bounds for the subintervals.
    Li = [L + i*interval for i in range(num_proc)]
    Ri = [Li[i] for i in range(1, num_proc)] + [R]
    # Length of the batches to be sieved at one time.
    b = min(2**args[3], interval)
    # Log of the smoothness bound, currently only allowing powers of 2.
    logB = args[4]
    # Read a precomputed table of all primes less than B=2**logB.
    primes, log_primes = read_primes(logB)

    # Create folders if they don't exist already.
    results_path = f'results_parameter_set_{args[1]}'
    Path(results_path).mkdir(parents=True, exist_ok=True)
    status_path = f'status_parameter_set_{args[1]}'
    Path(status_path).mkdir(parents=True, exist_ok=True)
    # File name to store value x and solutions that generate found
    # twin smooth numbers.
    results_file = results_path + f'/{L}_to_{R}.txt'
    print('Printing results to file ' + results_file)

    with open(results_file, 'a', newline='') as sols_file:
        sols_file.write(f'p values found using parameter set {args[1]},'
                        + f' sieving for numbers in the range from {L} to {R}'
                        + f' - {datetime.datetime.now()}\n')

    # #profiling
    # pr = cProfile.Profile()
    # pr.enable()

    start_time = time.time()
    # Set up and start the parallel processes.
    processes = []
    for i in range(num_proc):
        p = mp.Process(target=sieve_and_boost, args=(Li[i], Ri[i], b, primes,
                       log_primes, logB, i, results_file,
                       status_path, param_set))
        processes.append(p)
        p.start()

    for p in processes:
        p.join()

    end_time = time.time()
    print(f'Time: {round(end_time - start_time, 3)}s')

    # pr.disable()
    # s = io.StringIO()
    # sortby = 'cumulative'
    # ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
    # ps.print_stats()
    # print(s.getvalue())


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("-param_set", type=int,
                        help="Choose parameter set (sorted by input size to polynomials)")
    parser.add_argument("-R", type=int,
                        help="2**R upper bound on size of smooth numbers found by sieving the interval")
    parser.add_argument("-b", type=int,
                        help="Size 2**b of subintervals sieved in one iteration")
    parser.add_argument("-logB", type=int,
                        help="log_2 of the smoothness bound B")
    parser.add_argument("-p", "--processes", type=int, default=1,
                        help="number of processes to be started in parallel")
    args = parser.parse_args()

    filename = sys.argv[0]

    main([filename, args.param_set, args.R, args.b, args.logB, args.processes])
