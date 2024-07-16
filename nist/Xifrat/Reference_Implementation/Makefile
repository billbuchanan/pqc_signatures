# 2023-03-31 old note no longer relevant.
# This Makefile uses "pkg-config" to automatically find
# necessary build flags for the OpenSSL library dependency.
# The "pkg-config" program and the OpenSSL package should be
# installed for this to work, otherwise, tweak the variables
# 'LDFLAGS' and 'CFLAGS' accordingly.

# LDFLAGS = `pkg-config --libs openssl` -L /lib/x86_64-linux-gnu
# CFLAGS = `pkg-config --cflags openssl`

CFLAGS = -I.

.PHONY: common clean pkgconf_test

OBJS = \
rng.o sign.o PQCgenKAT_sign.o \
rijndael.o shake.o sponge.o keccak-f-1600.o endian.o \
xifrat-sign.o xifrat-funcs.o

PQCgenKAT_sign: $(OBJS) 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

rng.o: rng.h
api.o: api.h
PQCgenKAT_sign.o: rng.h api.h

clean:; rm $(OBJS) PQCgenKAT_sign
common: common.h mysuitea-common.h
pkgconf_test:
	@ if { command -v pkg-config && \
		pkg-config --libs openssl && \
		pkg-config --cflags openssl ; } >/dev/null 2>&1 ; then : ; else \
	echo make cannot automatically determine some build flags. ; \
	echo see notes at the start of the Makefile for details. ; false ; fi

shake.o: shake.h keccak.h sponge.h common
sponge.o: sponge.h common
endian.o: endian.h common
keccak-f-1600.o: keccak.c.h keccak.h common

xifrat-sign.o: xifrat-sign.h xifrat-funcs.h shake.h common
xifrat-funcs.o: xifrat-funcs.h common
