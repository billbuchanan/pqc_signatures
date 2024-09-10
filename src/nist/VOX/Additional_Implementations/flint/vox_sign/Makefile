# select parameter set (one of: VOX{128,192,256} FULLVOX{128,192,256})
PARAM ?= VOX256

# verbosity level
VERBOSE?=0

# FLINT installation directory
FLINT_DIR ?= /usr/local

CC=gcc
CFLAGS = -std=c99 -pedantic -Wall -O2 -funroll-loops -g -mpopcnt -DVERBOSE=$(VERBOSE) -DPARAM_SET_$(PARAM) -DVOX_NAME='"$(PARAM)"'

HDR = api.h fips202.h rng.h vox.h vox_params.h
SRC = api.c fips202.c rng.c vox_q2.c vox_sample.c vox_encode.c F4F5.c
APP = test/vox_demo test/PQCgenKAT_sign

BUILD_DIR = .build/
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

INCS=-I$(CURDIR) -I$(CURDIR)/build -I$(FLINT_DIR)/include -I$(FLINT_DIR)/include
LIBS=-L$(CURDIR) -L$(FLINT_DIR)/lib -L$(FLINT_DIR)/lib -lflint -lcrypto


# Main targets
all: $(APP)

clean:
	-rm -rf .build/ $(APP) tables/gen_embed_table

KAT: test/PQCgenKAT_sign

bench: test/bench

.PHONY: all KAT clean

# Compilation rules
test/%: test/%.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(INCS) $(LIBS)

tables/gen_embed_table: tables/gen_embed_table.c $(HDR)
	$(CC) -o $@ $< $(CFLAGS) $(INCS) $(LIBS)

$(BUILD_DIR)/%.o:%.c $(HDR) | $(BUILD_DIR)
	$(CC) -o $@ $< $(CFLAGS) $(INCS) -c

$(BUILD_DIR):
	mkdir -p $@

.SECONDARY: $(OBJ)
