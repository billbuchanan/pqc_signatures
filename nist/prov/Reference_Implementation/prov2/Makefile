CC=gcc
WARNING_FLAGS=-Wall -Wextra -Wpedantic
SHAKE_PATH= SHAKE
CFLAGS= -O2 -I./$(SHAKE_PATH)/sha3
KATFLAGS= -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
LDFLAGS= $(SHAKE_PATH)/sha3/libshake.a

PROV_OBJ= field.o matrix.o prov.o
KAT_OBJ= field.o matrix.o prov.o api.o rng.o
SHAKE_OBJ = $(SHAKE_PATH)/hash.o
SHAKE= shake

all: $(SHAKE) kat

shake:
	$(MAKE) -C $(SHAKE_PATH)

rng.o: rng.c
	$(CC) -c -O2 -I/usr/local/opt/openssl/include $< -o $@

kat: PQCgenKAT_sign

bench: run_bench

PQCgenKAT_sign: $(KAT_OBJ) rng.o
	$(CC) $(@).c $(CFLAGS) $(KATFLAGS) $(KAT_OBJ) $(SHAKE_OBJ) -o $@ $(LDFLAGS) -lssl -lcrypto 

test: $(PROV_OBJ) test.c
	    $(CC) $(@).c $(CFLAGS) $(WARNING_FLAGS) $(PROV_OBJ) $(SHAKE_OBJ) -o $@ $(LDFLAGS)

run_bench: $(PROV_OBJ) bench.o
	$(CC) $(@).c $(CFLAGS) $(WARNING_FLAGS) -DPROV_BENCH $(PROV_OBJ) bench.o $(SHAKE_OBJ) -o $@ $(LDFLAGS)

%.o : %.c
	    $(CC) -c $(CFLAGS) $(WARNING_FLAGS) $< -o $@

.PHONY : clean shake kat bench

clean:
		rm test run_bench PQCgenKAT_sign 2>/dev/null || true
		rm *.rsp *.req 2>/dev/null || true
	    rm *.o 2>/dev/null || true
	    rm *.exe 2>/dev/null || true
