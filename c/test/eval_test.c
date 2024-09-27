#include "eval_test.h"
#include "../src/eval.h"
#include "../src/lexer.h"
#include "../src/object.h"
#include "../src/parser.h"
#include "unity.h"
#include <stdint.h>

object_t *test_eval(const char *input) {
  lexer_t *l = lexer_init(input);
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  return eval_program(prg);
}

void test_integer_object(object_t *o, int64_t expected) {
  TEST_ASSERT_EQUAL_INT(OBJECT_INTEGER, o->type);
  TEST_ASSERT_EQUAL_INT(expected, o->value.integer->value);
}

void test_eval_integer_expression(void) {
  typedef struct {
    char *input;
    int64_t expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"5", 5},
      {"10", 10},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *evald = test_eval(test_cases[i].input);
    test_integer_object(evald, test_cases[i].expected);
  }
}
