
# MiRitH (MinRank in the Head)

## License

This implementation of MiRitH is released under the Apache License (v2.0).
See `NOTICE.txt` and `LICENSE.txt`.

## Documentation

The documentation is contained in the folder `Supporting_Documentation`
and consists of `Coversheet.pdf`, `IP_Statements.pdf`, and
`MiRitH_Documentation.pdf`.

## Compile and Test

To compile and test the reference implementation of MiRitH, move to
the folder

`Reference_Implementation/mirith_[parameter set]/test`

and run the command `make all && ./test_mirith`.

To compile and test the AVX2 optimized implementation of MiRitH, move to
the folder

`Optimized_Implementation/mirith_avx2_[parameter set]/test`

and run the command `make all && ./test_mirith`.

To compile and test the NEON optimized implementation of MiRitH, move to
the folder

`Optimized_Implementation/mirith_neon_[parameter set]/test`

and run the command `make all && ./test_mirith`.

## Benchmark (without SUPERCOP)

To benchmark the AVX2 optimized implementation of MiRitH, move to the folder

`Optimized_Implementation/mirith_avx2_[parameter set]/bench`

and run the command `make all && ./bench_mirith`.

To benchmark the NEON optimized implementation of MiRitH, move to the folder

`Optimized_Implementation/mirith_neon_[parameter set]/bench`

and run the command `make all && ./bench_mirith`.

## Benchmark with SUPERCOP

To benchmark the AVX2 optimized implementation of MiRitH with SUPERCOP, copy
the folder

`Optimized_Implementation/mirith_avx2_[parameter set]`

into `[SUPERCOP folder]/crypto_sign`, and edit the file

`[SUPERCOP folder]/crypto_sign/mirith_avx2_[parameter set]/config.h`

by uncommenting the line

`/* #define MIRITH_SUPERCOP */`

Then in the SUPERCOP folder run the command

`./do-part crypto_sign mirith_avx2_[parameter set]`

To benchmark the NEON optimized implementation of MiRitH with SUPERCOP, copy
the folder

`Optimized_Implementation/mirith_neon_[parameter set]`

into `[SUPERCOP folder]/crypto_sign`, and edit the file

`[SUPERCOP folder]/crypto_sign/mirith_neon_[parameter set]/config.h`

by uncommenting the line

`/* #define MIRITH_SUPERCOP */`

Then in the SUPERCOP folder run the command

`./do-part crypto_sign mirith_neon_[parameter set]`

