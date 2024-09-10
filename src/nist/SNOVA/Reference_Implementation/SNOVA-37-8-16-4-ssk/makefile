CC = gcc

CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls -Wshadow -Wvla -Wpointer-arith -O3 -march=native -mtune=native

BUILD_OUT_PATH = ./build/

OLIST = $(BUILD_OUT_PATH)rng.o $(BUILD_OUT_PATH)snova.o

# snova params
SNOVA_V = 37
SNOVA_O = 8
SNOVA_L = 4
SK_IS_SEED = 1 # 0: sk = ssk; 1: sk = esk
CRYPTO_ALGNAME = \"SNOVA_$(SNOVA_V)_$(SNOVA_O)_$(SNOVA_L)\"
SNOVA_PARAMS = -D v_SNOVA=$(SNOVA_V) -D o_SNOVA=$(SNOVA_O) -D l_SNOVA=$(SNOVA_L) -D sk_is_seed=$(SK_IS_SEED) -D CRYPTO_ALGNAME=$(CRYPTO_ALGNAME)

clean:
	rm -f ./build/*.o *.a 

clean_all: 
	rm -f ./build/*.o *.a *.log *.req *.rsp

build/rng.o: 
	$(CC) $(CFLAGS) -c -o ./build/rng.o ./rng.c -lcrypto

build/snova.o: build/rng.o
	$(CC) $(CFLAGS) $(SNOVA_PARAMS) -c -o ./build/snova.o ./snova.c -lcrypto

build/sign.o: build/snova.o
	$(CC) $(CFLAGS) $(SNOVA_PARAMS) -c -o ./build/sign.o ./sign.c -lcrypto

test: build/rng.o build/snova.o
	$(CC) $(CFLAGS) $(SNOVA_PARAMS) $(OLIST) ./test/test.c -o test.a -lcrypto
	./test.a > test-$(SNOVA_V)-$(SNOVA_O)-$(SNOVA_L).log

test_api: build/rng.o build/snova.o build/sign.o
	$(CC) $(CFLAGS) $(SNOVA_PARAMS) $(OLIST) ./test/test_api.c ./build/sign.o -o test_api.a -lcrypto
	./test_api.a > test_api_$(SNOVA_V)_$(SNOVA_O)_$(SNOVA_L).log

PQCgenKAT: build/sign.o
	$(CC) $(CFLAGS) $(SNOVA_PARAMS) $(OLIST) ./build/sign.o ./PQCgenKAT_sign.c -o ./PQCgenKAT.a -lcrypto
	./PQCgenKAT.a
