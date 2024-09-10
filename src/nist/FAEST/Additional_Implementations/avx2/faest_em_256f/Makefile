CC?=gcc
CXX?=g++
CFLAGS+=-g -O2 -march=native -mtune=native -std=c11
CPPFLAGS+=-DHAVE_OPENSSL -DNDEBUG -MMD -MP -MF $*.d

SOURCES=$(filter-out randomness_os.c randomness_randombytes.c rng.c,$(wildcard *.c)) $(wildcard *.s)
EXECUTABLE_APITEST=api_test
EXECUTABLE_NISTKAT=PQCgenKAT_sign
LIBFAEST=libfaest.a
NISTKAT_SOURCES=PQCgenKAT_sign.c rng.c

all: $(LIBFAEST) $(EXECUTABLE_APITEST) $(EXECUTABLE_NISTKAT)
.PHONY: all

$(LIBFAEST): $(SOURCES:.c=.o) $(SOURCES:.s=.o)
	ar rcs $@ $^

$(EXECUTABLE_NISTKAT): CPPFLAGS+=-DHAVE_RANDOMBYTES
$(EXECUTABLE_NISTKAT): CFLAGS+=-Wno-sign-compare -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-result

$(EXECUTABLE_APITEST): $(EXECUTABLE_APITEST).o $(LIBFAEST) randomness_os.o

$(EXECUTABLE_NISTKAT): $(NISTKAT_SOURCES:.c=.o) $(LIBFAEST) randomness_randombytes.o

$(EXECUTABLE_NISTKAT) $(EXECUTABLE_APITEST):
	$(CC) $(CPPFLAGS) $(LDFLAGS) $^ -lcrypto -o $@

clean:
	rm -f *.d *.o $(LIBFAEST) $(EXECUTABLE_APITEST) $(EXECUTABLE_NISTKAT)
.PHONY: clean
