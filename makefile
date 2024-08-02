CC = gcc
CCWARNINGS = -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wredundant-decls -Wshadow
CFLAGS = -std=c17 -MMD -MP $(CCWARNINGS)
DEBUGFLAGS = -O0 -g3 $(CFLAGS)
RELEASEFLAGS = -O3 -g0 $(CFLAGS)

# Note - If on windows, and either mkdir or rm isn't found, make sure Git\usr\bin is in PATH and restart terminal if necessary

debug: build assemblerdebug

release: releasebuild assemblerrelease

clean:
	-rm -r build
	-rm -r releasebuild
	-rm assembler.exe

build:
	mkdir build

assemblerdebug: build/main.o build/instructions.o build/parsing.o
	$(CC) $^ -o assembler $(DEBUGFLAGS)

build/%.o: src/%.c
	$(CC) -c $< -o $@ $(DEBUGFLAGS)

-include $(wildcard build/*.d)

releasebuild:
	mkdir releasebuild

assemblerrelease: releasebuild/main.o releasebuild/instructions.o releasebuild/parsing.o
	$(CC) $^ -o assembler $(RELEASEFLAGS)

releasebuild/%.o: src/%.c
	$(CC) -c $< -o $@ $(RELEASEFLAGS)

-include $(wildcard releasebuild/*.d)