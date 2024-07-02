CC = gcc
CFLAGS = -std=c17 -g3 -Wall -O0
CFILES = src/main.c src/instructions.c src/parsing.c

Assembler: $(CFILES)
	$(CC) $(CFILES) -o "$@" $(CFLAGS)