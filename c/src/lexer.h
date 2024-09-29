#ifndef __LEXER_H__
#define __LEXER_H__

#include "token.h"
#include <stdlib.h>

typedef struct {
  const char *input;
  size_t pos;
  size_t read_pos;
  char ch;
} lexer_t;

lexer_t *lexer_init(const char *input);
token_t *lexer_next_token(lexer_t *l);
void lexer_free(lexer_t *l);

#endif // __LEXER_H__
