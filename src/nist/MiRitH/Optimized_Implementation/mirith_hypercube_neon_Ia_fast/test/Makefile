CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -flax-vector-conversions
DEPS=$(wildcard ../*.h)
OBJ=$(patsubst ../%.c,../%.o,$(wildcard ../*.c))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))

all: test_mirith

%.o: %.s
	$(CC) -c $(CFLAGS) -o $@ $< 

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

