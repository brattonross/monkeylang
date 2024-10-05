#include "common.h"
#include "unity.h"

void test_integer_object(object_t *o, int64_t expected) {
  TEST_ASSERT_EQUAL_INT(OBJECT_INTEGER, o->type);
  TEST_ASSERT_EQUAL_INT(expected, o->value.integer->value);
}

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
