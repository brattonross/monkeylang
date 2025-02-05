#include "../src/parser.c"
#include <assert.h>
#include <stddef.h>

void test_let_statements(void);
void test_let_statement(Statement s, String name);

int main(void) { test_let_statements(); }

void test_let_statements(void) {
  char *input = "let x = 5;\n"
                "let y = 10;\n"
                "let foobar = 838383;\n";

  Arena arena = {0};
  static const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, &arena_buffer, arena_size);

  Lexer lexer = {0};
  lexer_init(&lexer, input);
  Parser parser = {0};
  parser_init(&parser, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  assert(program->statements_len == 3);

  typedef struct {
    String expected_identifier;
  } TestCase;
  TestCase tests[] = {
      (TestCase){.expected_identifier = String("x")},
      (TestCase){.expected_identifier = String("y")},
      (TestCase){.expected_identifier = String("foobar")},
  };
  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    test_let_statement(*program_statement_at(program, i),
                       tests[i].expected_identifier);
  }
}

void test_let_statement(Statement s, String name) {
  assert(string_cmp(statement_token_literal(s), String("let")));
  assert(s.type == STATEMENT_LET);

  LetStatement let = s.value.let;
  assert(string_cmp(let.name.value, name));
  assert(string_cmp(let.name.token.literal, name));
}
