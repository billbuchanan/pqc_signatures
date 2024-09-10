# EagleSign

This repository contains the official reference implementation of the [EagleSign] signature scheme, and an optimized implementation for x86 CPUs supporting the AVX2 instruction set. 
Note that our implementation is inspired from Dilithium and Falcon Signature shemes.

## Build instructions

The implementations contain several test and benchmarking programs and a Makefile to facilitate compilation.

### Prerequisites

Some of the test programs require [OpenSSL](https://openssl.org). If the OpenSSL header files and/or shared libraries do not lie in one of the standard locations on your system, it is necessary to specify their location via compiler and linker flags in the environment variables `CFLAGS`, `NISTFLAGS`, and `LDFLAGS`.

For example, on macOS you can install OpenSSL via [Homebrew](https://brew.sh) by running
```sh
brew install openssl
```
Then, run
```sh
export CFLAGS="-I/usr/local/opt/openssl@1.1/include"
export NISTFLAGS="-I/usr/local/opt/openssl@1.1/include"
export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
```
before compilation, you need to add the OpenSSL header and library locations to the respective search paths.

### Test programs

To compile the test programs on Linux or macOS, choose the concerned implementation (Reference and Optimized) and go to the `EagleSign$NSL/` where `$NSL` ranges over the parameter sets 3 and 5, and run
```sh
make
```
This produces the executables
```sh
test/test_eaglesign$NSL
test/test_vectors$NSL
PQgenKAT_sign$NSL
```
where `$NSL` ranges over the parameter sets 3 and 5.

* `test_eaglesign$NSL` tests 1000 times to generate keys, sign a random message of 100 bytes and verify the produced signature. Also, the program will try to verify wrong signatures where a single random byte of a valid signature was randomly distorted. The program will abort with an error message and return -1 if there was an error. Otherwise it will output the key and signature sizes and return 0.
* `test_vectors$NSL` performs further tests of internal functions and prints generated test vectors for several intermediate values that occur in the EagleSign algorithms.
In order to generate the executables `test_eaglesign$NSL`'s and `test_vectors$NSL`'s, run the command 
```sh
make tests
```

* `PQgenKAT_sign$NSL` are the Known Answer Test (KAT) generation programs provided by NIST. It computes the official KATs and writes them to the files `PQsignKAT_$(CRYPTO_ALGNAME).{req,rsp}` where `$CRYPTO_ALGNAME` corresponds to the Nist Security Level based implementations of our Signature referenced above by `EagleSign$NSL`.
In order to generate the executables `PQgenKAT_sign$NSL`'s, run the command 
```sh
make kats
```

### Benchmarking programs

For benchmarking the implementations, we provide speed test programs for x86 CPUs that use the Time Step Counter (TSC) or the actual cycle counter provided by the Performance Measurement Counters (PMC) to measure performance. To compile the programs run
```sh
make speed
```
This produces the executables
```sh
test/test_speed$NSL
```
For every set of parameters `$NSL` mentioned earlier, the programs provide the median and average cycle counts obtained from 10,000 executions of different internal functions and API functions related to key generation, signing, and verification.

It should be noted that the reference implementation found in `EagleSign$NSL/` is not tailored for any specific platform and, due to its emphasis on clean code, it operates noticeably slower compared to a minimally optimized yet platform-independent implementation. Therefore, conducting benchmarks on the reference code will not yield accurate results.

## Shared libraries

All implementations can be compiled into shared libraries by running
```sh
make shared
```
For example in the directory `EagleSign$NSL/` of the reference implementation, this produces the libraries
```sh
libpq_eaglesign$NSL.so
```
for all parameter sets `$NSL`, and the required symmetric crypto libraries
```
libpq_aes256ctr_ref.so
libpq_fips202_ref.so
```
All global symbols in the libraries lie in the namespaces `pq_eaglesign$NSL`, `libpq_aes256ctr_ref` and `libpq_fips202_ref`. Hence it is possible to link a program against all libraries simultaneously and obtain access to all implementations for all parameter sets. The corresponding API header file is `EagleSign$NSL/api.h`, which contains prototypes for all API functions and preprocessor defines for the key and signature lengths.
