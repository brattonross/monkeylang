#ifndef __PARSER_H__
#define __PARSER_H__

#include "array_list.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdbool.h>

typedef struct {
  lexer_t *lexer;
  token_t *current_token;
  token_t *peek_token;
  array_list_t *errors;
} parser_t;

typedef enum {
  PARSER_SUCCESS = 0,
  PARSER_INVALID_ARGUMENT_ERROR = -1,
  PARSER_ALLOC_ERROR = -2,
} parser_error_t;

parser_t *parser_init(lexer_t *l);

program_t *parser_parse_program(parser_t *p);

parser_error_t parser_peek_error(parser_t *p, token_type_t t);

#endif // __PARSER_H__
