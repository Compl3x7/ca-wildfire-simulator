CC = gcc

SRCDIR = src
INCDIR = include
BUILDDIR = build

CFLAGS = -I$(INCDIR) $(SRCDIR)/*.c -Wall -Wextra -Wpedantic -O3
LIBS = -lm -lgsl -lgslcblas -fopenmp

PROGRAM = wildfire

build:
	$(CC) -o $(PROGRAM) $(CFLAGS) $(LIBS)

clean:
	rm -f $(PROGRAM)