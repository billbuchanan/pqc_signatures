CC=gcc
WARNING_FLAGS=-Wall -Wextra -Wpedantic
CFLAGS= -O2 -march=native $(WARNING_FLAGS) -I./sha3
SHA3LIB=libshake.a
SHA3_PATH=sha3
LDFLAGS= $(SHA3_PATH)/$(SHA3LIB) 

OBJECTS= hash.o

all: $(SHA3LIB) $(OBJECTS) 

$(SHA3LIB):
	$(MAKE) -C $(SHA3_PATH)

.c.o: 
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o 2>/dev/null
	$(MAKE) -C $(SHA3_PATH) clean
