#ifndef __AST_H__
#define __AST_H__

#include "token.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct expression_t expression_t;

typedef struct {
  token_t *token;
  char *value;
} identifier_t;

void identifier_free(identifier_t *ident);
char *identifier_to_string(identifier_t *ident);

const char *identifier_token_literal(identifier_t *i);

typedef struct {
  token_t *token;
  int64_t value;
} integer_literal_t;

void integer_literal_free(integer_literal_t *exp);

typedef struct {
  token_t *token;
  char *op;
  expression_t *right;
} prefix_expression_t;

void prefix_expression_free(prefix_expression_t *exp);

typedef struct {
  token_t *token;
  expression_t *left;
  char *op;
  expression_t *right;
} infix_expression_t;

void infix_expression_free(infix_expression_t *exp);

typedef struct {
  token_t *token;
  bool value;
} boolean_literal_t;

void boolean_literal_free(boolean_literal_t *exp);

typedef struct block_statement_t block_statement_t;

typedef struct {
  token_t *token;
  expression_t *condition;
  block_statement_t *consequence;
  block_statement_t *alternative;
} if_expression_t;

void if_expression_free(if_expression_t *e);

typedef struct {
  token_t *token;
  identifier_t **parameters;
  size_t parameters_len;
  block_statement_t *body;
} function_literal_t;

void function_literal_free(function_literal_t *exp);

typedef struct {
  token_t *token;
  expression_t *fn;
  size_t argc;
  expression_t **argv;
} call_expression_t;

void call_expression_free(call_expression_t *exp);

typedef struct {
  token_t *token;
  char *value;
} string_literal_t;

void string_literal_free(string_literal_t *exp);

typedef struct {
  token_t *token;
  size_t len;
  expression_t **elements;
} array_literal_t;

typedef struct {
  token_t *token;
  expression_t *left;
  expression_t *index;
} index_expression_t;

typedef struct {
  expression_t *key;
  expression_t *value;
} hash_item_t;
typedef struct {
  token_t *token;
  size_t len;
  hash_item_t **pairs;
} hash_literal_t;

typedef enum {
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER_LITERAL,
  EXPRESSION_PREFIX,
  EXPRESSION_INFIX,
  EXPRESSION_BOOLEAN_LITERAL,
  EXPRESSION_IF,
  EXPRESSION_FUNCTION_LITERAL,
  EXPRESSION_CALL,
  EXPRESSION_STRING,
  EXPRESSION_ARRAY_LITERAL,
  EXPRESSION_INDEX,
  EXPRESSION_HASH,
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
    string_literal_t *string;
    array_literal_t *arr;
    index_expression_t *index;
    hash_literal_t *hash;
  } value;
};

void expression_free(expression_t *exp);
char *expression_to_string(expression_t *e);

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

void let_statement_free(let_statement_t *let);

typedef struct {
  token_t *token;
  expression_t *value;
} return_statement_t;

void return_statement_free(return_statement_t *ret);

typedef struct {
  token_t *token;
  expression_t *expression;
} expression_statement_t;

void expression_statement_free(expression_statement_t *exp);

struct block_statement_t {
  token_t *token;
  statement_t **statements;
  size_t statements_len;
};

void block_statement_free(block_statement_t *block);
char *block_statement_to_string(block_statement_t *s);

struct statement_t {
  statement_type_t type;
  union {
    let_statement_t *let;
    return_statement_t *ret;
    expression_statement_t *exp;
    block_statement_t *block;
  } value;
};

void statement_free(statement_t *s);
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
