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
../Reference_Implementation/rng.o \
../Reference_Implementation/sign.o \
../Reference_Implementation/PQCgenKAT_sign.o \
../Reference_Implementation/rijndael.o \
../Reference_Implementation/shake.o \
../Reference_Implementation/sponge.o \
../Reference_Implementation/keccak-f-1600.o \
../Reference_Implementation/endian.o \
xifrat-sign.o xifrat-funcs.o

PQCgenKAT_sign: $(OBJS) 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

../Reference_Implementation/rng.o: \
../Reference_Implementation/rng.c \
../Reference_Implementation/rng.h \

../Reference_Implementation/sign.o: \
../Reference_Implementation/sign.c \
../Reference_Implementation/api.h \

../Reference_Implementation/PQCgenKAT_sign.o: \
../Reference_Implementation/PQCgenKAT_sign.c \
../Reference_Implementation/rng.h \
../Reference_Implementation/api.h

clean:; rm $(OBJS) PQCgenKAT_sign

common: \
common.h \
../Reference_Implementation/mysuitea-common.h

pkgconf_test:
	@ if { command -v pkg-config && \
		pkg-config --libs openssl && \
		pkg-config --cflags openssl ; } >/dev/null 2>&1 ; then : ; else \
	echo make cannot automatically determine some build flags. ; \
	echo see notes at the start of the Makefile for details. ; false ; fi

../Reference_Implementation/shake.o: \
../Reference_Implementation/shake.c \
../Reference_Implementation/shake.h \
../Reference_Implementation/keccak.h \
../Reference_Implementation/sponge.h \
common

../Reference_Implementation/sponge.o: \
../Reference_Implementation/sponge.c \
../Reference_Implementation/sponge.h \
common

../Reference_Implementation/endian.o: \
../Reference_Implementation/endian.c \
../Reference_Implementation/endian.h \
common

../Reference_Implementation/keccak-f-1600.o: \
../Reference_Implementation/keccak-f-1600.c \
../Reference_Implementation/keccak.c.h \
../Reference_Implementation/keccak.h \
common

xifrat-sign.o: \
../Reference_Implementation/shake.o \
../Reference_Implementation/sponge.o \
../Reference_Implementation/endian.o \
../Reference_Implementation/keccak-f-1600.o \
xifrat-sign.h xifrat-funcs.h \
common

xifrat-funcs.o: xifrat-funcs.h common
