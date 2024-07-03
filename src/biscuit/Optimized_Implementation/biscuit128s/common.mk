SHA3_ASM = $(wildcard sha3/*.s) $(wildcard sha3/$(SHA3_TARGET)/*.s)
SHA3_SRC = $(wildcard sha3/*.c) $(wildcard sha3/$(SHA3_TARGET)/*.c)

CC = gcc
CFLAGS = -Wall -pedantic -Wextra -O3
CPPFLAGS = -Isha3/$(SHA3_TARGET) -DUINTX_BITSIZE=$(UINTX_BITSIZE)

HDR = biscuit.h utils.h batch_tools.h $(wildcard params*.h)
SRC = $(BISCUIT_FILE) utils.c batch_tools.c
OBJ = $(SRC:.c=.o) $(SHA3_SRC:.c=.o) $(SHA3_ASM:.s=.o)

API_OBJ = rng.o api.o

EXE = PQCgenKAT_sign test perf_api benchmark

all: $(EXE)

test: test.c $(OBJ)
benchmark: benchmark.c $(OBJ)

PQCgenKAT_sign: LDLIBS = -lcrypto
PQCgenKAT_sign: PQCgenKAT_sign.c $(API_OBJ) $(OBJ)

perf_api: LDLIBS = -lcrypto
perf_api: perf_api.c $(API_OBJ) $(OBJ)

clean:
	rm -f $(API_OBJ) $(OBJ) $(EXE)
