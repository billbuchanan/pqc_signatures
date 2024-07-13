# HOWTO:
# 
# Run `make` or `make test` in each implementation folder to compile the `PQCgenKAT_sign` program which generates the KAT files.
# 
# Run `make benchmark` in each implementation folder to compile the `benchmark` program, which measures the CPU cycles of KeyGen, Sign, and Verify. 
# Note that the `benchmark` program only works on x64 platforms.

# Run sage code in Extra folder for four security evaluation experiments, parameter generation, with a generic implementation 

# This file
./README.txt

# IP Statement
./IP-Statements-signed.pdf

# Cover sheet
./Supporting_Documentation/cover_sheet.pdf

# Specification document
./Supporting_Documentation/eMLE-v1.pdf

# KAT files for eMLE-Sig-I (security category I)
./KAT/eMLE-Sig-I/PQCsignKAT_800.rsp
./KAT/eMLE-Sig-I/PQCsignKAT_800.req

# KAT files for eMLE-Sig-III (security category III)
./KAT/eMLE-Sig-III/PQCsignKAT_1200.rsp
./KAT/eMLE-Sig-III/PQCsignKAT_1200.req

# KAT files for eMLE-Sig-V (security category V)
./KAT/eMLE-Sig-V/PQCsignKAT_1600.rsp
./KAT/eMLE-Sig-V/PQCsignKAT_1600.req

# Reference implementation for eMLE-Sig-I
./Reference_Implementation/crypto_sign/eMLE-Sig-I/fips202.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/randvec.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/rng.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/conv.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/aes256ctr.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/PQCgenKAT_sign.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/cpucycles.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/littleendian.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/aes256ctr.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/fips202.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/rng.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/Makefile
./Reference_Implementation/crypto_sign/eMLE-Sig-I/api.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/LICENSE.md
./Reference_Implementation/crypto_sign/eMLE-Sig-I/randvec.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/impl.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/rng_benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/cpucycles.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/nist.c
./Reference_Implementation/crypto_sign/eMLE-Sig-I/impl.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/mod.h
./Reference_Implementation/crypto_sign/eMLE-Sig-I/conv.c

# Reference implementation for eMLE-Sig-III
./Reference_Implementation/crypto_sign/eMLE-Sig-III/fips202.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/randvec.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/rng.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/conv.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/aes256ctr.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/PQCgenKAT_sign.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/cpucycles.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/littleendian.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/aes256ctr.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/fips202.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/rng.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/Makefile
./Reference_Implementation/crypto_sign/eMLE-Sig-III/api.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/LICENSE.md
./Reference_Implementation/crypto_sign/eMLE-Sig-III/randvec.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/impl.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/rng_benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/cpucycles.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/nist.c
./Reference_Implementation/crypto_sign/eMLE-Sig-III/impl.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/mod.h
./Reference_Implementation/crypto_sign/eMLE-Sig-III/conv.c

# Reference implementation for eMLE-Sig-V
./Reference_Implementation/crypto_sign/eMLE-Sig-V/fips202.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/randvec.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/rng.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/conv.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/aes256ctr.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/PQCgenKAT_sign.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/cpucycles.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/littleendian.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/aes256ctr.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/fips202.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/rng.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/Makefile
./Reference_Implementation/crypto_sign/eMLE-Sig-V/api.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/LICENSE.md
./Reference_Implementation/crypto_sign/eMLE-Sig-V/randvec.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/impl.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/rng_benchmark.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/cpucycles.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/nist.c
./Reference_Implementation/crypto_sign/eMLE-Sig-V/impl.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/mod.h
./Reference_Implementation/crypto_sign/eMLE-Sig-V/conv.c

# Additional implementation with AES-NI for eMLE-Sig-I
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/fips202.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/randvec.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/rng.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/conv.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/aes256ctr.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/PQCgenKAT_sign.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/cpucycles.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/littleendian.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/aes256ctr.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/fips202.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/rng.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/Makefile
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/api.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/LICENSE.md
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/randvec.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/impl.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/rng_benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/cpucycles.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/nist.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/impl.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/mod.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-I/conv.c

# Additional implementation with AES-NI for eMLE-Sig-III
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/fips202.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/randvec.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/rng.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/conv.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/aes256ctr.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/PQCgenKAT_sign.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/cpucycles.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/littleendian.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/aes256ctr.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/fips202.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/rng.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/Makefile
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/api.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/LICENSE.md
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/randvec.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/impl.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/rng_benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/cpucycles.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/nist.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/impl.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/mod.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-III/conv.c

# Additional implementation with AES-NI for eMLE-Sig-V
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/fips202.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/randvec.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/rng.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/conv.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/aes256ctr.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/PQCgenKAT_sign.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/cpucycles.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/littleendian.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/aes256ctr.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/fips202.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/rng.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/Makefile
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/api.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/LICENSE.md
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/randvec.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/impl.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/rng_benchmark.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/cpucycles.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/nist.c
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/impl.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/mod.h
./Additional_Implementations/aesni/crypto_sign/eMLE-Sig-V/conv.c

# SageMath scripts for securty evaluation and parameter vc generation 
./Extra/sage-code/impl-gen.sage  # the generic implementation of eMLE-Sig 
./Extra/sage-code/correctness.sage # correctness test 
./Extra/sage-code/impl-vc.sage # revised implementaiton for generating parameter vc    
./Extra/sage-code/vc-gen.sage  # vc generator 
./Extra/sage-code/test-1.sage  # Effectiveness of the adapted attack method
./Extra/sage-code/test-2.sage  # Resilience to key recovery attack from public key
./Extra/sage-code/test-3.sage  # Resilience to key recovery attack via signatures 
./Extra/sage-code/test-4.sage  # Strong Unforgeability under Chosen Message Attacks


