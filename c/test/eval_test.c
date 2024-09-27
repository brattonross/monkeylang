#include "eval_test.h"
#include "../src/eval.h"
#include "../src/lexer.h"
#include "../src/object.h"
#include "../src/parser.h"
#include "unity.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
      {"-5", -5},
      {"-10", -10},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *evald = test_eval(test_cases[i].input);
    test_integer_object(evald, test_cases[i].expected);
  }
}

void test_boolean_object(object_t *o, bool expected) {
  TEST_ASSERT_EQUAL_INT(OBJECT_BOOLEAN, o->type);
  TEST_ASSERT_EQUAL(expected, o->value.boolean->value);
}

void test_eval_boolean_expression(void) {
  typedef struct {
    char *input;
    bool expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"true", true},
      {"false", false},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *o = test_eval(test_cases[i].input);
    test_boolean_object(o, test_cases[i].expected);
  }
}

void test_bang_operator(void) {
  typedef struct {
    char *input;
    bool expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *o = test_eval(test_cases[i].input);
    test_boolean_object(o, test_cases[i].expected);
  }
}
