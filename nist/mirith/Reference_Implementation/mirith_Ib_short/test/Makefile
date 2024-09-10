CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic
DEPS=$(wildcard ../*.h)
OBJ=$(patsubst ../%.c,../%.o,$(wildcard ../*.c))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS := ${CFLAGS} -lcrypto
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS := ${CFLAGS} -I/usr/local/opt/openssl/include
	LIBS :=-L/usr/local/opt/openssl/lib -lcrypto
endif

all: test_mirith

%.o: %.s
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

test_mirith: test_mirith.o $(OBJ)
	$(CC) ${LIBS} -o $@ $^ $(CFLAGS)

run: test_mirith
	./test_mirith

.PHONY: clean

clean:
	rm -f *.o *.su
	rm -f ../*.o ../*.su
	rm -f test_mirith
