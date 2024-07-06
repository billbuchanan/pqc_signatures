# Compiler settings
CC      := gcc
CFLAGS  := -Iinc -Wall -Wextra -Ofast -march=native -fstack-usage
LDLIBS  := -lcrypto

# Define binaries
PQCBIN  := pqcsign
# XBIN    := xtest

#	slower instrumentation flags
#CFLAGS	=	-Wall -Wextra -Wshadow -fsanitize=address,undefined -O2 -g 
#	options
#CFLAGS	+=	-DPOLYR_Q32
#CFLAGS	+=	-DMASK_RANDOM_ASCON


# Source files for each binary
PQCSRC  := $(filter-out PQCgenKAT_sign.c, $(wildcard *.c util/*.c))
XSRCS   := $(filter-out pqcsign.c, $(wildcard *.c util/*.c))

# Objects for each binary
PQCOBJS := $(PQCSRC:.c=.o)
XOBJS   := $(XSRCS:.c=.o)

# Default target
all: $(PQCBIN) 
#$(XBIN)

# Build pqcsign binary
$(PQCBIN): $(PQCOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Build xtest binary
$(XBIN): $(XOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Compilation pattern rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(PQCOBJS) $(XOBJS) $(PQCBIN)  *~ *.su
#$(XBIN)