#include "vm.h"
#include "code.h"
#include "object.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

vm_t *new_vm(bytecode_t *b) {
  vm_t *vm = malloc(sizeof(vm_t));
  if (!vm) {
    return NULL;
  }

  vm->constants_len = b->constants_len;
  vm->constants = calloc(b->constants_len, sizeof(object_t));
  if (!vm->constants) {
    free(vm);
    return NULL;
  }
  memcpy(vm->constants, b->constants, b->constants_len * sizeof(object_t));

  vm->instructions = malloc(sizeof(instructions_t));
  if (!vm->instructions) {
    free(vm);
    return NULL;
  }

  vm->instructions->cap = b->instructions->cap;
  vm->instructions->len = b->instructions->len;
  vm->instructions->arr = calloc(b->instructions->cap, sizeof(uint8_t));
  if (!vm->instructions->arr) {
    free(vm->instructions);
    free(vm);
    return NULL;
  }
  memcpy(vm->instructions->arr, b->instructions->arr,
         b->instructions->len * sizeof(uint8_t));

  vm->sp = 0;
  vm->stack_cap = 128;
  vm->stack = calloc(vm->stack_cap, sizeof(object_t));
  if (!vm->stack) {
    free(vm->instructions);
    free(vm);
    return NULL;
  }

  return vm;
}

static const uint64_t max_stack_size = 2048;

vm_error_t vm_push(vm_t *vm, object_t *obj) {
  if (vm->sp >= max_stack_size) {
    return VME_STACK_OVERFLOW;
  }

  if (vm->sp >= vm->stack_cap) {
    vm->stack_cap *= 2;
    vm->stack = realloc(vm->stack, vm->stack_cap * sizeof(object_t));
    if (!vm->stack) {
      return VME_ALLOC_ERROR;
    }
  }

  vm->stack[vm->sp] = obj;
  vm->sp++;

  return VME_SUCCESS;
}

object_t *vm_pop(vm_t *vm) {
  if (!vm) {
    return NULL;
  }
  object_t *o = vm->stack[vm->sp - 1];
  vm->sp--;
  return o;
}

vm_error_t vm_run(vm_t *vm) {
  for (size_t ip = 0; ip < vm->instructions->len; ++ip) {
    uint8_t op = vm->instructions->arr[ip];
    switch (op) {
    case OP_CONSTANT: {
      uint16_t const_index =
          read_uint16_big_endian(vm->instructions->arr + ip + 1);
      ip += 2;

      vm_error_t err = vm_push(vm, vm->constants[const_index]);
      if (err != VME_SUCCESS) {
        return err;
      }
    } break;
    case OP_ADD: {
      object_t *right = vm_pop(vm);
      object_t *left = vm_pop(vm);
      vm_push(vm, new_integer_object(left->value.integer->value +
                                     right->value.integer->value));
    } break;
    }
  }

  return VME_SUCCESS;
}

object_t *vm_stack_top(const vm_t *vm) {
  if (!vm || vm->sp == 0) {
    return NULL;
  }
  return vm->stack[vm->sp - 1];
}
