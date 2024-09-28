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
  environment_t *env = new_environment();
  return eval_program(prg, env);
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
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"-50 + 100 + -50", 0},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"20 + 2 * -10", 0},
      {"50 / 2 * 2 + 10", 60},
      {"2 * (5 + 10)", 30},
      {"3 * 3 * 3 + 10", 37},
      {"3 * (3 * 3) + 10", 37},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
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
      {"1 < 2", true},
      {"1 > 2", false},
      {"1 < 1", false},
      {"1 > 1", false},
      {"1 == 1", true},
      {"1 != 1", false},
      {"1 == 2", false},
      {"1 != 2", true},
      {"true == true", true},
      {"false == false", true},
      {"true == false", false},
      {"true != false", true},
      {"false != true", true},
      {"(1 < 2) == true", true},
      {"(1 < 2) == false", false},
      {"(1 > 2) == true", false},
      {"(1 > 2) == false", true},
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

void test_null_object(object_t *o) {
  TEST_ASSERT_EQUAL_INT(OBJECT_NULL, o->type);
}

void test_if_else_expression(void) {
  typedef struct {
    char *input;
    int64_t *expected;
  } test_case_t;
  test_case_t test_cases[] = {
      {"if (true) { 10 }", &(int64_t){10}},
      {"if (false) { 10 }", NULL},
      {"if (1) { 10 }", &(int64_t){10}},
      {"if (1 < 2) { 10 }", &(int64_t){10}},
      {"if (1 > 2) { 10 }", NULL},
      {"if (1 > 2) { 10 } else { 20 }", &(int64_t){20}},
      {"if (1 < 2) { 10 } else { 20 }", &(int64_t){10}},
  };
  size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *o = test_eval(test_cases[i].input);
    if (test_cases[i].expected == NULL) {
      test_null_object(o);
    } else {
      test_integer_object(o, *test_cases[i].expected);
    }
  }
}

void test_return_statements(void) {
  typedef struct {
    char *input;
    int64_t expected;
  } test_case_t;
  static const test_case_t test_cases[] = {{"return 10;", 10},
                                           {"return 10; 9;", 10},
                                           {"return 2 * 5; 9;", 10},
                                           {"9; return 2 * 5; 9;", 10},
                                           {"if (10 > 1) {\n"
                                            "  if (10 > 1) {\n"
                                            "    return 10;\n"
                                            "  }\n"
                                            "\n"
                                            "  return 1;\n"
                                            "}\n",
                                            10}};
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *o = test_eval(test_cases[i].input);
    test_integer_object(o, test_cases[i].expected);
  }
}

void test_error_handling(void) {
  typedef struct {
    char *input;
    char *expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {
          "5 + true;",
          "type mismatch: INTEGER + BOOLEAN",
      },
      {
          "5 + true; 5;",
          "type mismatch: INTEGER + BOOLEAN",
      },
      {
          "-true",
          "unknown operator: -BOOLEAN",
      },
      {
          "true + false;",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "5; true + false; 5",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "if (10 > 1) { true + false; }",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "if (10 > 1) {\n"
          "  if (10 > 1) {\n"
          "    return true + false;\n"
          "  }\n"
          "\n"
          "  return 1;\n"
          "}\n",
          "unknown operator: BOOLEAN + BOOLEAN",
      },
      {
          "foobar",
          "identifier not found: foobar",
      },
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    object_t *o = test_eval(test_cases[i].input);
    TEST_ASSERT_EQUAL_INT(OBJECT_ERROR, o->type);
    TEST_ASSERT_EQUAL_STRING(test_cases[i].expected, o->value.err->message);
  }
}

void test_let_statements(void) {
  typedef struct {
    char *input;
    int64_t expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    test_integer_object(test_eval(test_cases[i].input), test_cases[i].expected);
  }
}

void test_function_object(void) {
  object_t *evaluated = test_eval("fn(x) { x + 2; };");

  TEST_ASSERT_EQUAL_INT(OBJECT_FUNCTION, evaluated->type);
  TEST_ASSERT_EQUAL_INT(1, evaluated->value.fn->parameters_len);
  TEST_ASSERT_EQUAL_STRING(
      "x", identifier_to_string(evaluated->value.fn->parameters[0]));
  TEST_ASSERT_EQUAL_STRING(
      "(x + 2)", block_statement_to_string(evaluated->value.fn->body));
}

void test_function_application(void) {
  typedef struct {
    char *input;
    int64_t expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { x * 2; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
      {"fn(x) { x; }(5)", 5},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    test_integer_object(test_eval(test_cases[i].input), test_cases[i].expected);
  }
}

void test_closures(void) {
  static const char *input = "let newAdder = fn(x) {\n"
                             "  fn(y) { x + y };\n"
                             "};\n"
                             "\n"
                             "let addTwo = newAdder(2);\n"
                             "addTwo(2);\n";
  test_integer_object(test_eval(input), 4);
}
