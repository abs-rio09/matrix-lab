CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2
LDLIBS  := -lm

SRC     := src/main.c src/matrix.c
BIN     := build/matrix

.PHONY: all test memcheck clean

all: $(BIN)

$(BIN): $(SRC) src/matrix.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDLIBS)

# Python unittest suite drives the compiled binary
test: $(BIN)
	python3 -m unittest discover -s tests -v

# Leak check with valgrind on a real multiplication
memcheck: $(BIN)
	valgrind --leak-check=full --error-exitcode=1 ./$(BIN) mul examples/a.txt examples/b.txt

clean:
	rm -rf build
