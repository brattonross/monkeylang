#include "ast.h"
#include <stdio.h>
#include <string.h>

const char *statement_token_literal(const statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    return strdup(s->value.let->token->literal);
  case STATEMENT_RETURN:
    return strdup(s->value.ret->token->literal);
  default:
    return NULL;
  }
}

const char *identifier_token_literal(identifier_t *i) {
  return strdup(i->token->literal);
}

const char *program_token_literal(const program_t *p) {
  if (p->statements_len > 0) {
    return strdup(statement_token_literal(p->statements[0]));
  }
  return NULL;
}

void program_free(program_t *p) {
  free(p->statements);
  free(p);
}
