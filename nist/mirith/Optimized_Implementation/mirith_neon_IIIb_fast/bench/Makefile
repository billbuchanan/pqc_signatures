CC=gcc
CFLAGS= -O3 -march=native -flto -mtune=native -flax-vector-conversions -funroll-loops -fomit-frame-pointer -std=c11 -Wall -Wextra
DEPS=$(wildcard ../*.h)
OBJ= $(patsubst ../%.c,../%.o,$(wildcard ../*.c))
OBJ+=$(patsubst ../%.s,../%.o,$(wildcard ../*.s))
AWK=awk
VALGRIND=valgrind

BENCH=bench_mirith
O_BENCH=bench_mirith.o

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    ASMFLAGS := ${CFLAGS}
endif
ifeq ($(UNAME_S),Darwin)
    ASMFLAGS :=
endif



all: $(BENCH)

%.o: %.s
	$(CC) -c $(ASMFLAGS) -o $@ $<

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BENCH): $(O_BENCH) $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
		
run: $(BENCH)
	sudo ./$(BENCH)

avg: $(BENCH)
	sudo ./$(BENCH) > avg.in
	${AWK} '!/Benchmarks/' avg.in > temp && mv temp avg.in
	${AWK} '!/DONE/' avg.in > temp && mv temp avg.in
	${AWK} NF avg.in > temp && mv temp avg.in
	${AWK} '{sum = 0; for (i = 2; i <= NF; i++) sum += $$i; sum /= NF; print sum}' avg.in
	rm -f avg.in temp

massif_mirith_sign: $(OBJ)
	$(CC) -D MASSIF_SIGN=1 -g -o $@ $^ $(CFLAGS) $(LIBS) massif_mirith.c -o massif_mirith_sign
	${VALGRIND} --tool=massif --detailed-freq=1 --max-snapshots=1000 --stacks=yes --massif-out-file=massif.out.sign ./massif_mirith_sign

massif_mirith_vrfy: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS) massif_mirith.c -o massif_mirith_vrfy
	${VALGRIND} --tool=massif --detailed-freq=1 --max-snapshots=1000 --stacks=yes --massif-out-file=massif.out.vrfy ./massif_mirith_vrfy

massif_mirith_keygen: $(OBJ)
	$(CC) -D MASSIF_KEYGEN -g -o $@ $^ $(CFLAGS) $(LIBS) massif_mirith.c -o massif_mirith_keygen
	${VALGRIND} --tool=massif --detailed-freq=1 --max-snapshots=1000 --stacks=yes --massif-out-file=massif.out.keygen ./massif_mirith_keygen
	
.PHONY: clean

clean:
	rm -f *.o *.su
	rm -f ../*.o ../*.su
	rm -f $(BENCH)
	rm -f massif_mirith
	rm -f massif_mirith_vrfy
	rm -f massif_mirith_sign
	rm -f massif_mirith_keygen
	rm -f massif.out.*
	rm -f massif.out

