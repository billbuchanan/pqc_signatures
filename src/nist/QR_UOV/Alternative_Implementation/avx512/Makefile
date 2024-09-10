CC=gcc
CFLAGS=-O3 -fomit-frame-pointer -Wno-unused-result -Wno-aggressive-loop-optimizations -I. -march=native -fopenmp # -DQRUOV_HASH_LEGACY # -ggdb3
LDFLAGS=-lcrypto -Wl,-Bstatic -lcrypto -Wl,-Bdynamic -lm
OBJS=Fql.o mgf.o PQCgenKAT_sign.o qruov.o rng.o sign.o matrix.o

.SUFFIXES:
.SUFFIXES: .rsp .req .diff .c .o .h

.PHONY: all clean

all: qruov_config.h api.h PQCgenKAT_sign
	./PQCgenKAT_sign

PQCgenKAT_sign: Makefile ${OBJS}
	${CC} ${OBJS} ${CFLAGS} ${LDFLAGS} -o $@

qruov_config.h: qruov_config_h_gen.c
	${CC} @qruov_config.txt -DQRUOV_PLATFORM=avx512 -DQRUOV_CONFIG_H_GEN ${CFLAGS} ${LDFLAGS} qruov_config_h_gen.c
	./a.out > qruov_config.h
	rm a.out

api.h: api_h_gen.c
	${CC} -DAPI_H_GEN ${CFLAGS} ${LDFLAGS} api_h_gen.c
	./a.out > api.h
	rm a.out

clean:
	rm -f PQCgenKAT_sign PQCsignKAT_*.req PQCsignKAT_*.rsp ${OBJS}
