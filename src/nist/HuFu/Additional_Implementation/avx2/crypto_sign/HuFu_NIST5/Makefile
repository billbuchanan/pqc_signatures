CC=clang
ifeq "$(CC)" "gcc"
    COMPILER=gcc
else ifeq "$(CC)" "clang"
    COMPILER=clang
endif

ARCHITECTURE=_AMD64_

CC=/usr/bin/gcc
CFLAGS= -mavx2 -mbmi2 -mpopcnt -O3 -std=gnu11 -march=native -Wextra -DNIX -mfpmath=sse -msse2 -ffp-contract=off
LDFLAGS= -lm -lssl -lcrypto

SOURCES= aes/aes.c aes/aes_ni.c random/random.c sha3/fips202.c sha3/fips202x4.c sha3/shake.c sign.c rANS/compress.c normaldist/fixpoint.c gen_mat.c normaldist/normaldist.c sampling/samplez.c sampling/sampling.c util.c pack.c cpucycles.c rng.c PQCgenKAT_sign.c
        
PQCgenKAT_sign: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	-rm PQCgenKAT_sign *.o
