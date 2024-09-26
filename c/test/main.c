#include "lexer_test.h"
#include "parser_test.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_lexer_next_token);
  RUN_TEST(test_parser_let_statements);
  RUN_TEST(test_parser_return_statements);
  RUN_TEST(test_parser_identifier_expression);
  RUN_TEST(test_parser_prefix_expressions);
  RUN_TEST(test_parser_infix_expressions);
  RUN_TEST(test_operator_precedence_parsing);

  return UNITY_END();
}
