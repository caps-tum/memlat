# Intel compiler
CC=icc

CFLAGS=-O3

bench: bench.o

clean:
	rm -rf *.o *~ bench
