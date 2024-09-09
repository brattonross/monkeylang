#include "ast_test.h"
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
  RUN_TEST(test_program_to_string);
  RUN_TEST(test_parser_identifier_expression);

  return UNITY_END();
}
