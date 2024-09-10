# select parameter set (one of: VOX128 VOX192 VOX256)
PARAM ?= VOX256

CC=gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -O3 -funroll-loops -march=native -DPARAM_SET_$(PARAM)
LIBS = -lcrypto

BUILD_DIR = .build

# Sources
###########
HDR = $(wildcard *.h) fips202/fips202.h rng/rng.h
SRC = $(wildcard *.c) fips202/fips202.c rng/rng.c
BUILD_DIRS_ALL = $(BUILD_DIR) $(BUILD_DIR)/fips202 $(BUILD_DIR)/rng

SRC += fips202/fips202x4.c fips202/keccak4x/KeccakP-1600-times4-SIMD256.c
BUILD_DIRS_ALL += $(BUILD_DIR)/fips202/keccak4x

OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

# Executables
###############
APP = test/PQCgenKAT_sign test/vox_test

# Main targets
###############

all: $(APP)

KAT: test/PQCgenKAT_sign
	./test/PQCgenKAT_sign

bench: test/vox_test
	./test/vox_test

clean:
	-rm -rf $(BUILD_DIR) $(APP)

.PHONY: all KAT clean

# Compilation rules
####################

test/%: test/%.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -I. -Irng/ $(LIBS)

$(BUILD_DIR)/%.o:%.c $(HDR) | $(BUILD_DIRS_ALL)
	$(CC) -o $@ $< $(CFLAGS) -c

$(BUILD_DIRS_ALL): %:
	mkdir -p $@

.SECONDARY: $(OBJ)
