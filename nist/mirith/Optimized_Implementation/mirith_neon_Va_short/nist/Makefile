CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -flax-vector-conversions -I/opt/homebrew/Cellar/openssl@3/3.1.0/include/ -L/opt/homebrew/Cellar/openssl@3/3.1.0/lib
DEPS=$(wildcard ../*.h)
LIBS=-lcrypto
OBJ=rng.o $(subst ../random.o, ,$(patsubst ../%.c,../%.o,$(wildcard ../*.c)))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))

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

