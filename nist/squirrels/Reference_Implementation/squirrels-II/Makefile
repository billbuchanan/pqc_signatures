# Build script for the Squirrels implementation.
#
# ==========================(LICENSE BEGIN)============================
#
# Copyright (c) 2023  Squirrels Project
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ===========================(LICENSE END)=============================
#
# @author   Guilhem Niot <guilhem@gniot.fr>

# This Makefile compiles the implementation in this directory along with
# the known answer tests generator located in the ../KAT/generator/
# directory. It targets a given NIST security level, configurable with
# the variable NIST_LEVEL. The output is two executable programs in the 
# build/ subdirectory: build/benchmarks launches speed tests for key
# generation, signature generation and verification, and build/katint
# generation test vectors in files .req and .rsp files in the expected NIST
# format.

.POSIX:

NIST_LEVEL?=2

CC = gcc
CFLAGS = -W -Wall -O2 -I../../lib/build/mpfr/include -I../../lib/build/gmp/include -I../../lib/build/flint/include/flint -I../../lib/build/fplll/include \
	-DSQUIRRELS_LEVEL=$(NIST_LEVEL)
LD = gcc -v
LDFLAGS = 
LIBSRPATH = '$$ORIGIN'/../../../lib/build
LIBS = -lm \
	-L../../lib/build/mpfr/lib -Wl,-rpath,$(LIBSRPATH)/mpfr/lib -lmpfr \
	-L../../lib/build/gmp/lib -Wl,-rpath,$(LIBSRPATH)/gmp/lib -lgmp \
	-L../../lib/build/flint/lib -Wl,-rpath,$(LIBSRPATH)/flint/lib -lflint \
	-L../../lib/build/fplll/lib -Wl,-rpath,$(LIBSRPATH)/fplll/lib -lfplll \
	-lstdc++

OBJ1 = build/codec.o build/common.o build/keygen_lll.o build/keygen.o  build/minors.o build/nist.o build/normaldist.o build/param.o build/sampler.o build/shake.o build/sign.o build/vector.o

OBJ2 = build/PQCgenKAT_sign.o build/katrng.o
OBJ3 = build/benchmarks.o build/katrng.o

HEAD1 = api.h fpr.h inner.h param.h
HEAD2 = api.h ../../KAT/generator/katrng.h

all: lib build build/katint build/benchmarks

lib:
	make -C ../../lib

build:
	-mkdir build

clean:
	-rm -f build/katint build/benchmarks $(OBJ1) $(OBJ2) $(OBJ3)

build/katint: $(OBJ1) $(OBJ2)
	$(LD) $(LDFLAGS) -o build/katint $(OBJ1) $(OBJ2) $(LIBS)

build/benchmarks: $(OBJ1) $(OBJ3)
	$(LD) $(LDFLAGS) -o build/benchmarks $(OBJ1) $(OBJ3) $(LIBS)

build/codec.o: codec.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/codec.o codec.c

build/common.o: common.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/common.o common.c

build/keygen_lll.o: keygen_lll.cpp $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/keygen_lll.o keygen_lll.cpp

build/keygen.o: keygen.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/keygen.o keygen.c

build/minors.o: minors.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/minors.o minors.c

build/nist.o: nist.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/nist.o nist.c

build/normaldist.o: normaldist.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/normaldist.o normaldist.c

build/param.o: param.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/param.o param.c

build/sampler.o: sampler.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/sampler.o sampler.c

build/shake.o: shake.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/shake.o shake.c

build/sign.o: sign.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/sign.o sign.c

build/vector.o: vector.c $(HEAD1)
	$(CC) $(CFLAGS) -c -o build/vector.o vector.c

build/katrng.o: ../../KAT/generator/katrng.c $(HEAD2)
	$(CC) $(CFLAGS) -I . -c -o build/katrng.o ../../KAT/generator/katrng.c

build/PQCgenKAT_sign.o: ../../KAT/generator/PQCgenKAT_sign.c $(HEAD2)
	$(CC) $(CFLAGS) -I . -c -o build/PQCgenKAT_sign.o ../../KAT/generator/PQCgenKAT_sign.c

build/benchmarks.o: benchmarks.c $(HEAD1) $(HEAD2)
	$(CC) $(CFLAGS) -I . -c -o build/benchmarks.o benchmarks.c
