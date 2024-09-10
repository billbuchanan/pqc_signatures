HAWK SIGNATURE SCHEME
---------------------

Hawk has three defined parameter sets, Hawk-256, Hawk-512 and Hawk-1024.
  - Hawk-256 is the "challenge" variant, meant for research purposes; its
    security level is estimated around 2^64.
  - Hawk-512 provides the NIST level I security level (similar to AES-128).
  - Hawk-1024 provides the NIST level V security level (similar to AES-256).

Archive contents:

  - Algorithm description and specification:
        hawk.pdf

  - Reference implementation, for the three defined parameter sets:
        Reference_Implementation/hawk256/
        Reference_Implementation/hawk512/
        Reference_Implementation/hawk1024/

  - Optimized implementation, for x86 systems with AVX2 support:
        Optimized_Implementation/avx2/hawk256/
        Optimized_Implementation/avx2/hawk512/
        Optimized_Implementation/avx2/hawk1024/

  - Test vectors (as produced by each implementation; file name is derived
    from the private key length, in bytes):
        For Hawk-256:
            KAT/PQCsignKAT_96.req
            KAT/PQCsignKAT_96.rsp

        For Hawk-512:
            KAT/PQCsignKAT_184.req
            KAT/PQCsignKAT_184.rsp

        For Hawk-1024:
            KAT/PQCsignKAT_360.req
            KAT/PQCsignKAT_360.rsp

  - Printouts of detailed intermediate values, to help with development and
    testing of other implementations:
        KAT/IntermediateValues256.txt
        KAT/IntermediateValues512.txt
        KAT/IntermediateValues1024.txt


IMPLEMENTATION NOTES
--------------------

Each implementation follows the API requested by NIST and documented in
the api.h file, with three functions:

    crypto_sign_keypair(): generates a new key pair.

    crypto_sign(): signs a message, producing an encapsulated object that
    includes both the original message and the signature.

    crypto_sign_open(): receives an encapsulated object as produced by
    crypto_sign(), extracts the original message, and verifies the signature.

These API functions are implemented by api.c, which itself works over an
internal API described by hawk.h, that supports stream processing, use
of explicit temporary buffers (if working on a small-stack system), and
other options.

PQCgenKAT_sign.c is a command-line program that generates test vectors.
It uses all the other files. It is derived from an example file provided
by NIST.

rng.c and rng.h implement an AES-based DRBG, which is used by
PQCgenKAT_sign.c to make reproducible test vectors. They are derived
from example files provided by NIST, except that a custom AES
implementation has been included, to avoid a dependency on the OpenSSL
library.

api.c and api.h implement the API requested by NIST. api.c calls an
externally provided randombytes() function, which is meant to generate
uniformly random byte values:

    void randombytes(unsigned char *x, unsigned long long xlen);

In order to use the API provided by api.c and api.h, such a function
must be made available by the caller. The rng.c file provides a
randombytes() function, but does so with an AES-based DRBG that itself
requires initialization with a properly random seed. Moreover, the used
AES is a perfunctory table-based implementation, which is conceptually
vulnerable to some side-channel attacks (timing attacks leveraging the
interaction of table accesses with caches); thus, that implementation of
randombytes() should be used only for testing purposes. In a practical
application, an effective randombytes() function may be obtained by
wrapping around an OS-provided random source, e.g. the getrandom()
system function.

Alternatively, an application using Hawk might work with the internal
API only (hawk.h). In that API, a pseudorandom source is provided by the
caller as a SHAKE state (in output mode); it is up to the caller to
initialize that state with enough randomness. The internal API does NOT
depend on any of the following files: PQCgenKAT_sign.c, api.c, api.h,
rng.c, rng.h (these files may therefore be omitted when integrating the
Hawk implementation code into the calling application).

The Makefile uses the 'c99' C compiler, which should be available on any
Unix-like system (such as Linux), with that compiler being an alias for
GCC or Clang. The optimized x86 implementations with AVX2 should compile
on any x86 system, but will run only on systems that indeed have AVX2
support in the CPU. For compilation on Windows systems with MSVC, an
alternate file (Makefile.win32) is provided, to be used with:
    nmake /f Makefile.win32
(from a Visual C command prompt)
