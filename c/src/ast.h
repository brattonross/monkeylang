#ifndef __AST_H__
#define __AST_H__

#include "token.h"
#include <stdbool.h>
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

typedef struct {
  token_t *token;
  bool value;
} boolean_literal_t;

typedef struct block_statement_t block_statement_t;

typedef struct {
  token_t *token;
  expression_t *condition;
  block_statement_t *consequence;
  block_statement_t *alternative;
} if_expression_t;

typedef struct {
  token_t *token;
  identifier_t **parameters;
  size_t parameters_len;
  block_statement_t *body;
} function_literal_t;

typedef struct {
  token_t *token;
  expression_t *fn;
  size_t argc;
  expression_t **argv;
} call_expression_t;

typedef enum {
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER_LITERAL,
  EXPRESSION_PREFIX,
  EXPRESSION_INFIX,
  EXPRESSION_BOOLEAN_LITERAL,
  EXPRESSION_IF,
  EXPRESSION_FUNCTION_LITERAL,
  EXPRESSION_CALL,
} expression_type_t;

struct expression_t {
  expression_type_t type;
  union {
    identifier_t *ident;
    integer_literal_t *integer;
    prefix_expression_t *prefix;
    infix_expression_t *infix;
    boolean_literal_t *boolean;
    if_expression_t *if_;
    function_literal_t *fn;
    call_expression_t *call;
  } value;
};

typedef enum {
  STATEMENT_LET,
  STATEMENT_RETURN,
  STATEMENT_EXPRESSION,
  STATEMENT_BLOCK,
} statement_type_t;

typedef struct statement_t statement_t;

typedef struct {
  token_t *token;
  identifier_t *ident;
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

struct block_statement_t {
  token_t *token;
  statement_t **statements;
  size_t statements_len;
};

struct statement_t {
  statement_type_t type;
  union {
    let_statement_t *let;
    return_statement_t *ret;
    expression_statement_t *exp;
    block_statement_t *block;
  } value;
};

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
