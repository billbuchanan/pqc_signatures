CC ?= /usr/bin/cc
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wpointer-arith -Wreturn-local-addr -O3 -mtune=native -march=native
NISTFLAGS += -Wno-unused-result -O3
SOURCES = sign.c poly.c coeff.c utils.c encode.c
HEADERS = config.h params.h api.h sign.h poly.h coeff.h utils.h encode.h
KECCAK_SOURCES = $(SOURCES) fips202.c
KECCAK_HEADERS = $(HEADERS) fips202.h

.PHONY: all shared clean

all: PQCgenKAT_sign

test: \
  test/test_red \
  test/test_inverse \
  test/test_keygen \
  test/test_sign_stack \
  test/test_sign

shared: \
  fuleeca1_ref.so \
  fuleeca_fips202_ref.so \

fuleeca_fips202_ref.so: fips202.c fips202.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

fuleeca1_ref.so: $(SOURCES) $(HEADERS)
	$(CC) -shared -fPIC $(CFLAGS) -DLEESIGN_MODE=1 \
	  -o $@ $(SOURCES)

test/test_red: test/test_red.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -UDBENCH -o $@ $< randombytes.c $(KECCAK_SOURCES)

test/test_inverse: test/test_inverse.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -UDBENCH -o $@ $< randombytes.c $(KECCAK_SOURCES)

test/test_keygen: test/test_keygen.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -UDBENCH -o $@ $< randombytes.c $(KECCAK_SOURCES)

test/test_sign: test/test_sign.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -UDBENCH -o $@ $< randombytes.c $(KECCAK_SOURCES) -lcpucycles

test/test_sign_stack: test/test_sign_stack.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -UDBENCH -o $@ $< randombytes.c $(KECCAK_SOURCES)

PQCgenKAT_sign: PQCgenKAT_sign.c rng.c rng.h $(KECCAK_SOURCES) \
	$(KECCAK_HEADERS)
		$(CC) $(NISTFLAGS) -o $@ $< rng.c $(KECCAK_SOURCES) $(LDFLAGS) -lcrypto

clean:
	rm -f *~ test/*~ *.gcno *.gcda *.lcov *.o *.i *.s
	rm -f fuleeca1_ref.so
	rm -f fuleeca_fips202_ref.so
	rm -f PQCgenKAT_sign
	rm -f test/test_red
	rm -f test/test_inverse
	rm -f test/test_keygen
	rm -f test/test_sign
	rm -f test/test_sign_stack
