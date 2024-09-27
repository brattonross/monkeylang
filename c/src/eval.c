#include "eval.h"
#include "ast.h"
#include "object.h"

object_t *eval_expression(expression_t *e) {
  switch (e->type) {
  case EXPRESSION_INTEGER_LITERAL: {
    object_t *o = malloc(sizeof(object_t));
    if (o == NULL) {
      return NULL;
    }
    o->type = OBJECT_INTEGER;
    o->value.integer = malloc(sizeof(integer_object_t));
    if (o->value.integer == NULL) {
      free(o);
      return NULL;
    }
    o->value.integer->value = e->value.integer->value;
    return o;
  }
  default:
    return NULL;
  }
}

object_t *eval_statement(statement_t *s) {
  switch (s->type) {
  case STATEMENT_EXPRESSION:
    return eval_expression(s->value.exp->expression);
  default:
    return NULL;
  }
}

object_t *eval_statements(size_t len, statement_t **s) {
  object_t *o = NULL;
  for (size_t i = 0; i < len; ++i) {
    o = eval_statement(s[i]);
  }
  return o;
}

object_t *eval_program(program_t *p) {
  return eval_statements(p->statements_len, p->statements);
}
