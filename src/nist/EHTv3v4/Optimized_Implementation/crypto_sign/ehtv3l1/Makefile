CC = /usr/bin/gcc
CFLAGS = -g -O3 -std=c99
LDFLAGS = -static-libgcc -lssl -lcrypto -lm

SOURCES = sign.c eht_keygen.c eht_siggen.c eht_sigver.c keccak.c tables.c parameters.c rng.c general_functions.c general_functions_with_tables.c PQCgenKAT_sign.c
HEADERS = api.h eht_keygen.h eht_siggen.h eht_sigver.h keccak.h tables.h parameters.h rng.h general_functions.h general_functions_with_tables.h

PQCgenKAT_sign: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

.PHONY: clean

clean:
	-rm PQCgenKAT_sign