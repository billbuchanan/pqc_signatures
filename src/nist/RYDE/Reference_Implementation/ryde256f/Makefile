
SCRIPT_VERSION=v1.0
SCRIPT_AUTHOR=RYDE team

CC=gcc
C_FLAGS:=-O3 -flto -std=c99 -pedantic -Wall -Wextra -DSHAKE_TIMES4
C_FLAGS_VERBOSE:=-O3 -flto -std=c99 -pedantic -Wall -Wextra -DSHAKE_TIMES4 -DVERBOSE

RANDOMBYTES_SRC:=lib/randombytes/randombytes.c
RANDOMBYTES_INCLUDE:=-I lib/randombytes -lcrypto

XKCP_SRC:=lib/XKCP
XKCP_SRC_SIMPLE:=$(XKCP_SRC)/SimpleFIPS202.c
XKCP_INCLUDE:=-I$(XKCP_SRC) -I$(XKCP_SRC)/avx2
XKCP_INCLUDE_SIMPLE:=-I $(XKCP_SRC)
XKCP_LINKER:=-L$(XKCP_SRC) -lshake

WRAPPER_SRC:=src/wrapper
WRAPPER_INCLUDE:=-I $(WRAPPER_SRC)

RBC_SRC:=src/rbc-43
RBC_INCLUDE:=-I $(RBC_SRC)

SRC:=src
INCLUDE:=-I src $(RBC_INCLUDE) $(WRAPPER_INCLUDE) $(XKCP_INCLUDE) $(RANDOMBYTES_INCLUDE)

MAIN_RYDE:=src/main.c
KAT_RYDE:=src/PQCgenKAT_sign.c

RYDE_OBJS:=rbc_43_elt.o rbc_43_vec.o rbc_43_qpoly.o rbc_43_vspace.o rbc_43_mat.o keypair.o signature.o verification.o mpc.o parsing.o tree.o sign.o
RYDE_OBJS_VERBOSE:=rbc_43_elt.o rbc_43_vec.o rbc_43_qpoly.o rbc_43_vspace.o rbc_43_mat.o keypair-verbose.o signature-verbose.o verification-verbose.o mpc.o parsing.o tree.o sign.o
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



rbc_%.o: $(RBC_SRC)/rbc_%.c | folders
	@echo -e "\n### Compiling $@\n"
	$(CC) $(C_FLAGS) -c $< $(RBC_INCLUDE) $(WRAPPER_INCLUDE) $(XKCP_INCLUDE) -o $(BUILD)/$@

%.o: $(SRC)/%.c | folders
	@echo -e "\n### Compiling $@\n"
	$(CC) $(C_FLAGS) -c $< $(INCLUDE) -o $(BUILD)/$@

%-verbose.o: $(SRC)/%.c | folders
	@echo -e "\n### Compiling $@ (verbose mode)\n"
	$(CC) $(C_FLAGS_VERBOSE) -c $< $(INCLUDE) -o $(BUILD)/$@



all: ryde256f-main ryde256f-kat ryde256f-verbose ##@Build Build all the project



.PHONY: ryde256f-main
ryde256f-main: $(RYDE_OBJS) $(LIB_OBJS) | xkcp folders ##@Build a single example
	@echo -e "\n### Compiling RYDE-256F\n"
	$(CC) $(C_FLAGS) $(MAIN_RYDE) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@

.PHONY: ryde256f-kat
ryde256f-kat: $(RYDE_OBJS) $(LIB_OBJS) | xkcp folders ##@Build generate KAT files: PQCsignKAT_XX.req and PQCsignKAT_XX.rsp
	@echo -e "\n### Compiling RYDE-256F (KATs)\n"
	$(CC) $(C_FLAGS) $(KAT_RYDE) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@

.PHONY: ryde256f-verbose
ryde256f-verbose: $(RYDE_OBJS_VERBOSE) $(LIB_OBJS) | xkcp folders ##@Build Generate intermediate values of a single example
	@echo -e "\n### Compiling RYDE-256F (verbose mode)\n"
	$(CC) $(C_FLAGS_VERBOSE) $(MAIN_RYDE) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(XKCP_LINKER) -o $(BIN)/$@



.PHONY: clean
clean: ##@Miscellaneous Clean data
	make -C $(XKCP_SRC) clean
	rm -f PQCsignKAT_*
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
