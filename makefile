CC = gcc
CFLAGS = -std=c17 -D__USE_MINGW_ANSI_STDIO -g -Wall -O1
CFILES = src/main.c

Assembler: $(CFILES)
	$(CC) $(CFILES) -o "$@" $(CFLAGS)