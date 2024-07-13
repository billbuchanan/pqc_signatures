CC = cc
CFLAGS := ${CFLAGS} -DUSE_PREON192B -DAES192=1 -DUSE_PRNG -O3
LFLAGS := ${LFLAGS} -lm -lssl -lcrypto
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

all: $(OBJS)
	@test -e .stopmakeonce || $(CC) $(CFLAGS) $(OBJS) $(LFLAGS) -o PQCgenKAT_sign
	@test -e .stopmakeonce && rm .stopmakeonce || true

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean 

clean:
	@rm -f *.o PQCgenKAT_sign
