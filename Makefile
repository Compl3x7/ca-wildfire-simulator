#    Wildfire simulator using Probabilistic Cellular Automata (PCA)
#    Copyright (C) 2025-2026  Alexandre Rodrigues
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

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