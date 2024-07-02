CC = gcc
CFLAGS = -std=c17 -D__USE_MINGW_ANSI_STDIO -g3 -Wall -O0
CFILES = src/main.c src/instructions.c src/parsing.c

Assembler: $(CFILES)
	$(CC) $(CFILES) -o "$@" $(CFLAGS)