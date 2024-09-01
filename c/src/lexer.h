#include "token.h"
#include <stdlib.h>

#ifndef __LEXER_H
#define __LEXER_H

typedef struct {
  const char *input;
  size_t pos;
} lexer_t;

lexer_t *new_lexer(const char *input);

char current_char(lexer_t *l);

void read_char(lexer_t *l);

token_t next_token(lexer_t *l);

char *read_identifier(lexer_t *l);

#endif // __LEXER_H
