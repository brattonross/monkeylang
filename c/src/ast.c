#include "ast.h"
#include <stdio.h>

const char *statement_token_literal(const statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    return s->value.let->token->literal;
  default:
    return NULL;
  }
}

const char *identifier_token_literal(identifier_t *i) {
  return i->token->literal;
}

const char *program_token_literal(const program_t *p) {
  if (p->statements_len > 0) {
    return statement_token_literal(p->statements[0]);
  }
  return "";
}

void program_free(program_t *p) {
  free(p->statements);
  free(p);
}
