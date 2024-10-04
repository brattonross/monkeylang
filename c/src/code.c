#include "code.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

definition_t *definition_lookup(opcode_t op) {
  switch (op) {
  case OP_CONSTANT: {
    definition_t *d = malloc(sizeof(definition_t));
    if (!d) {
      return NULL;
    }

    strncpy(d->name, "OpConstant", 10);
    d->operand_widths_len = 1;
    d->operand_widths[0] = 2;
    return d;
  }
  default:
    return NULL;
  }
}

instruction_t *make(opcode_t op, size_t operands_len, size_t operands[]) {
  definition_t *def = definition_lookup(op);
  if (!def) {
    return NULL;
  }

  size_t instruction_len = 1;
  for (size_t i = 0; i < def->operand_widths_len; ++i) {
    instruction_len += def->operand_widths[i];
  }

  instruction_t *instruction = malloc(sizeof(instruction_t));
  if (!instruction) {
    return NULL;
  }

  instruction->len = instruction_len;
  instruction->arr[0] = op;

  size_t offset = 1;
  for (size_t i = 0; i < operands_len; ++i) {
    uint8_t width = def->operand_widths[i];
    switch (width) {
    case 2:
      size_t operand = operands[i];
      instruction->arr[offset] = (operand >> 8) & 0xFF;
      instruction->arr[offset + 1] = operand & 0xFF;
      break;
    }
    offset += width;
  }

  return instruction;
}
