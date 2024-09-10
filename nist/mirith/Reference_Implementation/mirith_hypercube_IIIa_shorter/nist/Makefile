CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic 
DEPS=$(wildcard ../*.h)
LIBS=-lcrypto
OBJ=rng.o $(subst ../random.o, ,$(patsubst ../%.c,../%.o,$(wildcard ../*.c)))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS := ${CFLAGS} -lcrypto
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS := ${CFLAGS} -I/usr/local/opt/openssl/include
	LIBS :=-L/usr/local/opt/openssl/lib -lcrypto
endif

all: PQCgenKAT_sign

%.o: %.s
	$(CC) -c $(CFLAGS) -o $@ $< 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rng.o: rng.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
PQCgenKAT_sign: PQCgenKAT_sign.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
		
run: PQCgenKAT_sign
	./PQCgenKAT_sign

.PHONY: clean

clean:
	rm -f *.o *.su
	rm -f ../*.o ../*.su
	rm -f PQCgenKAT_sign
	rm -f PQCsignKAT_*.req
	rm -f PQCsignKAT_*.rsp

