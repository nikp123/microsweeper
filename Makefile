src = $(wildcard src/*.c)
obj = $(src:.c=.o)

PREFIX ?= /usr/local
CONFIG_PREFIX ?= /etc

minimal_FLAGS = -DBUILD_MINIMAL
2k_FLAGS      = -DBUILD_2K
qr_FLAGS      = -DBUILD_QR
full_FLAGS    = -DBUILD_FULL

TARGET ?= full

CFLAGS = -m32 -DSMOL ${${TARGET}_FLAGS}
LDFLAGS = -lX11 -lc -O2 -nostdlib -m32 -L/usr/lib32
CC = tcc

all: microsweeper
all: compress_pack

debug: LDFLAGS = -g -lX11 -lc -L/usr/lib32
debug: CFLAGS  = -g -m32 ${${TARGET}_FLAGS}
debug: CC = gcc
debug: microsweeper

microsweeper: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

#	strip --strip-all microsweeper

compress_pack:
	xz -9 -z microsweeper
	cat vondehi microsweeper.xz > microsweeper
	chmod +x microsweeper
	rm microsweeper.xz


.PHONY: clean
clean:
	rm -f $(obj) microsweeper microsweeper.xz

