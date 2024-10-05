#include "code_test.h"
#include "../src/code.h"
#include "unity.h"
#include <stdint.h>
#include <stdlib.h>

instructions_t *instructions_flatten(size_t n, instructions_t **instructions) {
  instructions_t *instruction = malloc(sizeof(instructions_t));
  if (!instruction) {
    return NULL;
  }

  size_t total_len = 0;
  for (size_t i = 0; i < n; ++i) {
    total_len += instructions[i]->len;
  }

  instruction->len = total_len;
  instruction->arr = calloc(total_len, sizeof(uint8_t));

  size_t index = 0;
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < instructions[i]->len; ++j) {
      instruction->arr[index] = instructions[i]->arr[j];
      index++;
    }
  }

  return instruction;
}

void test_make(void) {
  typedef struct {
    opcode_t op;
    size_t operands_len;
    ssize_t operands[1];
    size_t expected_len;
    uint8_t expected[3];
  } test_case_t;
  static const test_case_t test_cases[] = {
      {OP_CONSTANT, 1, {65534}, 3, {OP_CONSTANT, 255, 254}},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    test_case_t test_case = test_cases[i];
    instructions_t *instruction =
        make(test_case.op, test_case.operands_len, test_case.operands);

    TEST_ASSERT_EQUAL_INT(test_case.expected_len, instruction->len);
    for (size_t j = 0; j < instruction->len; ++j) {
      TEST_ASSERT_EQUAL_UINT8(test_case.expected[j], instruction->arr[j]);
    }
  }
}

void test_instructions_string(void) {
  instructions_t *instructions[] = {
      make(OP_CONSTANT, 1, (ssize_t[1]){1}),
      make(OP_CONSTANT, 1, (ssize_t[1]){2}),
      make(OP_CONSTANT, 1, (ssize_t[1]){65535}),
  };
  static const char *expected = "0000 OpConstant 1\n"
                                "0003 OpConstant 2\n"
                                "0006 OpConstant 65535\n";

  instructions_t *flattened = instructions_flatten(3, instructions);
  TEST_ASSERT_EQUAL_STRING(expected, instruction_to_string(flattened));
}

void test_read_operands(void) {
  typedef struct {
    opcode_t op;
    size_t operands_len;
    ssize_t *operands;
    size_t bytes_read;
  } test_case_t;
  test_case_t test_cases[] = {
      {OP_CONSTANT, 1, (ssize_t[1]){65535}, 2},
  };
  size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    test_case_t test_case = test_cases[i];
    instructions_t *instruction =
        make(test_case.op, test_case.operands_len, test_case.operands);

    definition_t *def = definition_lookup(test_case.op);
    TEST_ASSERT_NOT_NULL(def);

    operands_t *os = read_operands(def, instruction->arr + 1);
    TEST_ASSERT_EQUAL_INT(test_case.bytes_read, os->bytes_read);

    for (size_t j = 0; j < test_case.operands_len; ++j) {
      TEST_ASSERT_EQUAL_INT(os->operands[j], test_case.operands[j]);
    }
  }
}
