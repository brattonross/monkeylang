#include "vm_test.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/vm.h"
#include "unity.h"
#include <stdint.h>
#include <stdlib.h>

void test_vm_integer_arithmetic(void) {
  typedef struct {
    char *input;
    int64_t expected;
  } test_case_t;
  static const test_case_t test_cases[] = {
      {"1", 1}, {"2", 2}, {"1 + 2", 2}, // FIXME:
  };
  static const size_t test_cases_len = sizeof(test_cases) / sizeof(*test_cases);

  for (size_t i = 0; i < test_cases_len; ++i) {
    lexer_t *l = lexer_init(test_cases[i].input);
    parser_t *p = parser_init(l);
    program_t *prg = parser_parse_program(p);

    compiler_t *c = new_compiler();
    compiler_error_t err = compiler_compile_program(c, prg);
    TEST_ASSERT_EQUAL_INT(COMPILERE_SUCCESS, err);

    vm_t *vm = new_vm(compiler_bytecode(c));
    vm_error_t vm_err = vm_run(vm);
    TEST_ASSERT_EQUAL_INT(VME_SUCCESS, vm_err);

    object_t *stack_elem = vm_stack_top(vm);
    TEST_ASSERT_NOT_NULL(stack_elem);
    TEST_ASSERT_EQUAL_INT(OBJECT_INTEGER, stack_elem->type);
    TEST_ASSERT_EQUAL_INT(test_cases[i].expected,
                          stack_elem->value.integer->value);
  }
}
