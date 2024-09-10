SHELL := /bin/bash

LIBS = -lssl -lcrypto
CC := gcc

ifdef DEBUG
CFLAGS := -g -Wall -DDEBUG
OBJDIR := debug
else
CFLAGS := -O3 -Wall
OBJDIR := build
endif

ifdef LOG
CFLAGS += -DDEBUG
endif

CFLAGS += -I$(OBJDIR) -INIST

EXES = test bench PQCgenKAT_sign

TARGETS := ${EXES:%=$(OBJDIR)/%}

.PHONY: default clean

default: $(EXES)

OBJECTS = meds.o util.o seed.o osfreq.o fips202.o matrixmod.o bitstream.o randombytes.o
HEADERS = $(wildcard *.h)

BUILDOBJ := ${OBJECTS:%=$(OBJDIR)/%}


$(EXES) : % :
	@make $(OBJDIR)/$(@F)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BUILDOBJ) : $(OBJDIR)/%.o: %.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETS) : $(OBJDIR)/%: %.c $(BUILDOBJ)
	$(CC) $(@F).c $(BUILDOBJ) $(CFLAGS) $(LIBS) -o $@


TEST: test
	$(OBJDIR)/test

KAT: PQCgenKAT_sign
	$(OBJDIR)/PQCgenKAT_sign

BENCH: bench
	$(OBJDIR)/bench

clean:
	rm -rf build/ debug/ PQCsignKAT_*

