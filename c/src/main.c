#include "array_list.h"
#include "ast.h"
#include "env.h"
#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

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

int main() {
  const char *username = getenv("USER");
  printf("Hello %s! This is the Monkey programming language!\n", username);
  printf("Feel free to type in commands\n");

  environment_t *env = new_environment();
  while (1) {
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

  return 0;
}
