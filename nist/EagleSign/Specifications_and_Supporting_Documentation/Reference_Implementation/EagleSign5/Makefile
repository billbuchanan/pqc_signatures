CC ?= /usr/bin/cc
CFLAGS += 
NISTFLAGS += 
SOURCES = sign.c packing.c polyvec.c poly.c ntt.c reduce.c polymatrix.c
HEADERS = config.h params.h api.h sign.h packing.h polymatrix.h polyvec.h poly.h ntt.h \
  reduce.h symmetric.h randombytes.h
KECCAK_SOURCES = $(SOURCES) fips202.c symmetric-shake.c
KECCAK_HEADERS = $(HEADERS) fips202.h

.PHONY: all speed shared clean

all: \
  test/test_eaglesign3 \
  test/test_eaglesign5 \
  PQgenKAT_sign3 \
  PQgenKAT_sign5 \
  test/test_speed3 \
  test/test_speed5 \
  libpq_eaglesign3_ref.so \
  libpq_eaglesign5_ref.so \
  libpq_fips202_ref.so \
  libpq_aes256ctr_ref.so \

kats: \
  PQgenKAT_sign3 \
  PQgenKAT_sign5 \

tests: \
  test/test_eaglesign3 \
  test/test_eaglesign5 \
  test/test_vectors3 \
  test/test_vectors5 \

speed: \
  test/test_speed3 \
  test/test_speed5 \

shared: \
  libpq_eaglesign3_ref.so \
  libpq_eaglesign5_ref.so \
  libpq_fips202_ref.so \
  libpq_aes256ctr_ref.so \

libpq_fips202_ref.so: fips202.c fips202.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

libpq_aes256ctr_ref.so: aes256ctr.c aes256ctr.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

libpq_eaglesign3_ref.so: $(SOURCES) $(HEADERS) symmetric-shake.c
	$(CC) -shared -fPIC $(CFLAGS) -DEAGLESIGN_MODE=3 \
	  -o $@ $(SOURCES) symmetric-shake.c

libpq_eaglesign5_ref.so: $(SOURCES) $(HEADERS) symmetric-shake.c
	$(CC) -shared -fPIC $(CFLAGS) -DEAGLESIGN_MODE=5 \
	  -o $@ $(SOURCES) symmetric-shake.c

test/test_eaglesign3: test/test_eaglesign.c randombytes.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=3 \
	  -o $@ $< randombytes.c $(KECCAK_SOURCES)

test/test_eaglesign5: test/test_eaglesign.c randombytes.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=5 \
	  -o $@ $< randombytes.c $(KECCAK_SOURCES)

test/test_vectors3: test/test_vectors.c $(KECCAK_SOURCES) $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=3 \
	  -o $@ $< $(KECCAK_SOURCES)

test/test_vectors5: test/test_vectors.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=5 \
	  -o $@ $< $(KECCAK_SOURCES)

test/test_speed3: test/test_speed.c test/speed_print.c test/speed_print.h \
  test/cpucycles.c test/cpucycles.h randombytes.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=3 \
	  -o $@ $< test/speed_print.c test/cpucycles.c randombytes.c \
	  $(KECCAK_SOURCES)

test/test_speed5: test/test_speed.c test/speed_print.c test/speed_print.h \
  test/cpucycles.c test/cpucycles.h randombytes.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(CFLAGS) -DEAGLESIGN_MODE=5 \
	  -o $@ $< test/speed_print.c test/cpucycles.c randombytes.c \
	  $(KECCAK_SOURCES)

PQgenKAT_sign3: PQgenKAT_sign.c rng.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(NISTFLAGS) -DEAGLESIGN_MODE=3 \
	  -o $@ $< rng.c $(KECCAK_SOURCES) $(LDFLAGS) -lcrypto

PQgenKAT_sign5: PQgenKAT_sign.c rng.c $(KECCAK_SOURCES) \
  $(KECCAK_HEADERS)
	$(CC) $(NISTFLAGS) -DEAGLESIGN_MODE=5 \
	  -o $@ $< rng.c $(KECCAK_SOURCES) $(LDFLAGS) -lcrypto

clean:
	rm -f *~ test/*~ *.gcno *.gcda *.lcov
	rm -f libpq_eaglesign3_ref.so
	rm -f libpq_eaglesign5_ref.so
	rm -f libpq_fips202_ref.so
	rm -f libpq_aes256ctr_ref.so
	rm -f test/test_eaglesign3
	rm -f test/test_eaglesign5
	rm -f test/test_vectors3
	rm -f test/test_vectors5
	rm -f test/test_speed3
	rm -f test/test_speed5
	rm -f PQgenKAT_sign3
	rm -f PQgenKAT_sign5
