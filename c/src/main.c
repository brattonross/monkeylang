#include "array_list.h"
#include "ast.h"
#include "compiler.h"
#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *prompt = ">> ";
static const char *monkey_face = "            __,__\n"
                                 "   .--.  .-\"     \"-.  .--.\n"
                                 "  / .. \\/  .-. .-.  \\/ .. \\\n"
                                 " | |  '|  /   Y   \\  |'  | |\n"
                                 " | \\   \\  \\ 0 | 0 /  /   / |\n"
                                 "  \\ '- ,\\.-\"\"\"\"\"\"\"-./, -' /\n"
                                 "   ''-' /_   ^ ^   _\\ '-''\n"
                                 "       |  \\._   _./  |\n"
                                 "       \\   \\ '~' /   /\n"
                                 "        '._ '-=-' _.'\n"
                                 "           '-----'\n";

int run_interpreter(void) {
  const char *username = getenv("USER");
  printf("Hello %s! This is the Monkey programming language!\n", username);
  printf("Feel free to type in commands\n");

  environment_t *env = new_environment();
  while (true) {
    printf("%s", prompt);

    char *input = NULL;
    size_t len;
    ssize_t read = getline(&input, &len, stdin);
    if (read == EOF) {
      break;
    }

    lexer_t *l = lexer_init(input);
    if (l == NULL) {
      fprintf(stderr, "failed to initialize lexer\n");
      exit(EXIT_FAILURE);
    }

    parser_t *p = parser_init(l);
    if (p == NULL) {
      fprintf(stderr, "failed to initialize parser\n");
      exit(EXIT_FAILURE);
    }

    program_t *prg = parser_parse_program(p);
    if (prg == NULL) {
      fprintf(stderr, "failed to parse program\n");
      exit(EXIT_FAILURE);
    }

    if (p->errors->len > 0) {
      fprintf(stderr,
              "%s\nWhoops! We ran into some monkey business here!\n parser "
              "errors:\n",
              monkey_face);
      for (size_t i = 0; i < p->errors->len; ++i) {
        fprintf(stderr, "\t%s\n", p->errors->arr[i]);
      }
      goto advance;
    }

    object_t *evald = eval_program(prg, env);
    if (evald != NULL) {
      fprintf(stdout, "%s\n", object_inspect(evald));
    }

  advance:
    free(input);
    input = NULL;
    free(evald);
    evald = NULL;
    parser_free(p);
  }

  return EXIT_SUCCESS;
}

int run_compile_execute(void) {
  while (true) {
    printf("%s", prompt);

    char *input = NULL;
    size_t len;
    ssize_t read = getline(&input, &len, stdin);
    if (read == EOF) {
      break;
    }

    lexer_t *l = lexer_init(input);
    if (l == NULL) {
      fprintf(stderr, "failed to initialize lexer\n");
      exit(EXIT_FAILURE);
    }

    parser_t *p = parser_init(l);
    if (p == NULL) {
      fprintf(stderr, "failed to initialize parser\n");
      exit(EXIT_FAILURE);
    }

    program_t *prg = parser_parse_program(p);
    if (prg == NULL) {
      fprintf(stderr, "failed to parse program\n");
      exit(EXIT_FAILURE);
    }

    if (p->errors->len > 0) {
      fprintf(stderr,
              "%s\nWhoops! We ran into some monkey business here!\n parser "
              "errors:\n",
              monkey_face);
      for (size_t i = 0; i < p->errors->len; ++i) {
        fprintf(stderr, "\t%s\n", p->errors->arr[i]);
      }
      goto advance;
    }

    compiler_t *c = new_compiler();
    if (c == NULL) {
      fprintf(stderr, "failed to initialize compiler\n");
      exit(EXIT_FAILURE);
    }

    compiler_error_t cmp_err = compiler_compile_program(c, prg);
    if (cmp_err != COMPILERE_SUCCESS) {
      fprintf(stderr, "compilation failed:\n%s\n",
              humanize_compiler_error(cmp_err));
      exit(EXIT_FAILURE);
    }

    vm_t *vm = new_vm(compiler_bytecode(c));
    if (vm == NULL) {
      fprintf(stderr, "failed to initialize virtual machine\n");
      exit(EXIT_FAILURE);
    }

    vm_error_t vm_err = vm_run(vm);
    if (vm_err != VME_SUCCESS) {
      fprintf(stderr, "failed to execute bytecode:\n%s\n",
              humanize_vm_error(vm_err));
      exit(EXIT_FAILURE);
    }

    object_t *stack_top = vm_stack_top(vm);
    puts(object_inspect(stack_top));

  advance:
    free(input);
    free(c);
    parser_free(p);
  }

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    return run_interpreter();
  }

  if (argc == 2 && strncmp(argv[1], "run", 3) == 0) {
    return run_compile_execute();
  }

  // TODO: Print help

  return EXIT_SUCCESS;
}
