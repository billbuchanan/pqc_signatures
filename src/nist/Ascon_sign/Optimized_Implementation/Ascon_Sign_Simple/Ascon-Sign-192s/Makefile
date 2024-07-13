THASH = simple

CC=/usr/bin/gcc
CFLAGS=-Wall -Wextra -Wpedantic -O3 -std=c99 -Wconversion -Wmissing-prototypes -DPARAMS=$(PARAMS) $(EXTRA_CFLAGS)

SOURCES =  address.c randombytes.c merkle.c wots.c wotsx1.c utils.c utilsx1.c fors.c sign.c

SOURCES += hash_ascon.c ascon_opt64/ascon.c ascon_opt64/permutations.c  thash_ascon_$(THASH).c

DET_SOURCES = $(SOURCES:randombytes.%=rng.%)

TESTS = test/fors \
		test/ascon_sign \

BENCHMARK = test/benchmark

.PHONY: clean test benchmark

default: PQCgenKAT_sign

all: PQCgenKAT_sign tests benchmarks

tests: $(TESTS)

test: $(TESTS:=.exec)

benchmarks: $(BENCHMARK)

benchmark: $(BENCHMARK:=.exec)

PQCgenKAT_sign: PQCgenKAT_sign.c $(DET_SOURCES)
	$(CC) $(CFLAGS) -o $@ $(DET_SOURCES) $< -lcrypto

test/benchmark: test/benchmark.c test/cycles.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ test/cycles.c $(SOURCES) $< $(LDLIBS)

test/%: test/%.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $< $(LDLIBS)

test/%.exec: test/%
	@$<

clean:
	-$(RM) -v $(TESTS)
	-$(RM) -v $(BENCHMARK)
	-$(RM) -v PQCgenKAT_sign
	-$(RM) -v PQCsignKAT_*.rsp
	-$(RM) -v PQCsignKAT_*.req
