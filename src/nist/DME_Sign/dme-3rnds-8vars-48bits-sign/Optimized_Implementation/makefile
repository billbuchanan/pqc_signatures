all: rng.o sign.o sha3.o dme.o kat

rng.o: rng.c rng.h
	gcc -c rng.c -O3

sign.o: sign.c api.h sha3.h dme.h rng.h
	gcc -c sign.c -Wall -Wextra -Werror -pedantic -std=gnu99 -O3

sha3.o: sha3.c sha3.h
	gcc -c sha3.c -Wall -Wextra -Werror -pedantic -std=gnu99 -O3

dme.o: dme.c dme.h rng.h
	gcc -o dme.o -c dme.c -Wall -Wextra -Werror -pedantic -std=gnu99 -O3

PQCgenKAT_sign.o: PQCgenKAT_sign.c rng.h api.h
	gcc -c PQCgenKAT_sign.c

PQCgenKAT_sign: PQCgenKAT_sign.o dme.o rng.o sha3.o sign.o
	gcc -o PQCgenKAT_sign PQCgenKAT_sign.o dme.o rng.o sha3.o sign.o -Wall -Wextra -O3 -lcrypto

kat: PQCgenKAT_sign
	./PQCgenKAT_sign

clean:
	rm -f *.o
	rm -f PQCgenKAT_sign
	rm -f *.req
	rm -f *.rsp

.PHONY: all clean kat
