#include "../src/parser.h"
#include "../src/ast.h"
#include "../src/lexer.h"
#include "parser.h"
#include "unity.h"

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
