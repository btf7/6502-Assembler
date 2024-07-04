CC = gcc
CFLAGS = -std=c17 -g3 -Wall -Og
CFILES = src/main.c src/instructions.c src/parsing.c

Assembler: $(CFILES)
	$(CC) $(CFILES) -o "$@" $(CFLAGS)