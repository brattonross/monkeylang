#pragma once

#include "ast.h"
#include "code.h"
#include "object.h"

typedef struct {
  instructions_t *instructions;
  size_t constants_len;
  size_t constants_cap;
  object_t **constants;
} compiler_t;

compiler_t *new_compiler(void);

typedef enum {
  COMPILER_SUCCESS,
} compiler_error_t;

compiler_error_t compiler_compile_program(compiler_t *c, program_t *p);

typedef struct {
  instructions_t *instructions;
  size_t constants_len;
  object_t **constants;
} bytecode_t;

bytecode_t *compiler_bytecode(compiler_t *c);
