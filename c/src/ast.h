#ifndef __AST_H__
#define __AST_H__

#include "token.h"
#include <stdlib.h>

typedef union {
} expression_t;

typedef struct {
  token_t *token;
  const char *value;
} identifier_t;

const char *identifier_token_literal(identifier_t *i);

typedef enum {
  STATEMENT_LET,
  STATEMENT_RETURN,
} statement_type_t;

typedef struct {
  token_t *token;
  identifier_t *name;
  expression_t value;
} let_statement_t;

typedef struct {
  token_t *token;
  expression_t *value;
} return_statement_t;

typedef struct {
  statement_type_t type;
  union {
    let_statement_t *let;
    return_statement_t *ret;
  } value;
} statement_t;

const char *statement_token_literal(const statement_t *s);

typedef struct {
  statement_t **statements;
  size_t statements_len;
  size_t statements_cap;
} program_t;

void program_free(program_t *p);

const char *program_token_literal(const program_t *p);

#endif // __AST_H__
