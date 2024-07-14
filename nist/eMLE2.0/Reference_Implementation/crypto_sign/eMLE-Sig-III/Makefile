CFLAGS = -O3 -Wall -Wextra -Wpedantic 
LDFLAGS = -fPIC -lssl -lcrypto
OBJS = PQCgenKAT_sign.o impl.o fips202.o conv.o aes256ctr.o randvec.o rng.o nist.o 
BENCHMARK_OBJS = impl.o fips202.o conv.o aes256ctr.o randvec.o rng_benchmark.o benchmark.o cpucycles.o

PQCgenKAT_sign: $(OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

benchmark: $(BENCHMARK_OBJS)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.c
	gcc $(CFLAGS) -o $@ -c $< 

impl.o: impl.h fips202.h conv.h aes256ctr.h randvec.h mod.h impl.c
fips202.o: fips202.h fips202.c
conv.o: conv.h mod.h conv.c
aes256ctr.o: aes256ctr.h aes256ctr.c
randvec.o: randvec.h aes256ctr.h mod.h littleendian.h randvec.c
rng.o: rng.h rng.c
nist.o: api.h impl.h rng.h littleendian.h nist.c
PQCgenKAT_sign.o: api.h rng.h PQCgenKAT_sign.c
rng_benchmark.o: rng.h rng_benchmark.c
benchmark.o: impl.h rng.h cpucycles.h benchmark.c
cpucycles.o: cpucycles.h cpucycles.c

test: PQCgenKAT_sign
all: PQCgenKAT_sign

.PHONY: clean
clean:
	rm -f PQCgenKAT_sign benchmark *.o
