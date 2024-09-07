#ifndef __PARSER_H__
#define __PARSER_H__

#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdbool.h>

typedef struct {
  lexer_t *lexer;
  token_t *current_token;
  token_t *peek_token;
} parser_t;

parser_t *parser_init(lexer_t *l);

program_t *parser_parse_program(parser_t *p);

#endif // __PARSER_H__
