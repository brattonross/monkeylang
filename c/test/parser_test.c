#include "../src/parser.c"
#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void test_let_statements(void);
void test_let_statement(Statement s, String name);
void test_return_statements(void);
void test_identifier_expression(void);
void test_integer_literal_expression(void);
void test_parsing_prefix_expressions(void);
void test_parsing_infix_expressions(void);
void test_operator_precedence_parsing(void);
void test_boolean_expression(void);

int main(void) {
  test_let_statements();
  test_return_statements();
  test_identifier_expression();
  test_integer_literal_expression();
  test_parsing_prefix_expressions();
  test_parsing_infix_expressions();
  test_operator_precedence_parsing();
  test_boolean_expression();
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

void test_integer_literal(Arena *arena, Expression *expression, int64_t value) {
  assert(expression->type == EXPRESSION_INTEGER);
  IntegerLiteral integer_literal = expression->data.integer;
  assert(integer_literal.value == value);
  assert(string_cmp(integer_literal.token.literal,
                    string_fmt(arena, "%" PRId64, value)));
}

void test_parsing_prefix_expressions(void) {
  struct {
    char *input;
    String op;
    int64_t integer_value;
  } test_cases[] = {
      {
          .input = "!5;",
          .op = String("!"),
          .integer_value = 5,
      },
      {
          .input = "-15;",
          .op = String("-"),
          .integer_value = 15,
      },
  };

  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);
    Program *program = parser_parse_program(&parser, &arena);
    check_parser_errors(&parser);

    assert(program->statements_len == 1);
    assert(program->current_chunk->statements[0].type == STATEMENT_EXPRESSION);
    ExpressionStatement expression_statement =
        program->current_chunk->statements[0].data.expression_statement;

    assert(expression_statement.expression->type == EXPRESSION_PREFIX);
    PrefixExpression prefix_expression =
        expression_statement.expression->data.prefix;

    assert(string_cmp(prefix_expression.op, test_cases[i].op));
    test_integer_literal(&arena, prefix_expression.right,
                         test_cases[i].integer_value);

    arena_reset(&arena);
  }
}

void test_parsing_infix_expressions(void) {
  struct {
    char *input;
    int64_t left_value;
    String op;
    int64_t right_value;
  } test_cases[] = {
      {
          "5 + 5;",
          5,
          String("+"),
          5,
      },
      {
          "5 - 5;",
          5,
          String("-"),
          5,
      },
      {
          "5 * 5;",
          5,
          String("*"),
          5,
      },
      {
          "5 / 5;",
          5,
          String("/"),
          5,
      },
      {
          "5 > 5;",
          5,
          String(">"),
          5,
      },
      {
          "5 < 5;",
          5,
          String("<"),
          5,
      },
      {
          "5 == 5;",
          5,
          String("=="),
          5,
      },
      {
          "5 != 5;",
          5,
          String("!="),
          5,
      },
  };

  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);
    Program *program = parser_parse_program(&parser, &arena);
    check_parser_errors(&parser);

    assert(program->statements_len == 1);
    assert(program->current_chunk->statements[0].type == STATEMENT_EXPRESSION);
    ExpressionStatement expression_statement =
        program->current_chunk->statements[0].data.expression_statement;

    assert(expression_statement.expression->type == EXPRESSION_INFIX);
    InfixExpression infix_expression =
        expression_statement.expression->data.infix;

    test_integer_literal(&arena, infix_expression.left,
                         test_cases[i].left_value);
    assert(string_cmp(infix_expression.op, test_cases[i].op));
    test_integer_literal(&arena, infix_expression.right,
                         test_cases[i].right_value);

    arena_reset(&arena);
  }
}

void test_operator_precedence_parsing(void) {
  struct {
    char *input;
    String expected;
  } test_cases[] = {{
                        "-a * b",
                        String("((-a) * b)"),
                    },
                    {
                        "!-a",
                        String("(!(-a))"),
                    },
                    {
                        "a + b + c",
                        String("((a + b) + c)"),
                    },
                    {
                        "a + b - c",
                        String("((a + b) - c)"),
                    },
                    {
                        "a * b * c",
                        String("((a * b) * c)"),
                    },
                    {
                        "a * b / c",
                        String("((a * b) / c)"),
                    },
                    {
                        "a + b / c",
                        String("(a + (b / c))"),
                    },
                    {
                        "a + b * c + d / e - f",
                        String("(((a + (b * c)) + (d / e)) - f)"),
                    },
                    {
                        "3 + 4; -5 * 5",
                        String("(3 + 4)((-5) * 5)"),
                    },
                    {
                        "5 > 4 == 3 < 4",
                        String("((5 > 4) == (3 < 4))"),
                    },
                    {
                        "5 < 4 != 3 > 4",
                        String("((5 < 4) != (3 > 4))"),
                    },
                    {
                        "3 + 4 * 5 == 3 * 1 + 4 * 5",
                        String("((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"),
                    }};

  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);
    Program *program = parser_parse_program(&parser, &arena);
    check_parser_errors(&parser);

    String actual = program_to_string(program, &arena);
    assert(string_cmp(actual, test_cases[i].expected));

    arena_reset(&arena);
  }
}

void test_boolean_expression_true(void) {
  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  Lexer lexer = {0};
  lexer_init(&lexer, "true;");
  Parser parser = {0};
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);

  assert(program->statements_len == 1);
  assert(program->first_chunk->statements[0].type == STATEMENT_EXPRESSION);

  ExpressionStatement expression_statement =
      program->first_chunk->statements[0].data.expression_statement;
  assert(expression_statement.expression->type == EXPRESSION_BOOLEAN);

  Boolean boolean = expression_statement.expression->data.boolean;
  assert(boolean.value == true);
  assert(string_cmp(boolean.token.literal, String("true")));
}

void test_boolean_expression_false(void) {
  Arena arena = {0};
  char arena_buffer[8192];
  arena_init(&arena, &arena_buffer, 8192);

  Lexer lexer = {0};
  lexer_init(&lexer, "false;");
  Parser parser = {0};
  parser_init(&parser, &arena, &lexer);

  Program *program = parser_parse_program(&parser, &arena);
  check_parser_errors(&parser);

  assert(program->statements_len == 1);
  assert(program->first_chunk->statements[0].type == STATEMENT_EXPRESSION);

  ExpressionStatement expression_statement =
      program->first_chunk->statements[0].data.expression_statement;
  assert(expression_statement.expression->type == EXPRESSION_BOOLEAN);

  Boolean boolean = expression_statement.expression->data.boolean;
  assert(boolean.value == false);
  assert(string_cmp(boolean.token.literal, String("false")));
}

void test_boolean_expression(void) {
  test_boolean_expression_true();
  test_boolean_expression_false();
}
