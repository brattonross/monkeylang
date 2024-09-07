.PHONY: test

build:
	mkdir -p bin
	zig cc src/*.c -o bin/monkey

run: build
	./bin/monkey

test:
	mkdir -p bin
	zig cc test/*.c src/ast.c src/lexer.c src/parser.c -o bin/test
	./bin/test

test-debug:
	mkdir -p bin
	zig cc -g test/*.c src/ast.c src/lexer.c src/parser.c -o bin/test
	gdb ./bin/test
