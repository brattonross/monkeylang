#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  const char *username = getenv("USER");
  printf("Hello %s! This is the Monkey programming language!\n", username);
  printf("Feel free to type in commands\n");

  while (1) {
    char *input = NULL;
    size_t len;
    ssize_t read = getline(&input, &len, stdin);
    if (read == EOF) {
      break;
    }
    lexer_t *lexer = lexer_init(input);
    if (lexer == NULL) {
      perror("failed to initialize lexer");
      abort();
    }

    token_t *token = lexer_next_token(lexer);
    while (token->type != TOKEN_EOF) {
      if (token == NULL) {
        perror("failed to get next token");
        abort();
      }
      token = lexer_next_token(lexer);
      printf("type: %d, literal: %s\n", token->type, token->literal);
    }

    free(token);
    free(input);
    lexer_free(lexer);
  }

  return 0;
}
