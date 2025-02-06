#include "../src/ast.c"
#include "../src/mem.c"
#include <assert.h>

void test_string(void);

int main(void) { test_string(); }

void test_string(void) {
  Arena arena = {0};
  char arena_buf[8192];
  arena_init(&arena, &arena_buf, 8192);

  Program *program = program_create(&arena);

  Statement s = {
      .type = STATEMENT_LET,
      .data =
          {
              .let_statement =
                  {
                      .token = {.type = TOKEN_LET, .literal = String("let")},
                      .name =
                          &(Identifier){
                              .token = {.type = TOKEN_IDENT,
                                        .literal = String("myVar")},
                              .value = String("myVar"),
                          },
                      .value =
                          &(Expression){
                              .type = EXPRESSION_IDENTIFIER,
                              .data =
                                  {
                                      .identifier =
                                          {
                                              .token = {.type = TOKEN_IDENT,
                                                        .literal = String(
                                                            "anotherVar")},
                                              .value = String("anotherVar"),
                                          },
                                  },
                          },
                  },
          },
  };
  program_append_statement(program, &arena, s);

  assert(string_cmp(String("let myVar = anotherVar;"),
                    program_to_string(program, &arena)));
}
