CC = gcc

LDFLAGS =  -L/usr/local/lib
CFLAGS = -I/usr/local/include -Wunused-variable -Wunused-function
LIBFLAGS = -lcrypto -lssl -lm
CFILES := $(shell find src -name '*.c' | sed -e 's/\.c/\.o/')

OBJS = ${CFILES}

ifeq ($(DEBUG), 1)
    DBG_FLAGS = -g -O0 -DDEBUG
else
    DBG_FLAGS = -O2 -DNDEBUG -Wunused-variable -Wunused-function
endif


%.o : %.c
	$(CC) $(CFLAGS) $(DBG_FLAGS) -o $@ -c $< 
	
sign: ${OBJS} ./PQCgenKAT_sign.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(DBG_FLAGS) -o $@ $^ $(LIBFLAGS)

runtime: ${OBJS} ./runtime.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(DBG_FLAGS) -o $@ $^ $(LIBFLAGS)

test: ${OBJS} ./test.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(DBG_FLAGS) -o $@ $^ $(LIBFLAGS)


matrix.o : matrix.h
rng.o : rng.h
api.o : api.h

clean:
	cd src; rm -f *.o; cd ..
	rm -f *.o
	rm -f sign runtime
