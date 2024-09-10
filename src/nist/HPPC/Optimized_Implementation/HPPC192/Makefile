SOURCES=rng.c sign.c serializer.c gen.c
HEADERS=rng.h serializer.h gen.h ntl_utils.h api.h

LDFLAGS=-lm4ri -lflint -lgmp -lm -lstdc++ -lntl -ldl -lcrypto -pthread -lpthread -lgf2x
THREADED=-pthread -lpthread
OPTIMIZED=-Ofast -march=native
DFLAGS=-DMULT_STRASSEN

PQCgenKAT_sign:
	g++ -c ntl_utils.cpp $(THREADED) 
	gcc $@.c $(HEADERS) $(SOURCES) ntl_utils.o -o $@ $(LDFLAGS) $(OPTIMIZED) $(THREADED) -std=c99

test:
	g++ -c ntl_utils.cpp $(THREADED)
	gcc $@.c $(HEADERS) $(SOURCES) ntl_utils.o -o $@ $(LDFLAGS) $(THREADED) -std=c99

fast_test:
	g++ -c ntl_utils.cpp $(THREADED)
	gcc test.c $(HEADERS) $(SOURCES) ntl_utils.o -o $@ $(LDFLAGS) $(OPTIMIZED) $(THREADED) -std=c99

strassen_fast_test:
	g++ -c ntl_utils.cpp $(THREADED)
	gcc test.c $(HEADERS) $(SOURCES) ntl_utils.o -o $@ $(LDFLAGS) $(OPTIMIZED) $(DFLAGS) $(THREADED) -std=c99

.PHONY: clean
clean:
	rm -f PQCgenKAT_sign test fast_test strassen_fast_test *.o >/dev/null
