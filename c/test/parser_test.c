#include "parser_test.h"
#include "../src/ast.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "unity.h"
#include <stdint.h>
#include <stdlib.h>

void check_parser_errors(parser_t *p) {
  for (int i = 0; i < p->errors->len; ++i) {
    fprintf(stderr, "parser error: %s\n", (char *)p->errors->arr[i]);
  }
  TEST_ASSERT_EQUAL_INT(0, p->errors->len);
}

void test_let_statement(const statement_t *s, const char *expected_name) {
  TEST_ASSERT_EQUAL_STRING("let", statement_token_literal(s));
  TEST_ASSERT_EQUAL_INT(STATEMENT_LET, s->type);
  TEST_ASSERT_EQUAL_STRING(expected_name, s->value.let->name->value);
}

void test_parser_let_statements(void) {
  lexer_t *l = lexer_init("let x = 5;\n"
                          "let y = 10;\n"
                          "let foobar = 838383;\n"
                          "");
  parser_t *p = parser_init(l);
  TEST_ASSERT_NOT_NULL(p);

  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);
  TEST_ASSERT_NOT_NULL(prg);
  TEST_ASSERT_EQUAL_INT(3, prg->statements_len);

  static const char *tests[] = {
      "x",
      "y",
      "foobar",
  };
  for (int i = 0; i < prg->statements_len; ++i) {
    statement_t *statement = prg->statements[i];
    test_let_statement(statement, tests[i]);
  }
}

void test_parser_return_statements(void) {
  lexer_t *l = lexer_init("return 5;\n"
                          "return 10;\n"
                          "return 993322;\n"
                          "");
  parser_t *p = parser_init(l);
  TEST_ASSERT_NOT_NULL(p);

  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_NOT_NULL(prg);
  TEST_ASSERT_EQUAL_INT(3, prg->statements_len);

  for (int i = 0; i < prg->statements_len; ++i) {
    statement_t *s = prg->statements[i];
    TEST_ASSERT_EQUAL_INT(STATEMENT_RETURN, s->type);
    TEST_ASSERT_EQUAL_STRING("return", statement_token_literal(s));
  }
}

void test_parser_identifier_expression(void) {
  lexer_t *l = lexer_init("foobar;");
  parser_t *p = parser_init(l);

  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, s->expression->type);

  identifier_t *ident = s->expression->value.ident;
  TEST_ASSERT_EQUAL_STRING("foobar", ident->value);
  TEST_ASSERT_EQUAL_STRING("foobar", identifier_token_literal(ident));
}

void test_parser_integer_literal_expression(void) {
  lexer_t *l = lexer_init("5;");
  parser_t *p = parser_init(l);

  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, s->expression->type);

  integer_literal_t *integer = s->expression->value.integer;
  TEST_ASSERT_EQUAL_INT(5, integer->value);
  TEST_ASSERT_EQUAL_STRING("5", integer->token->literal);
}

void test_integer_literal(expression_t *exp, int64_t value) {
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, exp->type);

  integer_literal_t *integer = exp->value.integer;
  TEST_ASSERT_EQUAL_INT64(value, integer->value);
  char buf[sizeof(int64_t) * 8 + 1];
  sprintf(buf, "%ld", integer->value);
  TEST_ASSERT_EQUAL_STRING(buf, integer->token->literal);
}

void test_parser_prefix_expressions(void) {
  typedef struct {
    char *input;
    char *op;
    int64_t value;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {.input = "!5;", .op = "!", .value = 5},
      {.input = "-15;", .op = "-", .value = 15},
  };
  static const size_t test_cases_len = 2;
  for (size_t i = 0; i < test_cases_len; ++i) {
    lexer_t *l = lexer_init(test_cases[i].input);
    parser_t *p = parser_init(l);

    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

    expression_statement_t *s = prg->statements[0]->value.exp;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_PREFIX, s->expression->type);

    prefix_expression_t *exp = s->expression->value.prefix;
    TEST_ASSERT_EQUAL_STRING(test_cases[i].op, exp->op);

    test_integer_literal(exp->right, test_cases[i].value);
  }
}

void test_parser_infix_expressions(void) {
  typedef struct {
    char *input;
    int64_t left_value;
    char *operator;
    int64_t right_value;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"5 + 5;", 5, "+", 5},   {"5 - 5;", 5, "-", 5},   {"5 * 5;", 5, "*", 5},
      {"5 / 5;", 5, "/", 5},   {"5 > 5;", 5, ">", 5},   {"5 < 5;", 5, "<", 5},
      {"5 == 5;", 5, "==", 5}, {"5 != 5;", 5, "!=", 5},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    lexer_t *l = lexer_init(test_cases[i].input);
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

    expression_statement_t *s = prg->statements[0]->value.exp;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, s->expression->type);

    infix_expression_t *exp = s->expression->value.infix;

    test_integer_literal(exp->left, test_cases[i].left_value);
    TEST_ASSERT_EQUAL_STRING(test_cases[i].operator, exp->op);
    test_integer_literal(exp->right, test_cases[i].right_value);
  }
}
