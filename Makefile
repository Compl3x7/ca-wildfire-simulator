CC = gcc

SRCDIR = src
INCDIR = include
BUILDDIR = build

CFLAGS = -I$(INCDIR) $(SRCDIR)/*.c -Wall -Wextra -Wpedantic -O2
LIBS = -lm -lgsl -lgslcblas -fopenmp

PROGRAM = wildfire

build:
	$(CC) -o $(PROGRAM) $(CFLAGS) $(LIBS)

clean:
	rm -f $(PROGRAM)