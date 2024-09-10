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

SOURCES= aes/aes.c aes/aes_ni.c random/random.c sha3/fips202.c sha3/fips202x4.c sha3/shake.c sign.c rANS/compress.c normaldist/fixpoint.c gen_mat.c normaldist/normaldist.c sampling/samplez.c sampling/sampling.c util.c cpucycles.c rng.c PQCgenKAT_sign.c 
        
HEADERS= aes/aes.h aes/aes_local.h random/random.h sha3/fips202.h sha3/fips202x4.h sha3/shake.h api.h cbd.h cholesky.h compress.h fixpoint.h fpr.h funcs.h hash.h normaldist.h pack.h params.h print.h samplez.h sampling.h cpucycles.h rng.h

PQCgenKAT_sign: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	-rm PQCgenKAT_sign *.o
