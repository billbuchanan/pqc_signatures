
SCRIPT_VERSION=v1.0
SCRIPT_AUTHOR=MIRA team

CC=gcc
C_FLAGS:=-O3 -flto -mavx2 -mpclmul -msse4.2 -maes -std=c99 -pedantic -Wall -Wextra -DSHAKE_TIMES4
C_FLAGS_VERBOSE:=-O3 -flto -mavx2 -mpclmul -msse4.2 -maes -std=c99 -pedantic -Wall -Wextra -DSHAKE_TIMES4 -DVERBOSE

RANDOMBYTES_SRC:=lib/randombytes/randombytes.c
RANDOMBYTES_INCLUDE:=-I lib/randombytes -lcrypto

XKCP_SRC:=lib/XKCP
XKCP_SRC_SIMPLE:=$(XKCP_SRC)/SimpleFIPS202.c
XKCP_INCLUDE:=-I$(XKCP_SRC) -I$(XKCP_SRC)/avx2
XKCP_INCLUDE_SIMPLE:=-I $(XKCP_SRC)
XKCP_LINKER:=-L$(XKCP_SRC) -lshake

WRAPPER_SRC:=src/wrapper
WRAPPER_INCLUDE:=-I $(WRAPPER_SRC)

FFI_SRC:=src/finite_fields
FFI_INCLUDE:=-I $(FFI_SRC)

SRC:=src
INCLUDE:=-I src $(FFI_INCLUDE) $(WRAPPER_INCLUDE) $(XKCP_INCLUDE) $(RANDOMBYTES_INCLUDE)

MAIN_MIRA:=src/main_mira.c
MAIN_KAT:=src/PQCgenKAT_sign.c

MIRA_OBJS:=finite_fields.o keygen.o sign.o verify.o nist_sign.o mpc.o parsing.o tree.o
MIRA_VERBOSE_OBJS:=finite_fields.o keygen-verbose.o sign-verbose.o verify-verbose.o nist_sign.o mpc.o parsing.o tree.o
LIB_OBJS:=SimpleFIPS202.o randombytes.o

BUILD:=bin/build
BIN:=bin



folders:
	@echo -e "\n### Creating build folders\n"
	mkdir -p $(BUILD)

randombytes.o: folders
	@echo -e "\n### Compiling $@\n"
	$(CC) $(C_FLAGS) -c $(RANDOMBYTES_SRC) $(RANDOMBYTES_INCLUDE) -o $(BUILD)/$@

SimpleFIPS202.o: folders
	@echo -e "\n### Compiling $@\n"
	$(CC) $(C_FLAGS) -c $(XKCP_SRC_SIMPLE) $(XKCP_INCLUDE_SIMPLE) $(XKCP_INCLUDE) $(XKCP_LINKER) -o $(BUILD)/SimpleFIPS202.o

xkcp: folders
	@echo -e "\n### Compiling XKCP\n"
	make -C $(XKCP_SRC)



finite_fields.o: $(FFI_SRC)/finite_fields.c | folders
	@echo -e "\n### Compiling finite_fields\n"
	$(CC) $(C_FLAGS) -c $< $(FFI_INCLUDE) $(WRAPPER_INCLUDE) $(XKCP_INCLUDE) -o $(BUILD)/$@

%.o: $(SRC)/%.c | folders
	@echo -e "\n### Compiling $@\n"
	$(CC) $(C_FLAGS) -c $< $(INCLUDE) -o $(BUILD)/$@

%-verbose.o: $(SRC)/%.c | folders
	@echo -e "\n### Compiling $@ (verbose mode)\n"
	$(CC) $(C_FLAGS_VERBOSE) -c $< $(INCLUDE) -D VERBOSE -o $(BUILD)/$@



all: mira192f-main mira192f-kat mira192f-verbose ##@Build Build all the project


.PHONY: mira192f-main
mira192f-main: $(MIRA_OBJS) $(LIB_OBJS) | xkcp folders ##@Build a single example
	@echo -e "\n### Compiling MIRA-192F\n"
	$(CC) $(C_FLAGS) $(MAIN_MIRA) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@

.PHONY: mira192f-kat
mira192f-kat: $(MIRA_OBJS) $(LIB_OBJS) | xkcp folders ##@Build generate KAT files: PQCsignKAT_XX.req and PQCsignKAT_XX.rsp
	@echo -e "\n### Compiling MIRA-192F (KATs)\n"
	$(CC) $(C_FLAGS) $(MAIN_KAT) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@

.PHONY: mira192f-verbose
mira192f-verbose: $(MIRA_VERBOSE_OBJS) $(LIB_OBJS) | xkcp folders ##@Build Generate intermediate values of a single example
	@echo -e "\n### Compiling MIRA-192F (verbose mode)\n"
	$(CC) $(C_FLAGS_VERBOSE) $(MAIN_MIRA) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@



.PHONY: clean
clean:
	make -C $(XKCP_SRC) clean
	rm -f PQCkemKAT_*
	rm -f vgcore.*
	rm -rf ./bin



.PHONY: help
HELP_FUN = \
    %help; while(<>){push@{$$help{$$2//'options'}},[$$1,$$3] \
    if/^([\w-_]+)\s*:.*\#\#(?:@(\w+))?\s(.*)$$/}; \
    print"$$_:\n", map"  \033[37m$$_->[0]\033[0m".(" "x(20-length($$_->[0])))."$$_->[1]\n",\
    @{$$help{$$_}},"\n" for keys %help; \

help: ##@Miscellaneous Show this help
	@echo -e "Usage: make [target] ..."
	@perl -e '$(HELP_FUN)' $(MAKEFILE_LIST)
	@echo -e "Written by \033[36m$(SCRIPT_AUTHOR)\033[0m, version \033[37m$(SCRIPT_VERSION)\033[0m"
	@echo -e "Please report any bug or error to the authors."
