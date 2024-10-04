#include "code_test.h"
#include "../src/code.h"
#include "unity.h"
#include <stdint.h>
#include <stdlib.h>

void test_make(void) {
  typedef struct {
    opcode_t op;
    size_t operands_len;
    size_t operands[1];
    size_t expected_len;
    uint8_t expected[3];
  } test_case_t;
  static const test_case_t test_cases[] = {
      {OP_CONSTANT, 1, {65534}, 3, {OP_CONSTANT, 255, 254}},
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    test_case_t test_case = test_cases[i];
    instruction_t *instruction =
        make(test_case.op, test_case.operands_len, test_case.operands);

    TEST_ASSERT_EQUAL_INT(test_case.expected_len, instruction->len);
    for (size_t j = 0; j < instruction->len; ++j) {
      TEST_ASSERT_EQUAL_UINT8(test_case.expected[j], instruction->arr[j]);
    }
  }
}
