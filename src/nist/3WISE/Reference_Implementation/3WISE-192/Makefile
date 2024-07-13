SOURCES=rng.c sign.c serializer.c gen.c
HEADERS=rng.h serializer.h gen.h api.h
INCLUDES=-I$(CURDIR)/../flint/include/
LDFLAGS=-L$(CURDIR)/../flint/lib -Wl,-rpath=$(CURDIR)/../flint/lib -lflint -lgmp -lcrypto
OPTIMIZED=-Ofast -march=native
DFLAGS=-DMULT_STRASSEN

all: PQCgenKAT_sign test fast_test pqcsign

pqcsign:
	gcc pqcsign.c $(SOURCES) -o $@ $(OPTIMIZED) $(INCLUDES) $(LDFLAGS) -std=c99

pqcsign.o: pqcsign.o
	$(CC) $(CFLAGS) pqcsign.o -o $@ $(LDFLAGS)

PQCgenKAT_sign:
	gcc PQCgenKAT_sign.c $(SOURCES) -o $@ $(OPTIMIZED) $(INCLUDES) $(LDFLAGS) -std=c99

test:
	gcc test.c $(SOURCES) -o $@ $(INCLUDES) $(LDFLAGS) -std=c99

fast_test:
	gcc test.c $(SOURCES) -o $@ $(OPTIMIZED) $(INCLUDES) $(LDFLAGS) -std=c99

.PHONY: clean
clean:
	rm -f PQCgenKAT_sign test fast_test *.o >/dev/null