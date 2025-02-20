#include "../src/eval.c"
#include "../src/lexer.c"
#include "../src/mem.c"
#include "../src/object.c"
#include "../src/parser.c"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void test_eval_integer_expression(void);
void test_eval_boolean_expression(void);
void test_bang_operator(void);
void test_if_else_expressions(void);
void test_return_statements(void);
void test_error_handling(void);

int main(void) {
  test_eval_integer_expression();
  test_eval_boolean_expression();
  test_bang_operator();
  test_if_else_expressions();
  test_return_statements();
  test_error_handling();
}

void test_eval_integer_expression(void) {
  struct {
    char *input;
    int64_t expected;
  } test_cases[] = {
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

  Arena arena = {0};
  const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.type == OBJECT_INTEGER);
    assert(evaluated.data.integer_object.value == test_cases[i].expected);

    arena_reset(&arena);
  }
}

void test_eval_boolean_expression(void) {
  struct {
    char *input;
    bool expected;
  } test_cases[] = {
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

  Arena arena = {0};
  const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.type == OBJECT_BOOLEAN);
    assert(evaluated.data.boolean_object.value == test_cases[i].expected);

    arena_reset(&arena);
  }
}

void test_bang_operator(void) {
  struct {
    char *input;
    bool expected;
  } test_cases[] = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };

  Arena arena = {0};
  const size_t arena_size = 8192;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.type == OBJECT_BOOLEAN);
    assert(evaluated.data.boolean_object.value == test_cases[i].expected);

    arena_reset(&arena);
  }
}

void test_if_else_expressions(void) {
  struct {
    char *input;
    ObjectType expected_type;
    int64_t expected_value;
  } test_cases[] = {
      {"if (true) { 10 }", OBJECT_INTEGER, 10},
      {"if (false) { 10 }", OBJECT_NULL, 0},
      {"if (1) { 10 }", OBJECT_INTEGER, 10},
      {"if (1 < 2) { 10 }", OBJECT_INTEGER, 10},
      {"if (1 > 2) { 10 }", OBJECT_NULL, 0},
      {"if (1 > 2) { 10 } else { 20 }", OBJECT_INTEGER, 20},
      {"if (1 < 2) { 10 } else { 20 }", OBJECT_INTEGER, 10},
  };

  Arena arena = {0};
  const size_t arena_size = 16 * 1024;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.type == test_cases[i].expected_type);

    switch (test_cases[i].expected_type) {
    case OBJECT_INTEGER:
      assert(evaluated.data.integer_object.value ==
             test_cases[i].expected_value);
      break;
    case OBJECT_NULL:
      // no need to check value :)
      break;
    default:
      fprintf(stderr, "unhandled if-else expression type %.*s\n",
              (int)expression_type_strings[test_cases[i].expected_type].length,
              expression_type_strings[test_cases[i].expected_type].buffer);
      exit(EXIT_FAILURE);
    }

    arena_reset(&arena);
  }
}

void test_return_statements(void) {
  struct {
    char *input;
    int64_t expected_value;
  } test_cases[] = {
      {"return 10;", 10},
      {"return 10; 9;", 10},
      {"return 2 * 5; 9;", 10},
      {"9; return 2 * 5; 9;", 10},
      {
          "if (10 > 1) {\n"
          "  if (10 > 1) {\n"
          "    return 10;\n"
          "  }\n"
          "\n"
          "  return 1;\n"
          "}\n",
          10,
      },
  };

  Arena arena = {0};
  const size_t arena_size = 16 * 1024;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.data.integer_object.value == test_cases[i].expected_value);

    arena_reset(&arena);
  }
}

void test_error_handling(void) {
  struct {
    char *input;
    String expected_message;
  } test_cases[] = {
      {"5 + true;", String("type mismatch: INTEGER + BOOLEAN")},
      {"5 + true; 5;", String("type mismatch: INTEGER + BOOLEAN")},
      {"-true", String("unknown operator: -BOOLEAN")},
      {"true + false;", String("unknown operator: BOOLEAN + BOOLEAN")},
      {"5; true + false; 5", String("unknown operator: BOOLEAN + BOOLEAN")},
      {
          "if (10 > 1) { true + false; }",
          String("unknown operator: BOOLEAN + BOOLEAN"),
      },
      {
          "if (10 > 1) {\n"
          "  if (10 > 1) {\n"
          "    return true + false;\n"
          "  }\n"
          "\n"
          "  return 1;\n"
          "}\n",
          String("unknown operator: BOOLEAN + BOOLEAN"),
      },
  };

  Arena arena = {0};
  const size_t arena_size = 16 * 1024;
  char arena_buffer[arena_size];
  arena_init(&arena, arena_buffer, arena_size);

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[i]); ++i) {
    Lexer lexer = {0};
    lexer_init(&lexer, test_cases[i].input);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    assert(evaluated.type == OBJECT_ERROR);
    assert(string_cmp(evaluated.data.error_object.message,
                      test_cases[i].expected_message));

    arena_reset(&arena);
  }
}
