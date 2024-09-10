CC=gcc
CFLAGS= -O3 -Wall -Wextra -march=native -fomit-frame-pointer

SOURCES= KeccakHash.c KeccakHashtimes4.c KeccakSponge.c KeccakSpongetimes4.c KeccakP-1600-opt64.c KeccakP-1600-times4-on1.c
OBJECTS= KeccakHash.o KeccakHashtimes4.o KeccakSponge.o KeccakSpongetimes4.o KeccakP-1600-opt64.o KeccakP-1600-times4-on1.o
SHAKELIB= libshake.a

all: $(SOURCES) $(SHAKELIB)

$(SHAKELIB): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -f *.o
	rm -f libshake.a
