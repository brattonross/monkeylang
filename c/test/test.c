#include "lexer.h"
#include "parser.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_lexer_next_token);
  RUN_TEST(test_parser_let_statements);

  return UNITY_END();
}
