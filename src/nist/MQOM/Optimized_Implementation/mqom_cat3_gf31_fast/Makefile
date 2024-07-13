CC?=gcc
ALL_FLAGS?=-O3 -flto -fPIC -std=c11 -march=native -Wall -Wextra -Wpedantic -Wshadow -DPARAM_HYPERCUBE_7R -DPARAM_GF31 -DPARAM_L3 -DPARAM_RND_EXPANSION_X4 -DHASHX4 -DXOFX4 -DPRGX4 -DNDEBUG -mavx
ALL_FLAGS+=$(EXTRA_ALL_FLAGS)

SYM_OBJ= rnd.o hash.o xof.o
ARITH_OBJ= gf31-matrix.o gf31.o
MPC_OBJ= mpc.o witness.o serialization-specific.o precomputed.o
CORE_OBJ= keygen.o sign.o views.o commit.o sign-mpcith-hypercube.o tree.o

HASH_PATH=sha3
HASH_MAKE_OPTIONS=PLATFORM=avx2

APP_BENCH_OBJ=benchmark/bench.o benchmark/timing.o
APP_WRITE_BENCH_OBJ=benchmark/bench-write.o
APP_READ_BENCH_OBJ=benchmark/bench-read.o
APP_KAT_MAIN_OBJ=generator/PQCgenKAT_sign.o generator/rng.o
APP_KAT_CHECK_OBJ=generator/PQCgenKAT_check.o generator/rng.o

HASH_INCLUDE=-Isha3 -I. -Isha3/avx2
APP_INCLUDE=-Ibenchmark

%.o : %.c
	$(CC) -c $(ALL_FLAGS) $(HASH_INCLUDE) $(APP_INCLUDE) -I. $< -o $@

all: kat_gen bench

libhash:
	$(HASH_MAKE_OPTIONS) make -C $(HASH_PATH)

# Generation of the KAT
sign: kat_gen
	cp kat_gen sign

kat_gen: $(APP_KAT_MAIN_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) libhash
	$(CC) $(APP_KAT_MAIN_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) $(ALL_FLAGS) -L$(HASH_PATH) -L. -lhash -lcrypto -o $@

kat_check: $(APP_KAT_CHECK_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) libhash
	$(CC) $(APP_KAT_CHECK_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) $(ALL_FLAGS) -L$(HASH_PATH) -L. -lhash -lcrypto -o $@

## Benchmark Tool

bench: $(APP_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) libhash
	$(CC) $(APP_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) $(ALL_FLAGS) -L$(HASH_PATH) -lhash -lm -L. -o $@

bench-write: $(APP_WRITE_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) libhash
	$(CC) $(APP_WRITE_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) $(ALL_FLAGS) -L$(HASH_PATH) -lhash -lm -L. -o $@

bench-read: $(APP_READ_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) libhash
	$(CC) $(APP_READ_BENCH_OBJ) $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ) $(ALL_FLAGS) -L$(HASH_PATH) -lhash -lm -L. -o $@

# Cleaning

clean:
	rm -f $(SYM_OBJ) $(ARITH_OBJ) $(MPC_OBJ) $(CORE_OBJ)
	rm -f $(APP_BENCH_OBJ) $(APP_RAW_BENCH_OBJ)  $(APP_WRITE_BENCH_OBJ) $(APP_READ_BENCH_OBJ) $(APP_KAT_MAIN_OBJ)
	rm -rf unit-*
	rm -f bench bench-*
	$(HASH_MAKE_OPTIONS) make -C $(HASH_PATH) clean
