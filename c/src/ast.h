#ifndef __AST_H__
#define __AST_H__

#include "token.h"
#include <stdlib.h>

typedef struct expression_t expression_t;

typedef struct {
  token_t *token;
  const char *value;
} identifier_t;

const char *identifier_token_literal(identifier_t *i);

typedef struct {
  token_t *token;
  int64_t value;
} integer_literal_t;

typedef struct {
  token_t *token;
  char *op;
  expression_t *right;
} prefix_expression_t;

typedef struct {
  token_t *token;
  expression_t *left;
  char *op;
  expression_t *right;
} infix_expression_t;

typedef enum {
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER_LITERAL,
  EXPRESSION_PREFIX,
  EXPRESSION_INFIX,
} expression_type_t;

struct expression_t {
  expression_type_t type;
  union {
    identifier_t *ident;
    integer_literal_t *integer;
    prefix_expression_t *prefix;
    infix_expression_t *infix;
  } value;
};

typedef enum {
  STATEMENT_LET,
  STATEMENT_RETURN,
  STATEMENT_EXPRESSION,
} statement_type_t;

typedef struct {
  token_t *token;
  identifier_t *name;
  expression_t *value;
} let_statement_t;

typedef struct {
  token_t *token;
  expression_t *value;
} return_statement_t;

typedef struct {
  token_t *token;
  expression_t *expression;
} expression_statement_t;

typedef struct {
  statement_type_t type;
  union {
    let_statement_t *let;
    return_statement_t *ret;
    expression_statement_t *exp;
  } value;
} statement_t;

const char *statement_token_literal(const statement_t *s);

typedef struct {
  statement_t **statements;
  size_t statements_len;
  size_t statements_cap;
} program_t;

void program_free(program_t *p);

char *program_token_literal(const program_t *p);
char *program_to_string(const program_t *p);

#endif // __AST_H__
