#include "../src/parser.c"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void test_let_statements(void);
void test_let_statement(Statement s, String name);
void test_return_statements(void);

int main(void) {
  test_let_statements();
  test_return_statements();
}

void check_parser_errors(const Parser *p) {
  if (p->errors.length == 0) {
    return;
  }
  for (size_t i = 0; i < p->errors.length; ++i) {
    fprintf(stderr, "parser error: %.*s\n",
            (int)p->errors.errors[i].message.len,
            p->errors.errors[i].message.buffer);
  }
  exit(EXIT_FAILURE);
}

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
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);
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

  LetStatement let = s.value.let_statement;
  assert(string_cmp(let.name->value, name));
  assert(string_cmp(let.name->token.literal, name));
}

void test_return_statements(void) {
  char *input = "return 5;\n"
                "return 10;\n"
                "return 838383;\n";

  Arena arena = {0};
  static const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, &arena_buffer, arena_size);

  Lexer lexer = {0};
  lexer_init(&lexer, input);
  Parser parser = {0};
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);
  assert(program->statements_len == 3);

  StatementIterator iter = {0};
  statement_iterator_init(&iter, program->first_chunk);
  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    assert(s->type == STATEMENT_RETURN);
    assert(
        string_cmp(s->value.return_statement.token.literal, String("return")));
  }
}
