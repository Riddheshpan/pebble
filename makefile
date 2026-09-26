CC = gcc

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic

LLVM_CFLAGS = $(shell llvm-config --cflags)
LLVM_LDFLAGS = $(shell llvm-config --ldflags)
LLVM_LIBS = $(shell llvm-config --libs core native)

CFLAGS += $(LLVM_CFLAGS)
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS)

SMOKE = pebble-smoke

.PHONY: all smoke clean

all: smoke

smoke:
	$(CC) $(CFLAGS) compiler/codegen/smoke.c $(LDFLAGS) -o $(SMOKE)

clean:
	rm -f $(SMOKE) pebble-smoke.o pebble-smoke-native