#include "ast_test.h"
#include "../src/ast.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "unity.h"

void test_program_to_string(void) {
  lexer_t *l = lexer_init("let myVar = anotherVar;");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);

  const char *actual = program_to_string(prg);
  TEST_ASSERT_EQUAL_STRING("let myVar = anotherVar;", actual);
}
