#include "compiler_test.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/string_builder.h"
#include "common.h"
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
  instructions_t *expected_instructions =
      instructions_flatten(3, (instructions_t *[]){
                                  make(OP_CONSTANT, 1, (ssize_t[1]){0}),
                                  make(OP_CONSTANT, 1, (ssize_t[1]){1}),
                                  make(OP_ADD, 0, NULL),
                              });

  program_t *p = program_parse("1 + 2");
  compiler_t *c = new_compiler();

  compiler_error_t err = compiler_compile_program(c, p);
  TEST_ASSERT_EQUAL_INT(COMPILERE_SUCCESS, err);

  bytecode_t *bc = compiler_bytecode(c);
  TEST_ASSERT_EQUAL_INT(expected_instructions->len, bc->instructions->len);

  for (size_t i = 0; i < expected_instructions->len; ++i) {
    TEST_ASSERT_EQUAL_UINT8(expected_instructions->arr[i],
                            bc->instructions->arr[i]);
  }

  TEST_ASSERT_EQUAL_INT(2, bc->constants_len);

  test_integer_object(bc->constants[0], 1);
  test_integer_object(bc->constants[1], 2);
}
