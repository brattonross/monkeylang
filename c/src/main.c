#include "eval.c"
#include "lexer.c"
#include "mem.c"
#include "object.c"
#include "parser.c"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <readline/history.h>
#include <readline/readline.h>

int main(void) {
  unsigned char buf[32 * 1024];
  Arena arena = {0};
  arena_init(&arena, buf, 32 * 1024);

  while (true) {
    char *line = readline(">> ");
    if (!line) {
      break;
    }

    add_history(line);

    Lexer lexer = {0};
    lexer_init(&lexer, line);
    Parser parser = {0};
    parser_init(&parser, &arena, &lexer);

    Program *program = parser_parse_program(&parser, &arena);
    if (parser.errors.length > 0) {
      for (size_t i = 0; i < parser.errors.length; ++i) {
        fprintf(stderr, "ERROR: %.*s\n",
                (int)parser.errors.errors[i].message.length,
                parser.errors.errors[i].message.buffer);
      }
      goto cleanup;
    }

    Object evaluated = {0};
    eval_program(program, &arena, &evaluated);

    String str = object_to_string(&evaluated, &arena);
    printf("%.*s\n", (int)str.length, str.buffer);

  cleanup:
    free(line);
    arena_reset(&arena);
  }

  return EXIT_SUCCESS;
}
