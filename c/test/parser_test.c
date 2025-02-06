#include "../src/parser.c"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void test_let_statements(void);
void test_let_statement(Statement s, String name);
void test_return_statements(void);
void test_identifier_expression(void);
void test_integer_literal_expression(void);

int main(void) {
  test_let_statements();
  test_return_statements();
  test_identifier_expression();
  test_integer_literal_expression();
}

void check_parser_errors(const Parser *p) {
  if (p->errors.length == 0) {
    return;
  }
  for (size_t i = 0; i < p->errors.length; ++i) {
    fprintf(stderr, "parser error: %.*s\n",
            (int)p->errors.errors[i].message.length,
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

  LetStatement let = s.data.let_statement;
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
        string_cmp(s->data.return_statement.token.literal, String("return")));
  }
}

void test_identifier_expression(void) {
  char *input = "foobar;";

  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  Lexer lexer = {0};
  lexer_init(&lexer, input);
  Parser parser = {0};
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);

  assert(program->statements_len == 1);
  assert(program->first_chunk->statements[0].type == STATEMENT_EXPRESSION);

  ExpressionStatement expression_statement =
      program->first_chunk->statements[0].data.expression_statement;

  assert(expression_statement.expression->type == EXPRESSION_IDENTIFIER);
  Identifier identifier = expression_statement.expression->data.identifier;
  assert(string_cmp(identifier.value, String("foobar")));
  assert(string_cmp(identifier.token.literal, String("foobar")));
}

void test_integer_literal_expression(void) {
  char *input = "5;";

  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  Lexer lexer = {0};
  lexer_init(&lexer, input);
  Parser parser = {0};
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);

  assert(program->statements_len == 1);
  assert(program->first_chunk->statements[0].type == STATEMENT_EXPRESSION);

  ExpressionStatement expression_statement =
      program->first_chunk->statements[0].data.expression_statement;
  assert(expression_statement.expression->type == EXPRESSION_INTEGER);

  IntegerLiteral integer_literal =
      expression_statement.expression->data.integer;
  assert(integer_literal.value == 5);
  assert(string_cmp(integer_literal.token.literal, String("5")));
}
