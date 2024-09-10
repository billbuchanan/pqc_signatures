CC=    gcc
LD=    gcc

ifndef opt
# opt = avx2
opt = u64
endif

ifndef PROJ
PROJ = Ip
# PROJ = Is
#PROJ = III
#PROJ = V
endif

SRC_DIR  := $(PROJ)

CFLAGS	 := -O3 -std=c11 -Wall -Wextra -Wpedantic -fno-omit-frame-pointer
INCPATH  := -I/usr/local/include -I/opt/local/include -I/usr/include -I$(SRC_DIR)
LDFLAGS  := $(LDFLAGS)
LIBPATH  = -L/usr/local/lib -L/opt/local/lib -L/usr/lib
LIBS     = -lcrypto

ifeq ($(opt), avx2)
	CFLAGS += -D_BLAS_AVX2_
endif
ifeq ($(opt), u64)
	CFLAGS += -D_BLAS_UINT64_
endif


ifeq ($(PROJ), Ip)
	CFLAGS += -D_TUOV256_112_44
endif
ifeq ($(PROJ), Is)
	CFLAGS += -D_TUOV16_160_64
endif
ifeq ($(PROJ), III)
	CFLAGS += -D_TUOV256_184_72
endif
ifeq ($(PROJ), V)
	CFLAGS += -D_TUOV256_244_96
endif

ifdef pkc
CFLAGS += -D_TUOV_PKC
endif

ifdef skc
CFLAGS += -D_TUOV_PKC_SKC
endif



SRCS           :=  $(wildcard $(SRC_DIR)/*.c)
SRCS_O         :=  $(SRCS:.c=.o)
SRCS_O_NOTDIR  :=  $(notdir $(SRCS_O))

OBJ = $(SRCS_O_NOTDIR)

ifdef KAT
	CFLAGS       += -D_NIST_KAT_
	SRC_EXT_DIRS := nistkat
	INCPATH      += -I./nistkat
	OBJ          += rng.o
endif


.PHONY: tests clean

tests: api_test
	${hide}./api_test

api_test: $(OBJ) tests/api_test.c
	${hide}$(CC) $(CFLAGS) $(INCPATH) $(LDFLAGS) $(LIBPATH) $^ -o $@ $(LIBS) 

test_speed:  $(OBJ) tests/test_speed.c tests/speed_print.c tests/cpucycles.c
	${hide}$(CC) $(CFLAGS) $(INCPATH) $(LDFLAGS) $(LIBPATH) $^ -o $@ $(LIBS) 


PQCgenKAT_sign: $(OBJ) nistkat/PQCgenKAT_sign.c
	${hide}$(CC) $(CFLAGS) $(INCPATH) $(LDFLAGS) $(LIBPATH) $^ -o $@ $(LIBS)

%.o: $(SRC_DIR)/%.c
	${hide}$(CC) $(CFLAGS) $(INCPATH) -c $<


define GEN_O
%.o: $(1)/%.c
	${hide}$(CC) $(CFLAGS) $(INCPATH) -c $$<
endef
$(foreach dir, $(SRC_EXT_DIRS), $(eval $(call GEN_O,$(dir))))


clean:
	${hide}-rm -f *.o test_* api_test *.req *.rsp PQCgenKAT_sign
