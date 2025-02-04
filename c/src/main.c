#include "lexer.c"
#include "mem.c"
#include "token.c"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <readline/history.h>
#include <readline/readline.h>

int main(void) {
  unsigned char buf[4096];
  Arena arena = {0};
  arena_init(&arena, buf, 4096);

  while (true) {
    Lexer lexer = {0};
    char *line = readline(">> ");
    if (!line) {
      break;
    }

    add_history(line);
    lexer_init(&lexer, line);
    Token token;
    while ((token = lexer_next_token(&lexer)).type != TOKEN_EOF) {
      printf("Token{.type = %s, .literal = %.*s}\n",
             token_type_strings[token.type].buffer, (int)token.literal.len,
             token.literal.buffer);
    }

    free(line);
  }
  Token t = {.literal = String("a"), .type = TOKEN_IDENT};
  printf("token type is %s\n", token_type_strings[t.type].buffer);

  return EXIT_SUCCESS;
}
