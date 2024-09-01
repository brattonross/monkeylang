.PHONY: test

build:
	mkdir -p bin
	zig cc src/*.c -o bin/monkey

run: build
	./bin/monkey

test:
	mkdir -p bin
	zig cc test/*.c src/lexer.c -o bin/test
	./bin/test
