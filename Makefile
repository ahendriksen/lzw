CC= gcc

INCLUDES = -I ./include
CFLAGS = -std=c99 -O3 -DMCBSP_COMPATIBILITY_MODE -Wall $(INCLUDES)
LFLAGS= lib/compat-libmcbsp1.2.0.a -pthread -lm -lrt

OBJPRIM= obj/compresspar.o obj/bspedupack.o

IDIR = ./include
_DEPS = bspedupack.h mcbsp.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

.PHONY: all clean transfer

all: primes

primes: $(OBJPRIM)
	$(CC) $(CFLAGS) -o bin/parallel_lzw $(OBJPRIM) $(LFLAGS)


obj/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

transfer: 
	rsync -av ./src ./include ./job Makefile cartesius:Students15/s1078666/final/

clean:
	rm -f obj/primes.o bin/* 
