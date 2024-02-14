#include "lexer.h"
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
    lexer_t *lexer = lexer_new(input);
    for (token_t tok = lexer_next_token(lexer); tok.token_type != END_OF_FILE;
         tok = lexer_next_token(lexer)) {
      printf("type: %d, literal: %s\n", tok.token_type, tok.literal);
    }
    free(input);
  }

  return 0;
}
