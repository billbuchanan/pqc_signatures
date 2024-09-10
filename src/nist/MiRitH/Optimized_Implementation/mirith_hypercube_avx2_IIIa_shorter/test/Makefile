CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -mavx2 -g
DEPS=$(wildcard ../*.h)
OBJ=$(patsubst ../%.c,../%.o,$(wildcard ../*.c))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    ASMFLAGS := ${CFLAGS}
endif
ifeq ($(UNAME_S),Darwin)
    ASMFLAGS := ${CFLAGS} -x assembler-with-cpp -Wa,-defsym,old_gas_syntax=1 -Wa,-defsym,no_plt=1
endif

all: test_mirith

%.o: %.s
	$(CC) -c $(ASMFLAGS) -o $@ $< 

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

test_mirith: test_mirith.o $(OBJ)
	$(CC) ${LIBDIR} -o $@ $^ $(CFLAGS) $(LIBS)
	
run: test_mirith
	./test_mirith

.PHONY: clean

clean:
	rm -f *.o *.su
	rm -f ../*.o ../*.su
	rm -f test_mirith

