#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <string.h>

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

  parser_next_token(p);
  parser_next_token(p);

  return p;
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
  return false;
}

void free_statement(statement_t *s) {
  free(s->value.let);
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
  s->value.let->type = STATEMENT_LET;

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
  default:
    return NULL;
  }
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
