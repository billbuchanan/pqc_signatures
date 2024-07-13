SOURCES=rng.c sign.c serializer.c gen.c
HEADERS=rng.h serializer.h gen.h api.h

LDFLAGS=-lflint -lgmp -lcrypto
OPTIMIZED=-Ofast -march=native
DFLAGS=-DMULT_STRASSEN

PQCgenKAT_sign:
	gcc $@.c $(HEADERS) $(SOURCES) -o $@ $(OPTIMIZED) $(LDFLAGS) -std=c99

test:
	gcc $@.c $(HEADERS) $(SOURCES) -o $@ $(LDFLAGS) -std=c99

fast_test:
	gcc test.c $(HEADERS) $(SOURCES) -o $@ $(LDFLAGS) $(OPTIMIZED) -std=c99

.PHONY: clean
clean:
	rm -f PQCgenKAT_sign test fast_test strassen_fast_test *.o >/dev/null