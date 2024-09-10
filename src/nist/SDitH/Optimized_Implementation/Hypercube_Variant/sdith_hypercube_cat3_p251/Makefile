# This Makefile builds sdith_hypercube_<SEC_LEVEL>_<FIELD>_<VARIANT>.
# SEC_LEVEL: CAT_1|CAT_3|CAT_5
# FIELD: GF256|P251
# VARIANT: ref|avx2

.POSIX:

VARIANT = avx2
FIELD = P251
SEC_LEVEL = CAT_3

HASH_PATH=sha3
HASH_INCLUDE=-Isha3/avx2

ifeq ($(VARIANT), avx2)
	AVXFLAGS = -DAVX2 -mavx2 -mpclmul -mgfni -mavx -maes
	HASH_MAKE_OPTIONS = PLATFORM=avx2
else
	AVXFLAGS =
	HASH_MAKE_OPTIONS = PLATFORM=opt64
endif
CC = gcc
CFLAGS = -W -Wall -O3 -fPIC -DNDEBUG -D${SEC_LEVEL} -std=c11

ifeq ($(VARIANT), avx2)
	GF256_SRC = gf256.c gf2p32.c gf256-avx2-polytable-ct.c gf256-avx2.c gf256-avx2-gfni.c gf256-avx-pclmul.c
	P251_SRC = p251.c p251p4.c p251-avx2-ct.c
else
	GF256_SRC = gf2p32.c gf256.c
	P251_SRC = p251.c p251p4.c
endif

ifeq ($(FIELD), GF256)
	FIELD_SRC = ${GF256_SRC}
else
	FIELD_SRC = ${P251_SRC}
endif

CRYPTO_SRC = hash-sha3.c rng.c treeprg.c
SDITH_SRC = sdith.c precomputed.c sign.c
KAT_MAIN_SRC = generator/PQCgenKAT_sign.c generator/rng.c
BENCH_MAIN_SRC = benchmark/bench.c benchmark/timing.c

all: sign bench

libhash:
	$(HASH_MAKE_OPTIONS) make -C $(HASH_PATH)

sign: ${SRC} ${FIELD_SRC} ${CRYPTO_SRC} ${SDITH_SRC} ${KAT_MAIN_SRC} libhash
	-${CC} ${CFLAGS} ${AVXFLAGS} -o $@ ${SRC} ${FIELD_SRC} ${CRYPTO_SRC} ${SDITH_SRC} ${KAT_MAIN_SRC} -I. ${HASH_INCLUDE} -L${HASH_PATH} -lcrypto -lhash

bench: ${SRC} ${FIELD_SRC} ${CRYPTO_SRC} ${SDITH_SRC} ${BENCH_MAIN_SRC} libhash
	-${CC} ${CFLAGS} ${AVXFLAGS} -DBENCHMARK_CYCLES -o $@ ${SRC} ${FIELD_SRC} ${CRYPTO_SRC} ${SDITH_SRC} ${BENCH_MAIN_SRC} -I. ${HASH_INCLUDE} -L${HASH_PATH} -lcrypto -lhash -lm

clean:
	-rm -f *.o *.a sign bench
	$(HASH_MAKE_OPTIONS) make -C $(HASH_PATH) clean