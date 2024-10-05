#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef enum : uint8_t {
  OP_CONSTANT,
} opcode_t;

typedef struct {
  size_t operand_widths_len;
  uint8_t *operand_widths;
  char *name;
} definition_t;

definition_t *definition_lookup(opcode_t op);

typedef struct {
  size_t len;
  size_t cap;
  uint8_t *arr;
} instructions_t;

instructions_t *make(opcode_t op, size_t operands_len, ssize_t operands[]);
char *instruction_to_string(const instructions_t *instruction);

typedef struct {
  size_t len;
  size_t *operands;
  size_t bytes_read;
} operands_t;

operands_t *read_operands(definition_t *def, uint8_t *instructions);
