Squirrels
=========

This archive contains supporting files for the NIST submission
of Squirrels:

Reference_Implementation/
    squirrels-I/
        Reference implementation of Squirrels with recommended
        parameters for NIST level I.

    squirrels-II/
        Reference implementation of Squirrels with recommended
        parameters for NIST level II.

    squirrels-III/
        Reference implementation of Squirrels with recommended
        parameters for NIST level III.

    squirrels-IV/
        Reference implementation of Squirrels with recommended
        parameters for NIST level IV.

    squirrels-V/
        Reference implementation of Squirrels with recommended
        parameters for NIST level V.

Optimized_Implementation/
    squirrels-I/
        Optimized implementation of Squirrels with recommended
        parameters for NIST level I.

    squirrels-II/
        Optimized implementation of Squirrels with recommended
        parameters for NIST level II.

    squirrels-III/
        Optimized implementation of Squirrels with recommended
        parameters for NIST level III.

    squirrels-IV/
        Optimized implementation of Squirrels with recommended
        parameters for NIST level IV.

    squirrels-V/
        Optimized implementation of Squirrels with recommended
        parameters for NIST level V.

KAT/
    generator/
        NIST-provided code to generate KAT files.

lib/
    Source code of libraries required by Squirrels.

Supporting_Documentation/
    additional/
        params.py
            Script reproducing the selection of the parameters of Squirrels for
            each security level.

    squirrels.pdf
        Specification of Squirrels and supporting documentation.

    coversheet.pdf
        Statement about submitters and contact information.

Notes
=====

Each implementation under Reference_Implementation and  Optimized_Implementation 
has its own Makefile. When used, it will create an executable in build/katint
that produces the .req and .rsp files, which should be identical to the ones
provided in the KAT archive. It will also create an executable under 
build/benchmarks that will run some benchmarks.

How to build
============

Each version of Squirrels' comes with a Makefile that compiles it for the
current machine. It creates a file `build/katint` used to generate KATs.
Dependencies are automatically built in the `lib` folder and linked 
dynamically with executables.

Several usual build tools are required. On Ubuntu they can be installed with 
the following command:
```
sudo apt install make gcc g++ m4 automake
```

The code of dependencies is stored in `lib`. They can also be built independently by running `make -C lib`.

License
=======

This Squirrels implementation is provided under the MIT license, whose text
is included at the start of every source file.
