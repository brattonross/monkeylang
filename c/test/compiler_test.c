#include "compiler_test.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/string_builder.h"
#include "unity.h"
#include <stdint.h>
#include <stdio.h>

void compiler_debug(const compiler_t *c) {
  string_builder_t *sb = new_string_builder();

  sb_append(sb, "{\n");
  sb_appendf(sb, "\t.constants_cap = %zu,\n", c->constants_cap);
  sb_appendf(sb, "\t.constants_len = %zu,\n", c->constants_len);
  sb_append(sb, "\t.constants = [\n");
  for (size_t i = 0; i < c->constants_len; ++i) {
    sb_appendf(sb, "\t\t[%zu] = %s,\n", i, object_inspect(c->constants[i]));
  }
  sb_append(sb, "\t],\n");
  sb_appendf(sb, "\t.instructions->cap = %zu,\n", c->instructions->cap);
  sb_appendf(sb, "\t.instructions->len = %zu,\n", c->instructions->len);
  sb_append(sb, "\t.instructions = [\n");
  for (size_t i = 0; i < c->instructions->len; ++i) {
    sb_appendf(sb, "\t\t[%zu] = %zu,\n", i, c->instructions->arr[i]);
  }
  sb_append(sb, "\t],\n");
  sb_append(sb, "}\n");

  puts(sb->buf);
  free(sb->buf);
  free(sb);
}

program_t *program_parse(const char *input) {
  lexer_t *l = lexer_init(input);
  parser_t *p = parser_init(l);
  return parser_parse_program(p);
}

void test_integer_arithmetic(void) {
  program_t *p = program_parse("1 + 2");
  compiler_t *c = new_compiler();

  compiler_error_t err = compiler_compile_program(c, p);
  TEST_ASSERT_EQUAL_INT(COMPILER_SUCCESS, err);

  bytecode_t *bc = compiler_bytecode(c);
  TEST_ASSERT_EQUAL_INT(6, bc->instructions->len);

  ssize_t ops1[1] = {0};
  instructions_t *i1 = make(OP_CONSTANT, 1, ops1);
  ssize_t ops2[1] = {1};
  instructions_t *i2 = make(OP_CONSTANT, 1, ops2);

  uint8_t expected_instructions[6];
  size_t expected_instructions_len = 0;
  for (size_t i = 0; i < i1->len; ++i) {
    expected_instructions[i] = i1->arr[i];
    expected_instructions_len++;
  }
  for (size_t i = i2->len; i < i2->len; ++i) {
    expected_instructions[i + i1->len] = i2->arr[i];
    expected_instructions_len++;
  }

  for (size_t i = 0; i < expected_instructions_len; ++i) {
    TEST_ASSERT_EQUAL_UINT8(expected_instructions[i], bc->instructions->arr[i]);
  }

  TEST_ASSERT_EQUAL_INT(2, bc->constants_len);

  TEST_ASSERT_EQUAL_INT(OBJECT_INTEGER, bc->constants[0]->type);
  TEST_ASSERT_EQUAL_INT(1, bc->constants[0]->value.integer->value);

  TEST_ASSERT_EQUAL_INT(OBJECT_INTEGER, bc->constants[1]->type);
  TEST_ASSERT_EQUAL_INT(2, bc->constants[1]->value.integer->value);
}
