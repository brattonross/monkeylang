#include <stdlib.h>

#include "token.h"

typedef struct {
  char *input;
  size_t position;
  size_t read_position;
  char ch;
} lexer_t;

void lexer_read_char(lexer_t *lexer);

lexer_t *lexer_new(char *input);

token_t lexer_next_token(lexer_t *l);
