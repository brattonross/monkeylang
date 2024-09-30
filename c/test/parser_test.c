#include "parser_test.h"
#include "../src/ast.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "unity.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void check_parser_errors(parser_t *p) {
  for (size_t i = 0; i < p->errors->len; ++i) {
    fprintf(stderr, "parser error: %s\n", (char *)p->errors->arr[i]);
  }
  TEST_ASSERT_EQUAL_INT(0, p->errors->len);
}

void test_parser_let_statements(void) {
  {
    lexer_t *l = lexer_init("let x = 5;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_LET, prg->statements[0]->type);
    TEST_ASSERT_EQUAL_STRING("x", prg->statements[0]->value.let->ident->value);

    expression_t *value = prg->statements[0]->value.let->value;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, value->type);
    TEST_ASSERT_EQUAL_INT(5, value->value.integer->value);
  }

  {
    lexer_t *l = lexer_init("let y = true;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_LET, prg->statements[0]->type);
    TEST_ASSERT_EQUAL_STRING("y", prg->statements[0]->value.let->ident->value);

    expression_t *value = prg->statements[0]->value.let->value;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_BOOLEAN_LITERAL, value->type);
    TEST_ASSERT_EQUAL(true, value->value.boolean->value);
  }

  {
    lexer_t *l = lexer_init("let foobar = y;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_LET, prg->statements[0]->type);
    TEST_ASSERT_EQUAL_STRING("foobar",
                             prg->statements[0]->value.let->ident->value);

    expression_t *value = prg->statements[0]->value.let->value;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, value->type);
    TEST_ASSERT_EQUAL_STRING("y", value->value.ident->value);
  }
}

void test_parser_return_statements(void) {
  {
    lexer_t *l = lexer_init("return 5;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_RETURN, prg->statements[0]->type);

    return_statement_t *ret = prg->statements[0]->value.ret;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, ret->value->type);
    TEST_ASSERT_EQUAL_INT(5, ret->value->value.integer->value);
  }

  {
    lexer_t *l = lexer_init("return true;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_RETURN, prg->statements[0]->type);

    return_statement_t *ret = prg->statements[0]->value.ret;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_BOOLEAN_LITERAL, ret->value->type);
    TEST_ASSERT_EQUAL(true, ret->value->value.boolean->value);
  }

  {
    lexer_t *l = lexer_init("return foo;");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_RETURN, prg->statements[0]->type);

    return_statement_t *ret = prg->statements[0]->value.ret;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, ret->value->type);
    TEST_ASSERT_EQUAL_STRING("foo", ret->value->value.ident->value);
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

void test_boolean_literal(expression_t *exp, bool value) {
  TEST_ASSERT_EQUAL(EXPRESSION_BOOLEAN_LITERAL, exp->type);

  boolean_literal_t *boolean = exp->value.boolean;
  TEST_ASSERT_EQUAL(value, boolean->value);
  char *buf = boolean->value ? "true" : "false";
  TEST_ASSERT_EQUAL_STRING(buf, boolean->token->literal);
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
    expression_type_t value_type;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"5 + 5;", 5, "+", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 - 5;", 5, "-", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 * 5;", 5, "*", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 / 5;", 5, "/", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 > 5;", 5, ">", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 < 5;", 5, "<", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 == 5;", 5, "==", 5, EXPRESSION_INTEGER_LITERAL},
      {"5 != 5;", 5, "!=", 5, EXPRESSION_INTEGER_LITERAL},
      {"true == true", true, "==", true, EXPRESSION_BOOLEAN_LITERAL},
      {"true != false", true, "!=", false, EXPRESSION_BOOLEAN_LITERAL},
      {"false == false", false, "==", false, EXPRESSION_BOOLEAN_LITERAL},
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

    TEST_ASSERT_EQUAL_STRING(test_cases[i].operator, exp->op);

    if (test_cases[i].value_type == EXPRESSION_BOOLEAN_LITERAL) {
      test_boolean_literal(exp->left, test_cases[i].left_value);
      test_boolean_literal(exp->right, test_cases[i].right_value);
    } else {
      test_integer_literal(exp->left, test_cases[i].left_value);
      test_integer_literal(exp->right, test_cases[i].right_value);
    }
  }
}

void test_operator_precedence_parsing(void) {
  typedef struct {
    char *input;
    char *expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {
          "-a * b",
          "((-a) * b)",
      },
      {
          "!-a",
          "(!(-a))",
      },
      {
          "a + b + c",
          "((a + b) + c)",
      },
      {
          "a + b - c",
          "((a + b) - c)",
      },
      {
          "a * b * c",
          "((a * b) * c)",
      },
      {
          "a * b / c",
          "((a * b) / c)",
      },
      {
          "a + b / c",
          "(a + (b / c))",
      },
      {
          "a + b * c + d / e - f",
          "(((a + (b * c)) + (d / e)) - f)",
      },
      {
          "3 + 4; -5 * 5",
          "(3 + 4)((-5) * 5)",
      },
      {
          "5 > 4 == 3 < 4",
          "((5 > 4) == (3 < 4))",
      },
      {
          "5 < 4 != 3 > 4",
          "((5 < 4) != (3 > 4))",
      },
      {
          "3 + 4 * 5 == 3 * 1 + 4 * 5",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
      },
      {
          "true",
          "true",
      },
      {
          "false",
          "false",
      },
      {
          "3 > 5 == false",
          "((3 > 5) == false)",
      },
      {
          "3 < 5 == true",
          "((3 < 5) == true)",
      },
      {
          "1 + (2 + 3) + 4",
          "((1 + (2 + 3)) + 4)",
      },
      {
          "(5 + 5) * 2",
          "((5 + 5) * 2)",
      },
      {
          "2 / (5 + 5)",
          "(2 / (5 + 5))",
      },
      {
          "-(5 + 5)",
          "(-(5 + 5))",
      },
      {
          "!(true == true)",
          "(!(true == true))",
      },
      {
          "a + add(b * c) + d",
          "((a + add((b * c))) + d)",
      },
      {
          "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
          "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
      },
      {
          "add(a + b + c * d / f + g)",
          "add((((a + b) + ((c * d) / f)) + g))",
      },
      {
          "a * [1, 2, 3, 4][b * c] * d",
          "((a * ([1, 2, 3, 4][(b * c)])) * d)",
      },
      {
          "add(a * b[2], b[1], 2 * [1, 2][1])",
          "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))",
      },
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    lexer_t *l = lexer_init(test_cases[i].input);
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_STRING(test_cases[i].expected, program_to_string(prg));
  }
}

void test_parser_boolean_literal_expression(void) {
  lexer_t *l = lexer_init("true;");
  parser_t *p = parser_init(l);

  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_BOOLEAN_LITERAL, s->expression->type);

  boolean_literal_t *boolean = s->expression->value.boolean;
  TEST_ASSERT_EQUAL(true, boolean->value);
  TEST_ASSERT_EQUAL_STRING("true", boolean->token->literal);
}

void test_parser_if_expression(void) {
  lexer_t *l = lexer_init("if (x < y) { x }");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IF, s->expression->type);

  if_expression_t *exp = s->expression->value.if_;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, exp->condition->type);

  expression_t *left = exp->condition->value.infix->left;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, left->type);
  TEST_ASSERT_EQUAL_STRING("x", left->value.ident->value);

  TEST_ASSERT_EQUAL_STRING("<", exp->condition->value.infix->op);

  expression_t *right = exp->condition->value.infix->right;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, right->type);
  TEST_ASSERT_EQUAL_STRING("y", right->value.ident->value);

  TEST_ASSERT_EQUAL_INT(1, exp->consequence->statements_len);
  expression_statement_t *consequence =
      exp->consequence->statements[0]->value.exp;

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, consequence->expression->type);
  TEST_ASSERT_EQUAL_STRING("x", consequence->expression->value.ident->value);

  TEST_ASSERT_NULL(exp->alternative);
}

void test_parser_if_else_expression(void) {
  lexer_t *l = lexer_init("if (x < y) { x } else { y }");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IF, s->expression->type);

  if_expression_t *exp = s->expression->value.if_;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, exp->condition->type);

  expression_t *left = exp->condition->value.infix->left;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, left->type);
  TEST_ASSERT_EQUAL_STRING("x", left->value.ident->value);

  TEST_ASSERT_EQUAL_STRING("<", exp->condition->value.infix->op);

  expression_t *right = exp->condition->value.infix->right;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, right->type);
  TEST_ASSERT_EQUAL_STRING("y", right->value.ident->value);

  TEST_ASSERT_EQUAL_INT(1, exp->consequence->statements_len);
  expression_statement_t *consequence =
      exp->consequence->statements[0]->value.exp;

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, consequence->expression->type);
  TEST_ASSERT_EQUAL_STRING("x", consequence->expression->value.ident->value);

  TEST_ASSERT_EQUAL_INT(1, exp->alternative->statements_len);
  expression_statement_t *alternative =
      exp->alternative->statements[0]->value.exp;

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, alternative->expression->type);
  TEST_ASSERT_EQUAL_STRING("y", alternative->expression->value.ident->value);
}

void test_parser_function_literal_parsing(void) {
  lexer_t *l = lexer_init("fn(x, y) { x + y; }");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_FUNCTION_LITERAL, s->expression->type);

  function_literal_t *fn = s->expression->value.fn;
  TEST_ASSERT_EQUAL_INT(2, fn->parameters_len);
  TEST_ASSERT_EQUAL_STRING("x", fn->parameters[0]->value);
  TEST_ASSERT_EQUAL_STRING("y", fn->parameters[1]->value);

  TEST_ASSERT_EQUAL_INT(1, fn->body->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, fn->body->statements[0]->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX,
                        fn->body->statements[0]->value.exp->expression->type);
  infix_expression_t *infix =
      fn->body->statements[0]->value.exp->expression->value.infix;

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, infix->left->type);
  TEST_ASSERT_EQUAL_STRING("x", infix->left->value.ident->value);

  TEST_ASSERT_EQUAL_STRING("+", infix->op);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, infix->right->type);
  TEST_ASSERT_EQUAL_STRING("y", infix->right->value.ident->value);
}

void test_parser_function_parameter_parsing(void) {
  {
    lexer_t *l = lexer_init("fn() {};");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

    expression_statement_t *s = prg->statements[0]->value.exp;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_FUNCTION_LITERAL, s->expression->type);

    function_literal_t *fn = s->expression->value.fn;
    TEST_ASSERT_EQUAL_INT(0, fn->parameters_len);
    TEST_ASSERT_NULL(fn->parameters);
  }

  {
    lexer_t *l = lexer_init("fn(x) {};");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

    expression_statement_t *s = prg->statements[0]->value.exp;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_FUNCTION_LITERAL, s->expression->type);

    function_literal_t *fn = s->expression->value.fn;
    TEST_ASSERT_EQUAL_INT(1, fn->parameters_len);
    TEST_ASSERT_EQUAL_STRING("x", fn->parameters[0]->value);
  }

  {
    lexer_t *l = lexer_init("fn(x, y, z) {};");
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);
    check_parser_errors(p);

    TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
    TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

    expression_statement_t *s = prg->statements[0]->value.exp;
    TEST_ASSERT_EQUAL_INT(EXPRESSION_FUNCTION_LITERAL, s->expression->type);

    function_literal_t *fn = s->expression->value.fn;
    TEST_ASSERT_EQUAL_INT(3, fn->parameters_len);
    TEST_ASSERT_EQUAL_STRING("x", fn->parameters[0]->value);
    TEST_ASSERT_EQUAL_STRING("y", fn->parameters[1]->value);
    TEST_ASSERT_EQUAL_STRING("z", fn->parameters[2]->value);
  }
}

void test_parser_call_expression_parsing(void) {
  lexer_t *l = lexer_init("add(1, 2 * 3, 4 + 5);");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_CALL, s->expression->type);

  call_expression_t *call = s->expression->value.call;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, call->fn->type);
  TEST_ASSERT_EQUAL_STRING("add", call->fn->value.ident->value);
  TEST_ASSERT_EQUAL_INT(3, call->argc);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, call->argv[0]->type);
  TEST_ASSERT_EQUAL_INT(1, call->argv[0]->value.integer->value);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, call->argv[1]->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL,
                        call->argv[1]->value.infix->left->type);
  TEST_ASSERT_EQUAL_INT(2,
                        call->argv[1]->value.infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("*", call->argv[1]->value.infix->op);
  TEST_ASSERT_EQUAL_INT(
      3, call->argv[1]->value.infix->right->value.integer->value);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, call->argv[2]->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL,
                        call->argv[2]->value.infix->left->type);
  TEST_ASSERT_EQUAL_INT(4,
                        call->argv[2]->value.infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("+", call->argv[2]->value.infix->op);
  TEST_ASSERT_EQUAL_INT(
      5, call->argv[2]->value.infix->right->value.integer->value);
}

void test_string_literal_expression(void) {
  lexer_t *l = lexer_init("\"hello world\";");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *exp = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_STRING, exp->expression->type);

  string_literal_t *str = exp->expression->value.string;
  TEST_ASSERT_EQUAL_STRING("hello world", str->value);
}

void test_parsing_array_literals(void) {
  lexer_t *l = lexer_init("[1, 2 * 2, 3 + 3]");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *exp = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_ARRAY_LITERAL, exp->expression->type);

  array_literal_t *arr = exp->expression->value.arr;
  TEST_ASSERT_EQUAL_INT(3, arr->len);

  test_integer_literal(arr->elements[0], 1);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, arr->elements[1]->type);
  infix_expression_t *el1 = arr->elements[1]->value.infix;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, el1->left->type);
  TEST_ASSERT_EQUAL_INT(2, el1->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("*", el1->op);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, el1->right->type);
  TEST_ASSERT_EQUAL_INT(2, el1->right->value.integer->value);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, arr->elements[2]->type);
  infix_expression_t *el2 = arr->elements[2]->value.infix;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, el2->left->type);
  TEST_ASSERT_EQUAL_INT(3, el2->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("+", el2->op);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, el2->right->type);
  TEST_ASSERT_EQUAL_INT(3, el2->right->value.integer->value);
}

void test_parsing_index_expressions(void) {
  lexer_t *l = lexer_init("myArray[1 + 1]");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *s = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INDEX, s->expression->type);

  index_expression_t *index = s->expression->value.index;

  TEST_ASSERT_EQUAL_INT(EXPRESSION_IDENTIFIER, index->left->type);
  TEST_ASSERT_EQUAL_STRING("myArray", index->left->value.ident->value);

  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, index->index->type);
  infix_expression_t *infix = index->index->value.infix;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, infix->left->type);
  TEST_ASSERT_EQUAL_INT(1, infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("+", infix->op);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL, infix->right->type);
  TEST_ASSERT_EQUAL_INT(1, infix->right->value.integer->value);
}

void test_parsing_hash_literals_string_keys(void) {
  lexer_t *l = lexer_init("{\"one\": 1, \"two\": 2, \"three\": 3}");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *exp = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_HASH, exp->expression->type);

  hash_literal_t *hash = exp->expression->value.hash;
  TEST_ASSERT_EQUAL_INT(3, hash->len);

  TEST_ASSERT_EQUAL_STRING("one", hash->pairs[0]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(1, hash->pairs[0]->value->value.integer->value);

  TEST_ASSERT_EQUAL_STRING("two", hash->pairs[1]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(2, hash->pairs[1]->value->value.integer->value);

  TEST_ASSERT_EQUAL_STRING("three", hash->pairs[2]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(3, hash->pairs[2]->value->value.integer->value);
}

void test_parsing_empty_hash_literal(void) {
  lexer_t *l = lexer_init("{}");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *exp = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_HASH, exp->expression->type);

  hash_literal_t *hash = exp->expression->value.hash;
  TEST_ASSERT_EQUAL_INT(0, hash->len);
}

void test_parsing_hash_literals_with_expressions(void) {
  lexer_t *l =
      lexer_init("{\"one\": 0 + 1, \"two\": 10 - 8, \"three\": 15 / 5}");
  parser_t *p = parser_init(l);
  program_t *prg = parser_parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL_INT(1, prg->statements_len);
  TEST_ASSERT_EQUAL_INT(STATEMENT_EXPRESSION, prg->statements[0]->type);

  expression_statement_t *exp = prg->statements[0]->value.exp;
  TEST_ASSERT_EQUAL_INT(EXPRESSION_HASH, exp->expression->type);

  hash_literal_t *hash = exp->expression->value.hash;
  TEST_ASSERT_EQUAL_INT(3, hash->len);

  TEST_ASSERT_EQUAL_STRING("one", hash->pairs[0]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, hash->pairs[0]->value->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL,
                        hash->pairs[0]->value->value.infix->left->type);
  TEST_ASSERT_EQUAL_INT(
      0, hash->pairs[0]->value->value.infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("+", hash->pairs[0]->value->value.infix->op);
  TEST_ASSERT_EQUAL_INT(
      1, hash->pairs[0]->value->value.infix->right->value.integer->value);

  TEST_ASSERT_EQUAL_STRING("two", hash->pairs[1]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, hash->pairs[1]->value->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL,
                        hash->pairs[1]->value->value.infix->left->type);
  TEST_ASSERT_EQUAL_INT(
      10, hash->pairs[1]->value->value.infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("-", hash->pairs[1]->value->value.infix->op);
  TEST_ASSERT_EQUAL_INT(
      8, hash->pairs[1]->value->value.infix->right->value.integer->value);

  TEST_ASSERT_EQUAL_STRING("three", hash->pairs[2]->key->value.string->value);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INFIX, hash->pairs[2]->value->type);
  TEST_ASSERT_EQUAL_INT(EXPRESSION_INTEGER_LITERAL,
                        hash->pairs[2]->value->value.infix->left->type);
  TEST_ASSERT_EQUAL_INT(
      15, hash->pairs[2]->value->value.infix->left->value.integer->value);
  TEST_ASSERT_EQUAL_STRING("/", hash->pairs[2]->value->value.infix->op);
  TEST_ASSERT_EQUAL_INT(
      5, hash->pairs[2]->value->value.infix->right->value.integer->value);
}
