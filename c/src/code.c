#include "code.h"
#include "string_builder.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *op_constant_name = "OpConstant";
static const char *op_add_name = "OpAdd";

definition_t *definition_lookup(opcode_t op) {
  switch (op) {
  case OP_CONSTANT: {
    definition_t *d = malloc(sizeof(definition_t));
    if (!d) {
      return NULL;
    }

    d->name = malloc(strlen(op_constant_name) + 1);
    if (!d->name) {
      free(d);
      return NULL;
    }
    strncpy(d->name, op_constant_name, strlen(op_constant_name));

    d->operand_widths_len = 1;
    d->operand_widths = calloc(d->operand_widths_len, sizeof(uint8_t));
    if (!d->operand_widths) {
      free(d->name);
      free(d->operand_widths);
      free(d);
      return NULL;
    }
    d->operand_widths[0] = 2;
    return d;
  }
  case OP_ADD: {
    definition_t *d = malloc(sizeof(definition_t));
    if (!d) {
      return NULL;
    }

    d->name = malloc(strlen(op_add_name) + 1);
    if (!d->name) {
      free(d);
      return NULL;
    }
    strncpy(d->name, op_add_name, strlen(op_add_name));

    d->operand_widths = NULL;
    d->operand_widths_len = 0;

    return d;
  }
  default:
    return NULL;
  }
}

void write_uint16_big_endian(uint8_t *buf, uint16_t value) {
  buf[0] = (value >> 8) & 0xFF;
  buf[1] = value & 0xFF;
}

uint16_t read_uint16_big_endian(const uint8_t *buf) {
  return ((uint16_t)buf[0] << 8) | buf[1];
}

instructions_t *make(opcode_t op, size_t operands_len, ssize_t *operands) {
  definition_t *def = definition_lookup(op);
  if (!def) {
    return NULL;
  }

  size_t instruction_len = 1;
  for (size_t i = 0; i < def->operand_widths_len; ++i) {
    instruction_len += def->operand_widths[i];
  }

  instructions_t *instruction = malloc(sizeof(instructions_t));
  if (!instruction) {
    return NULL;
  }

  instruction->cap = 16;
  while (instruction->cap < instruction_len) {
    instruction->cap *= 2;
  }
  instruction->len = instruction_len;
  instruction->arr = calloc(instruction_len, sizeof(uint8_t));
  if (!instruction->arr) {
    free(instruction);
    return NULL;
  }
  instruction->arr[0] = op;

  size_t offset = 1;
  for (size_t i = 0; i < operands_len; ++i) {
    uint8_t width = def->operand_widths[i];
    switch (width) {
    case 2:
      size_t operand = operands[i];
      write_uint16_big_endian(instruction->arr + offset, operand);
      break;
    }
    offset += width;
  }

  return instruction;
}

char *format_instruction(const instructions_t *instruction,
                         const definition_t *def, const operands_t *operands) {
  if (operands->len != def->operand_widths_len) {
    size_t size =
        snprintf(NULL, 0, "ERROR: operand len %zu does not match defined %zu\n",
                 operands->len, def->operand_widths_len);
    char *buf = malloc(size);
    snprintf(buf, size, "ERROR: operand len %zu does not match defined %zu\n",
             operands->len, def->operand_widths_len);
    return buf;
  }

  switch (def->operand_widths_len) {
  case 0:
    return strdup(def->name);
  case 1:
    size_t size = snprintf(NULL, 0, "%s %zu", def->name, operands->operands[0]);
    char *buf = malloc(size + 1);
    snprintf(buf, size + 1, "%s %zu", def->name, operands->operands[0]);
    return buf;
  }

  size_t size =
      snprintf(NULL, 0, "ERROR: unhandled operand count for %s\n", def->name);
  char *buf = malloc(size);
  snprintf(buf, size, "ERROR: unhandled operand count for %s\n", def->name);
  return buf;
}

char *instruction_to_string(const instructions_t *instructions) {
  string_builder_t *sb = new_string_builder();

  size_t i = 0;
  while (i < instructions->len) {
    definition_t *def = definition_lookup(instructions->arr[i]);
    if (!def) {
      sb_appendf(sb, "ERROR: opcode %d undefined\n", instructions->arr[i]);
      continue;
    }

    operands_t *operands = read_operands(def, instructions->arr + i + 1);
    char *ins_str = format_instruction(instructions, def, operands);
    sb_appendf(sb, "%04d %s\n", i, ins_str);

    i += 1 + operands->bytes_read;
  }

  char *buf = strdup(sb->buf);
  free(sb->buf);
  free(sb);
  return buf;
}

operands_t *read_operands(definition_t *def, uint8_t *instructions) {
  operands_t *operands = malloc(sizeof(operands_t));
  if (!operands) {
    return NULL;
  }

  operands->len = def->operand_widths_len;
  operands->operands = NULL;
  if (operands->len > 0) {
    operands->operands = calloc(def->operand_widths_len, sizeof(size_t));
    if (!operands->operands) {
      free(operands);
      return NULL;
    }
  }

  size_t offset = 0;
  for (size_t i = 0; i < def->operand_widths_len; ++i) {
    uint8_t width = def->operand_widths[i];
    switch (width) {
    case 2:
      operands->operands[i] = read_uint16_big_endian(instructions + offset);
      break;
    }
    offset += width;
  }

  operands->bytes_read = offset;
  return operands;
}
