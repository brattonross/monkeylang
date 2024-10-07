#pragma once

#include "code.h"
#include "compiler.h"
#include "object.h"
#include <stdlib.h>

typedef struct {
  size_t constants_len;
  object_t **constants;
  instructions_t *instructions;
  size_t sp;
  size_t stack_cap;
  object_t **stack;
} vm_t;

typedef enum {
  VME_SUCCESS,
  VME_ALLOC_ERROR = -1,
  VME_STACK_OVERFLOW = -2,
} vm_error_t;

char *humanize_vm_error(vm_error_t err);

vm_t *new_vm(bytecode_t *b);
vm_error_t vm_run(vm_t *vm);
object_t *vm_stack_top(const vm_t *vm);
