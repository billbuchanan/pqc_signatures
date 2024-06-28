SageMath implementation of the sieve-and-boost approach, using
a C implementation for sieving for smooth numbers in an interval.

To run, first run `make all` in the [c subfolder](c). After that, run
```
python3 sieve-and-boost -h
```
to display options
```
usage: sieve-and-boost.py [-h] [-param_set PARAM_SET] [-R R] [-b B] [-logB LOGB] [-p PROCESSES]

options:
  -h, --help            show this help message and exit
  -param_set PARAM_SET  Choose parameter set (sorted by input size to polynomials)
  -R R                  2**R upper bound on size of smooth numbers found by sieving the interval
  -b B                  Size 2**b of subintervals sieved in one iteration
  -logB LOGB            log_2 of the smoothness bound B
  -p PROCESSES, --processes PROCESSES
                        number of processes to be started in parallel
```
