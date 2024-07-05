CC=gcc
CFLAGS= -O3 -Wall -Wextra -march=native -fomit-frame-pointer -mavx2

SOURCES= KeccakHash.c KeccakHashtimes4.c KeccakSponge.c KeccakSpongetimes4.c KeccakP-1600-AVX2.s KeccakP-1600-times4-SIMD256.c
OBJECTS= KeccakHash.o KeccakHashtimes4.o KeccakSponge.o KeccakSpongetimes4.o KeccakP-1600-AVX2.o KeccakP-1600-times4-SIMD256.o
SHAKELIB= libshake.a

all: $(SOURCES) $(SHAKELIB)

$(SHAKELIB): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -f *.o
	rm -f libshake.a
