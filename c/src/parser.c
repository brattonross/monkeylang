#include "parser.h"
#include "array_list.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <string.h>

parser_error_t parser_register_prefix(parser_t *p, token_type_t t,
                                      prefix_parse_fn fn);
parser_error_t parser_register_infix(parser_t *p, token_type_t t,
                                     infix_parse_fn fn);
expression_t *parser_parse_identifier(parser_t *p);
expression_t *parser_parse_integer_literal(parser_t *p);
expression_t *parser_parse_prefix_expression(parser_t *p);

void parser_next_token(parser_t *p) {
  p->current_token = p->peek_token;
  p->peek_token = lexer_next_token(p->lexer);
}

parser_t *parser_init(lexer_t *l) {
  if (l == NULL) {
    return NULL;
  }
  parser_t *p = malloc(sizeof(parser_t));
  if (p == NULL) {
    return NULL;
  }
  p->lexer = l;
  p->current_token = NULL;
  p->peek_token = NULL;
  p->errors = array_list_create(1);

  p->prefix_parse_fns = malloc(1);
  if (p->prefix_parse_fns == NULL) {
    return NULL;
  }

  parser_register_prefix(p, TOKEN_IDENTIFIER, parser_parse_identifier);
  parser_register_prefix(p, TOKEN_INT, parser_parse_integer_literal);
  parser_register_prefix(p, TOKEN_BANG, parser_parse_prefix_expression);
  parser_register_prefix(p, TOKEN_MINUS, parser_parse_prefix_expression);

  p->infix_parse_fns = malloc(1);
  if (p->infix_parse_fns == NULL) {
    return NULL;
  }

  parser_next_token(p);
  parser_next_token(p);

  return p;
}

parser_error_t parser_register_prefix(parser_t *p, token_type_t t,
                                      prefix_parse_fn fn) {
  prefix_parse_fn *tmp = realloc(
      p->prefix_parse_fns, p->prefix_parse_fns_len + sizeof(prefix_parse_fn));
  if (tmp == NULL) {
    return PARSER_ALLOC_ERROR;
  }
  p->prefix_parse_fns = tmp;
  p->prefix_parse_fns[t] = fn;
  p->prefix_parse_fns_len++;
  return PARSER_SUCCESS;
}

parser_error_t parser_register_infix(parser_t *p, token_type_t t,
                                     infix_parse_fn fn) {
  infix_parse_fn *tmp = realloc(p->infix_parse_fns, p->infix_parse_fns_len +
                                                        sizeof(infix_parse_fn));
  if (tmp == NULL) {
    return PARSER_ALLOC_ERROR;
  }
  p->infix_parse_fns = tmp;
  p->infix_parse_fns[t] = fn;
  p->infix_parse_fns_len++;
  return PARSER_SUCCESS;
}

parser_error_t parser_peek_error(parser_t *p, token_type_t t) {
  if (p == NULL) {
    return PARSER_INVALID_ARGUMENT_ERROR;
  }

  char *msg;
  if (asprintf(&msg, "expected next token to be %s, got %s",
               token_type_humanize(t),
               token_type_humanize(p->peek_token->type)) < 0) {
    return PARSER_ALLOC_ERROR;
  }
  if (array_list_push(p->errors, msg) != ARRAY_LIST_SUCCESS) {
    return PARSER_ALLOC_ERROR;
  }
  return PARSER_SUCCESS;
}

parser_error_t parser_no_prefix_parser_fn_error(parser_t *p, token_type_t t) {
  if (p == NULL) {
    return PARSER_INVALID_ARGUMENT_ERROR;
  }

  char *msg;
  if (asprintf(&msg, "no prefix parse function found for token type %s",
               token_type_humanize(t))) {
    return PARSER_ALLOC_ERROR;
  }
  if (array_list_push(p->errors, msg) != ARRAY_LIST_SUCCESS) {
    return PARSER_ALLOC_ERROR;
  }
  return PARSER_SUCCESS;
}

bool parser_current_token_is(const parser_t *p, token_type_t t) {
  return p->current_token->type == t;
}

bool parser_peek_token_is(const parser_t *p, token_type_t t) {
  return p->peek_token->type == t;
}

bool parser_expect_peek(parser_t *p, token_type_t t) {
  if (parser_peek_token_is(p, t)) {
    parser_next_token(p);
    return true;
  }
  parser_peek_error(p, t);
  return false;
}

void free_statement(statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    free(s->value.let);
    break;
  case STATEMENT_RETURN:
    free(s->value.ret);
    break;
  case STATEMENT_EXPRESSION:
    free(s->value.exp);
    break;
  }
  free(s);
}

statement_t *parse_let_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_LET;
  s->value.let = malloc(sizeof(let_statement_t));
  if (s->value.let == NULL) {
    free_statement(s);
    return NULL;
  }

  s->value.let->token = malloc(sizeof(token_t));
  if (s->value.let->token == NULL) {
    free_statement(s);
    return NULL;
  }
  memcpy(s->value.let->token, p->current_token, sizeof(token_t));
  s->value.let->token->literal = strdup(p->current_token->literal);

  if (!parser_expect_peek(p, TOKEN_IDENTIFIER)) {
    free_statement(s);
    return NULL;
  }

  s->value.let->name = malloc(sizeof(identifier_t));
  if (s->value.let->name == NULL) {
    free_statement(s);
    return NULL;
  }
  s->value.let->name->token = malloc(sizeof(token_t));
  if (s->value.let->name->token == NULL) {
    free_statement(s);
    return NULL;
  }
  memcpy(s->value.let->name->token, p->current_token, sizeof(token_t));
  s->value.let->name->value = strdup(p->current_token->literal);

  if (!parser_expect_peek(p, TOKEN_ASSIGN)) {
    free_statement(s);
    return NULL;
  }

  // TODO:
  s->value.let->value = NULL;

  while (!parser_current_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

statement_t *parse_return_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_RETURN;
  s->value.ret = malloc(sizeof(return_statement_t));
  if (s->value.ret == NULL) {
    free_statement(s);
    return NULL;
  }

  s->value.ret->token = malloc(sizeof(token_t));
  if (s->value.ret->token == NULL) {
    free_statement(s);
    return NULL;
  }
  memcpy(s->value.ret->token, p->current_token, sizeof(token_t));
  s->value.ret->token->literal = strdup(p->current_token->literal);

  while (!parser_current_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

expression_t *parser_parse_expression(parser_t *p,
                                      parser_precedence_t precedence) {
  prefix_parse_fn prefix = p->prefix_parse_fns[p->current_token->type];
  if (prefix == NULL) {
    parser_no_prefix_parser_fn_error(p, p->current_token->type);
    return NULL;
  }
  expression_t *left = prefix(p);
  return left;
}

statement_t *parse_expression_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_EXPRESSION;
  s->value.exp = malloc(sizeof(expression_statement_t));
  if (s->value.exp == NULL) {
    free_statement(s);
    return NULL;
  }

  s->value.exp->token = malloc(sizeof(token_t));
  if (s->value.exp->token == NULL) {
    free_statement(s);
    return NULL;
  }
  memcpy(s->value.exp->token, p->current_token, sizeof(token_t));
  s->value.exp->token->literal = strdup(p->current_token->literal);

  s->value.exp->expression =
      parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);
  if (s->value.exp->expression == NULL) {
    free_statement(s);
    return NULL;
  }

  while (!parser_current_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

statement_t *parser_parse_statement(parser_t *p) {
  if (p == NULL) {
    return NULL;
  }
  switch (p->current_token->type) {
  case TOKEN_LET:
    return parse_let_statement(p);
  case TOKEN_RETURN:
    return parse_return_statement(p);
  default:
    return parse_expression_statement(p);
  }
}

expression_t *parser_parse_identifier(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_IDENTIFIER;
  e->value.ident = malloc(sizeof(identifier_t));
  if (e->value.ident == NULL) {
    free(e);
    return NULL;
  }

  e->value.ident->token = malloc(sizeof(token_t));
  if (e->value.ident->token == NULL) {
    free(e);
    free(e->value.ident->token);
    return NULL;
  }

  e->value.ident->token->type = TOKEN_IDENTIFIER;
  e->value.ident->token->literal = strdup(p->current_token->literal);
  e->value.ident->value = strdup(p->current_token->literal);

  return e;
}

expression_t *parser_parse_integer_literal(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_INTEGER_LITERAL;
  e->value.integer = malloc(sizeof(integer_literal_t));
  if (e->value.integer == NULL) {
    free(e);
    return NULL;
  }

  e->value.integer->token = malloc(sizeof(token_t));
  if (e->value.integer->token == NULL) {
    free(e);
    free(e->value.integer->token);
    return NULL;
  }

  e->value.integer->token->type = TOKEN_INT;
  e->value.integer->token->literal = strdup(p->current_token->literal);
  e->value.integer->value = strtol(p->current_token->literal, NULL, 10);

  return e;
}

expression_t *parser_parse_prefix_expression(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_PREFIX;
  e->value.prefix = malloc(sizeof(prefix_expression_t));
  if (e->value.prefix == NULL) {
    free(e);
    return NULL;
  }

  e->value.prefix->token = malloc(sizeof(token_t));
  if (e->value.prefix->token == NULL) {
    free(e);
    free(e->value.prefix->token);
    return NULL;
  }

  e->value.prefix->token->type = p->current_token->type;
  e->value.prefix->token->literal = strdup(p->current_token->literal);
  e->value.prefix->op = strdup(p->current_token->literal);

  parser_next_token(p);

  e->value.prefix->right = parser_parse_expression(p, PARSER_PRECEDENCE_PREFIX);

  return e;
}

program_t *parser_parse_program(parser_t *p) {
  program_t *prg = malloc(sizeof(program_t));
  if (prg == NULL) {
    return NULL;
  }
  prg->statements_len = 0;
  prg->statements_cap = 128;
  prg->statements = calloc(prg->statements_cap, sizeof(statement_t));
  if (prg->statements == NULL) {
    program_free(prg);
    return NULL;
  }

  while ((p->current_token->type != TOKEN_EOF)) {
    statement_t *statement = parser_parse_statement(p);
    if (prg->statements_len == prg->statements_cap) {
      prg->statements_cap *= 2;
      prg->statements =
          realloc(prg->statements, prg->statements_cap * sizeof(statement_t));
      if (prg->statements == NULL) {
        program_free(prg);
        return NULL;
      }
    }
    prg->statements[prg->statements_len] = statement;
    prg->statements_len++;
    parser_next_token(p);
  }
  return prg;
}
