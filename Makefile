CC = gcc

SRCDIR = src
INCDIR = include
BUILDDIR = build


SRCS = $(wildcard $(SRCDIR)/*.c)

CFLAGS = -I$(INCDIR) -Wall -Wextra -Wpedantic -O3
LIBS = -lm -lgsl -lgslcblas -fopenmp

PROGRAM = wildfire

.PHONY: all clean

all: $(BUILDDIR)/$(PROGRAM)

$(BUILDDIR):
	@mkdir -p -- "$@"

$(BUILDDIR)/$(PROGRAM): $(SRCS) | $(BUILDDIR)
	$(CC) $(SRCS) -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -rf -- "$(BUILDDIR)"