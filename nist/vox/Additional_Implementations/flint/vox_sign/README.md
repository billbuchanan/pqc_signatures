VOX signature scheme
====================

Demo implementation using FLINT. Also supports the FULL-VOX parameter sets

### Dependencies

OpenSSL, FLINT

OpenSSL is only used in the randombytes() function provided by NIST for KAT generation.

It may be necessary to do
```sh
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
```
to run executables since FLINT is dynamically linked.
(adjust the path if FLINT is installed in another directory).


### Usage

To compile and run the code, simply do

```sh
make clean
make PARAM=VOX256
./test/PQCgenKAT_sign
./test/vox_demo
```

To select a different parameter set, replace the argument `PARAM=VOX256` as appropriate.

### License

This implementation is available under the MIT license below,
but we use the following third-party code under a different license :
  - keccak (in `fips202/` folder), based on public domain implementations
    by Ronny Van Keer and by Gilles Van Assche, Daniel J. Bernstein, Peter Schwabe.
    * Public domain
  - DRBG (in `rng/` folder) and KAT program (file `test/PQCgenKAT_sign.c`)
    that were part of the NIST submission template
    * Copyright (C) 2017 Bassham, Lawrence E (Fed). All rights reserved.

---

Copyright 2023  The VOX team

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---
