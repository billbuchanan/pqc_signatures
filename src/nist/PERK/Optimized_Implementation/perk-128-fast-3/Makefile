CC = gcc
CFLAGS:= -std=c99 -pedantic -Wall -Wextra -O3 -funroll-all-loops -march=native -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wshadow -Wno-newline-eof \
                  -mavx2 -mpclmul -msse4.2 -maes
ASMFLAGS := -x assembler-with-cpp -Wa,-defsym,old_gas_syntax=1 -Wa,-defsym,no_plt=1
LDFLAGS:= -lcrypto
ADDITIONAL_CFLAGS:= -Wno-missing-prototypes -Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-parameter

# Directories
BUILD_DIR:=build
BIN_DIR:=$(BUILD_DIR)/bin
LIB_DIR:=lib
SRC_DIR:=src

# main files and executables
MAIN_PERK_SRC:=$(SRC_DIR)/main.c
MAIN_KAT_SRC:=$(SRC_DIR)/PQCgenKAT_sign.c
MAIN_PERK_EXEC:=$(BIN_DIR)/perk-128-fast-3$(EXT)
MAIN_KAT_EXEC:=$(BIN_DIR)/perk-128-fast-3-PQCgenKAT_sign

# exclude sources from "find"
EXCL_SRC:=! -name $(notdir $(MAIN_PERK_SRC)) \
          ! -name $(notdir $(MAIN_KAT_SRC))

# PERK sources
PERK_SRC:= $(shell find $(SRC_DIR) -name "*.c" $(EXCL_SRC))
# Lib sources
LIB_CSRC := $(shell find $(LIB_DIR) -name "*.c" ! -path  "lib/djbsort/*")
SORT_CSRC := $(shell find $(LIB_DIR)/djbsort -name "*.c")
LIB_SSRC := $(shell find $(LIB_DIR) -name "*.s")

# PERK objects
PERK_OBJS:=$(PERK_SRC:%.c=$(BUILD_DIR)/%$(EXT).o)
# Lib objects
LIB_COBJS:=$(LIB_CSRC:%.c=$(BUILD_DIR)/%.o)
SORT_COBJS:=$(SORT_CSRC:%.c=$(BUILD_DIR)/%.o)
LIB_SOBJS:=$(LIB_SSRC:%.s=$(BUILD_DIR)/%.o)
LIB_OBJS:=$(LIB_COBJS) $(LIB_SOBJS) $(SORT_COBJS)

# include directories
LIB_INCLUDE:=-I $(LIB_DIR)/cryptocode -I $(LIB_DIR)/XKCP -I $(LIB_DIR)/randombytes -I $(LIB_DIR)/djbsort
PERK_INCLUDE:=-I $(SRC_DIR) $(LIB_INCLUDE)

.PHONY: all
all: perk-128-fast-3 perk-128-fast-3-verbose perk-128-fast-3-kat

# build rules
$(LIB_COBJS): $(BUILD_DIR)/%.o: %.c
	@echo -e "\n### Compiling external library file $@\n"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ADDITIONAL_CFLAGS) -c $< $(LIB_INCLUDE) -o $@

$(SORT_COBJS): $(BUILD_DIR)/%.o: %.c
	@echo -e "\n### Compiling external library file $@\n"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -fwrapv $(ADDITIONAL_CFLAGS) -c $< $(LIB_INCLUDE) -o $@

$(LIB_SOBJS): $(BUILD_DIR)/%.o: %.s
	@echo -e "\n### Assembling external library file $@\n"
	@mkdir -p $(dir $@)
	$(CC) $(ASMFLAGS) -c $< -o $@

$(PERK_OBJS): $(BUILD_DIR)/%$(EXT).o: %.c
	@echo -e "\n### Compiling perk-128-fast-3 file $@\n"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< $(PERK_INCLUDE) -o $@

# main targets
$(MAIN_PERK_EXEC): $(MAIN_PERK_SRC) $(PERK_OBJS) $(LIB_OBJS)
	@echo -e "\n### Compiling PERK scheme\n"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(PERK_INCLUDE) -o $@ $(LDFLAGS)

$(MAIN_KAT_EXEC): $(MAIN_KAT_SRC)  $(PERK_OBJS) $(LIB_OBJS)
	@echo -e "\n### Compiling PERK PQCgenKAT_sign\n"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Wno-strict-prototypes -Wno-unused-result $^ $(PERK_INCLUDE) -o $@ $(LDFLAGS)

.PHONY: perk-128-fast-3
perk-128-fast-3: $(MAIN_PERK_EXEC)

.PHONY: perk-128-fast-3-verbose
perk-128-fast-3-verbose:
	$(MAKE) EXT='-verbose' CFLAGS="$(CFLAGS) -DVERBOSE" perk-128-fast-3

.PHONY: perk-128-fast-3-kat
perk-128-fast-3-kat: $(MAIN_KAT_EXEC)

clean:
	rm -f PQCsignKAT_*
	rm -f vgcore.*
	rm -rf $(BUILD_DIR)