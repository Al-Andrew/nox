
CFLAGS=-Wall -Wextra -Wconversion -Wpedantic -std=c99
CFLAGS_DEBUG=-g -O0
CFLAGS_RELEASE=-O3

.PHONY: all clean
all: bin/nox_debug bin/nox_release

clean:
	rm -rf bin

bin:
	mkdir -p bin

bin/nox_debug: bin
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o bin/nox_debug src/main.c

bin/nox_release: bin
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -o bin/nox_release src/main.c

