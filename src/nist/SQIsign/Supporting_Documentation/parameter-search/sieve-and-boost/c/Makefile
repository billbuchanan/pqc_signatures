# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.
OPT=-O3     # Optimization option by default

CC=clang
ifeq "$(CC)" "gcc"
    COMPILER=gcc
else ifeq "$(CC)" "clang"
    COMPILER=clang
endif

ifeq "$(SET)" "EXTENDED"
    ADDITIONAL_SETTINGS=-fwrapv -fomit-frame-pointer -march=native
endif

CFLAGS=$(OPT) --std=c11 $(ADDITIONAL_SETTINGS) -D __LINUX__ -fPIC

OBJECTS=objs/test_sieve.o objs/test_extras.o
OBJECTS128=objs/test_sieve_128.o objs/test_extras.o

all: libsieve.so libsieve128.so tests tests128
c64: libsieve.so tests
c128: libsieve128.so tests128

objs/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@
	
libsieve.so: $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $@ $^

libsieve128.so: $(OBJECTS128)
	$(CC) $(CFLAGS) -shared -o $@ $^

tests: test_sieve.c test_extras.c
	$(CC) $(CFLAGS) test_sieve.c test_extras.c -o test_sieve -lm

tests128: test_sieve_128.c test_extras.c
	$(CC) $(CFLAGS) test_sieve_128.c test_extras.c -o test_sieve_128 -lm

.PHONY: clean

clean:
	rm -rf *.o *.so *.a test_sieve test_sieve_128 objs

