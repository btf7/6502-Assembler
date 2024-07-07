CC = gcc
CFLAGS = -std=c17 -g3 -Wall -O0
CFILES = src/main.c src/instructions.c src/parsing.c
HFILES = src/main.h

Assembler: $(CFILES) $(HFILES)
	$(CC) $(CFILES) -o "$@" $(CFLAGS)