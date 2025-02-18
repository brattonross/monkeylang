#include "../src/eval.c"
#include "../src/lexer.c"
#include "../src/mem.c"
#include "../src/object.c"
#include "../src/parser.c"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void test_eval_integer_expression(void);

int main(void) { test_eval_integer_expression(); }

void test_eval_integer_expression(void) {
  struct {
    char *input;
    int64_t expected;
  } test_cases[] = {
      {"5", 5},
      {"10", 10},
  };

  Arena arena = {0};
  const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &evaluated);

    assert(evaluated.type == OBJECT_INTEGER);
    assert(evaluated.data.integer.value == test_cases[i].expected);

    arena_reset(&arena);
  }
}
