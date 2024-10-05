#include "compiler.h"
#include "ast.h"
#include "code.h"
#include "object.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

compiler_t *new_compiler(void) {
  compiler_t *c = malloc(sizeof(compiler_t));
  if (!c) {
    return NULL;
  }

  c->instructions = malloc(sizeof(instructions_t));
  if (!c->instructions) {
    free(c);
    return NULL;
  }

  c->instructions->cap = 16;
  c->instructions->len = 0;
  c->instructions->arr = NULL;

  c->constants_cap = 16;
  c->constants_len = 0;
  c->constants = NULL;

  return c;
}

ssize_t compiler_add_constant(compiler_t *c, object_t *o) {
  if (c->constants == NULL || c->constants_cap == 0) {
    c->constants_cap = 16;
    c->constants = calloc(c->constants_cap, sizeof(object_t *));
    if (!c->constants) {
      free(c->instructions);
      free(c);
      return -1;
    }
  } else if (c->constants_len >= c->constants_cap) {
    while (c->constants_len >= c->constants_cap) {
      c->constants_cap *= 2;
    }

    c->constants = realloc(c->constants, c->constants_cap * sizeof(object_t));
    if (!c->constants) {
      for (size_t i = 0; i < c->constants_len; ++i) {
        object_free(c->constants[i]);
      }
      free(c);
      return -1;
    }
  }

  c->constants[c->constants_len] = o;
  c->constants_len++;

  return c->constants_len - 1;
}

ssize_t compiler_add_instruction(compiler_t *c, instructions_t *instructions) {
  size_t pos = c->instructions->len;
  size_t new_len = c->instructions->len + instructions->len;

  if (c->instructions->cap == 0 || !c->instructions->arr) {
    c->instructions->cap = 16;
    c->instructions->arr = calloc(c->instructions->cap, sizeof(uint8_t));
    if (!c->instructions->arr) {
      free(c->instructions);
      free(c);
      return -1;
    }
  } else if (new_len >= c->instructions->cap) {
    while (new_len >= c->instructions->cap) {
      c->instructions->cap *= 2;
    }

    c->instructions->arr =
        realloc(c->instructions->arr, c->instructions->cap * sizeof(uint8_t));
    if (!c->instructions->arr) {
      for (size_t i = 0; i < c->constants_len; ++i) {
        object_free(c->constants[i]);
      }
      free(c->instructions);
      free(c);
      return -1;
    }
  }

  for (size_t i = 0; i < instructions->len; ++i) {
    c->instructions->arr[pos + i] = instructions->arr[i];
  }
  c->instructions->len = new_len;

  return pos;
}

ssize_t compiler_emit(compiler_t *c, opcode_t op, size_t n, ssize_t *operands) {
  instructions_t *instruction = make(op, n, operands);
  ssize_t pos = compiler_add_instruction(c, instruction);
  return pos;
}

compiler_error_t compiler_compile_expression(compiler_t *c, expression_t *e) {
  switch (e->type) {
  case EXPRESSION_INFIX: {
    compiler_error_t err = compiler_compile_expression(c, e->value.infix->left);
    if (err != COMPILER_SUCCESS) {
      return err;
    }
    err = compiler_compile_expression(c, e->value.infix->right);
    if (err != COMPILER_SUCCESS) {
      return err;
    }
  } break;
  case EXPRESSION_INTEGER_LITERAL: {
    object_t *integer = new_integer_object(e->value.integer->value);
    compiler_emit(c, OP_CONSTANT, 1,
                  (ssize_t[1]){compiler_add_constant(c, integer)});
  } break;
  }
  return COMPILER_SUCCESS;
}

compiler_error_t compiler_compile_statement(compiler_t *c, statement_t *s) {
  switch (s->type) {
  case STATEMENT_EXPRESSION: {
    compiler_error_t err =
        compiler_compile_expression(c, s->value.exp->expression);
    if (err != COMPILER_SUCCESS) {
      return err;
    }
  } break;
  }
  return COMPILER_SUCCESS;
}

compiler_error_t compiler_compile_program(compiler_t *c, program_t *p) {
  for (size_t i = 0; i < p->statements_len; ++i) {
    compiler_error_t err = compiler_compile_statement(c, p->statements[i]);
    if (err != COMPILER_SUCCESS) {
      return err;
    }
  }
  return COMPILER_SUCCESS;
}

bytecode_t *compiler_bytecode(compiler_t *c) {
  bytecode_t *b = malloc(sizeof(bytecode_t));
  if (!b) {
    return NULL;
  }

  b->instructions = malloc(sizeof(instructions_t));
  if (!b->instructions) {
    free(b);
    return NULL;
  }

  b->instructions->arr = calloc(c->instructions->cap, sizeof(uint8_t));
  if (!b->instructions->arr) {
    free(b->instructions);
    free(b);
    return NULL;
  }
  memcpy(b->instructions->arr, c->instructions->arr,
         c->instructions->len * sizeof(uint8_t));
  b->instructions->cap = c->instructions->cap;
  b->instructions->len = c->instructions->len;

  b->constants = calloc(c->constants_len, sizeof(object_t));
  if (!b->constants) {
    free(b->instructions);
    free(b);
    return NULL;
  }
  b->constants_len = c->constants_len;
  memcpy(b->constants, c->constants, c->constants_len * sizeof(object_t));

  return b;
}
