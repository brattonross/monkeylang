build: mkbin
	zig cc src/*.c -o bin/monkey

mkbin:
	mkdir -p bin

run: build
	./bin/monkey

.PHONY: test
test: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic test/*.c src/array_list.c src/ast.c src/lexer.c src/parser.c src/token.c -o bin/test
	./bin/test

test-debug: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic -g test/*.c src/array_list.c src/ast.c src/lexer.c src/parser.c src/token.c -o bin/test
	gdb ./bin/test
