build: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic src/*.c -o bin/monkey

build-debug: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic -g src/*.c -o bin/monkey

mkbin:
	mkdir -p bin

run: build
	./bin/monkey

.PHONY: test
test: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic test/*.c src/array_list.c src/ast.c src/eval.c src/lexer.c src/object.c src/parser.c src/token.c -o bin/test
	./bin/test

test-debug: mkbin
	zig cc -std=gnu23 -Wall -Wextra -pedantic -g test/*.c src/array_list.c src/ast.c src/eval.c src/lexer.c src/object.c src/parser.c src/token.c -o bin/test
