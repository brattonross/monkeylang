#include "eval_test.h"
#include "lexer_test.h"
#include "parser_test.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();

  // lexer
  RUN_TEST(test_lexer_next_token);

  // parser
  RUN_TEST(test_parser_let_statements);
  RUN_TEST(test_parser_return_statements);
  RUN_TEST(test_parser_identifier_expression);
  RUN_TEST(test_parser_prefix_expressions);
  RUN_TEST(test_parser_infix_expressions);
  RUN_TEST(test_operator_precedence_parsing);
  RUN_TEST(test_parser_boolean_literal_expression);
  RUN_TEST(test_parser_if_expression);
  RUN_TEST(test_parser_if_else_expression);
  RUN_TEST(test_parser_function_literal_parsing);
  RUN_TEST(test_parser_function_parameter_parsing);
  RUN_TEST(test_parser_call_expression_parsing);
  RUN_TEST(test_string_literal_expression);

  // eval
  RUN_TEST(test_eval_integer_expression);
  RUN_TEST(test_eval_boolean_expression);
  RUN_TEST(test_bang_operator);
  RUN_TEST(test_if_else_expression);
  RUN_TEST(test_return_statements);
  RUN_TEST(test_error_handling);
  RUN_TEST(test_let_statements);
  RUN_TEST(test_function_object);
  RUN_TEST(test_function_application);
  RUN_TEST(test_closures);

  return UNITY_END();
}
