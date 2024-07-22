CC=gcc
CFLAGS=-I. -O3 -Wall -march=native
LDFLAGS=-lcrypto

OBJS=api.o fq_arithmetic/mf3.o fq_arithmetic/vf2.o fq_arithmetic/vf3.o keygen.o prng/fips202.o sign.o util/bitstream.o util/compress.o util/djbsort_portable.o util/gauss.o util/hash.o util/mf3permut.o util/tritstream.o verify.o wave/decode.o wave/randperm.o wave/reject.o wave/sample.o

all: PQCgenKAT_sign verify_test benchmark

PQCgenKAT_sign: $(OBJS) prng/prng-nist.o NIST-kat/rng.o PQCgenKAT_sign.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

verify_test: $(OBJS) prng/prng.o verify_test.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchmark: $(OBJS) prng/prng.o benchmark.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchmark_speed: $(OBJS) prng/prng.o benchmark_speed.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchmark_time: $(OBJS) prng/prng.o benchmark_time.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

prng/prng-nist.o:
	$(CC) $(CFLAGS) -DNIST_KAT -c -o prng/prng-nist.o prng/prng.c

clean:
	-rm $(OBJS) prng/prng.o prng/prng-nist.o NIST-kat/rng.o

veryclean: clean
	-rm PQCgenKAT_sign verify_test benchmark benchmark_speed benchmark_time
