src = $(wildcard src/*.c)
obj = $(src:.c=.o)

PREFIX ?= /usr/local
CONFIG_PREFIX ?= /etc

minimal_FLAGS = -DSMOL -DBUILD_MINIMAL
2k_FLAGS      = -DSMOL -DBUILD_2K
full_FLAGS    = -DSMOL -DBUILD_FULL

TARGET ?= minimal

CFLAGS = -m32 -DSMOL ${${TARGET}_FLAGS}
LDFLAGS = -lX11 -lc -O2 -nostdlib -m32 -L/usr/lib32
CC = tcc

all: microsweeper
all: compress_pack

debug: LDFLAGS = -g -lX11 -lc -L/usr/lib32
debug: CFLAGS  = -g -m32
debug: CC = gcc
debug: microsweeper

microsweeper: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

compress_pack:
	xz -9 -z microsweeper
	cat vondehi microsweeper.xz > microsweeper
	chmod +x microsweeper
	rm microsweeper.xz


.PHONY: clean
clean:
	rm -f $(obj) microsweeper microsweeper.xz

