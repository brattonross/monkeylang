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
    ssize_t *operands;
    size_t expected_len;
    uint8_t *expected;
  } test_case_t;
  test_case_t test_cases[] = {
      {OP_CONSTANT, 1, (ssize_t[1]){65534}, 3,
       (uint8_t[3]){OP_CONSTANT, 255, 254}},
      {OP_ADD, 0, NULL, 1, (uint8_t[1]){OP_ADD}},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    instructions_t *instructions = make(
        test_cases[i].op, test_cases[i].operands_len, test_cases[i].operands);

    TEST_ASSERT_EQUAL_INT(test_cases[i].expected_len, instructions->len);
    for (size_t j = 0; j < instructions->len; ++j) {
      TEST_ASSERT_EQUAL_UINT8(test_cases[i].expected[j], instructions->arr[j]);
    }
  }
}

void test_instructions_string(void) {
  instructions_t *instructions[] = {
      make(OP_ADD, 0, NULL),
      make(OP_CONSTANT, 1, (ssize_t[1]){2}),
      make(OP_CONSTANT, 1, (ssize_t[1]){65535}),
  };
  static const char *expected = "0000 OpAdd\n"
                                "0001 OpConstant 2\n"
                                "0004 OpConstant 65535\n";

  instructions_t *flattened = instructions_flatten(
      sizeof(instructions) / sizeof(*instructions), instructions);
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
    TEST_ASSERT_NOT_NULL(os);
    TEST_ASSERT_EQUAL_INT(test_case.bytes_read, os->bytes_read);

    for (size_t j = 0; j < test_case.operands_len; ++j) {
      TEST_ASSERT_EQUAL_INT(os->operands[j], test_case.operands[j]);
    }
  }
}
