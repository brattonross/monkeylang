#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef enum : uint8_t {
  OP_CONSTANT,
} opcode_t;

typedef struct {
  size_t operand_widths_len;
  uint8_t operand_widths[8];
  char name[32];
} definition_t;

definition_t *definition_lookup(opcode_t op);

typedef struct {
  size_t len;
  uint8_t arr[256];
} instruction_t;

instruction_t *make(opcode_t op, size_t operands_len, size_t operands[]);
