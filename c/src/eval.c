#include "eval.h"
#include "ast.h"
#include "object.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

object_t *new_boolean_object(bool value) {
  object_t *o = malloc(sizeof(object_t));
  if (o == NULL) {
    return NULL;
  }
  o->type = OBJECT_BOOLEAN;
  o->value.boolean = malloc(sizeof(boolean_object_t));
  if (o->value.boolean == NULL) {
    free(o);
    return NULL;
  }
  o->value.boolean->value = value;
  return o;
}

object_t *new_integer_object(int64_t value) {
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
  o->value.integer->value = value;
  return o;
}

object_t *eval_bang_operator_expression(const object_t *right) {
  switch (right->type) {
  case OBJECT_BOOLEAN:
    return new_boolean_object(!right->value.boolean->value);
  case OBJECT_NULL:
    return new_boolean_object(true);
  default:
    return new_boolean_object(false);
  }
}

object_t *eval_minus_prefix_operator_expression(const object_t *right) {
  if (right->type != OBJECT_INTEGER) {
    return NULL;
  }
  return new_integer_object(-right->value.integer->value);
}

object_t *eval_prefix_expression(const char *operator, const object_t * right) {
  if (strncmp(operator, "!", 1) == 0) {
    return eval_bang_operator_expression(right);
  } else if (strncmp(operator, "-", 1) == 0) {
    return eval_minus_prefix_operator_expression(right);
  }
  return NULL;
}

object_t *eval_integer_infix_expression(const char *operator,
                                        const object_t * left,
                                        const object_t *right) {
  int64_t left_value = left->value.integer->value;
  int64_t right_value = right->value.integer->value;
  if (strncmp(operator, "+", 1) == 0) {
    return new_integer_object(left_value + right_value);
  } else if (strncmp(operator, "-", 1) == 0) {
    return new_integer_object(left_value - right_value);
  } else if (strncmp(operator, "*", 1) == 0) {
    return new_integer_object(left_value * right_value);
  } else if (strncmp(operator, "/", 1) == 0) {
    return new_integer_object(left_value / right_value);
  } else if (strncmp(operator, "<", 1) == 0) {
    return new_boolean_object(left_value < right_value);
  } else if (strncmp(operator, ">", 1) == 0) {
    return new_boolean_object(left_value > right_value);
  } else if (strncmp(operator, "==", 2) == 0) {
    return new_boolean_object(left_value == right_value);
  } else if (strncmp(operator, "!=", 2) == 0) {
    return new_boolean_object(left_value != right_value);
  }
  return NULL;
}

object_t *eval_infix_expression(const char *operator, const object_t * left,
                                const object_t *right) {
  if (left->type == OBJECT_INTEGER && right->type == OBJECT_INTEGER) {
    return eval_integer_infix_expression(operator, left, right);
  }

  if (strncmp(operator, "==", 2) == 0) {
    return left->type == right->type && left->type == OBJECT_BOOLEAN
               ? new_boolean_object(left->value.boolean->value ==
                                    right->value.boolean->value)
               : new_boolean_object(true); // both objects are null
  } else if (strncmp(operator, "!=", 2) == 0) {
    if (left->type != right->type) {
      return new_boolean_object(true);
    }
    if (left->type == OBJECT_NULL) { // both objects are null
      return new_boolean_object(false);
    }
    return new_boolean_object(left->value.boolean->value !=
                              right->value.boolean->value);
  }
  return NULL;
}

object_t *eval_expression(expression_t *e) {
  switch (e->type) {
  case EXPRESSION_INTEGER_LITERAL: {
    return new_integer_object(e->value.integer->value);
  }
  case EXPRESSION_BOOLEAN_LITERAL: {
    return new_boolean_object(e->value.boolean->value);
  }
  case EXPRESSION_PREFIX: {
    object_t *right = eval_expression(e->value.prefix->right);
    object_t *out = eval_prefix_expression(e->value.prefix->op, right);
    object_free(right);
    return out;
  }
  case EXPRESSION_INFIX: {
    object_t *left = eval_expression(e->value.infix->left);
    object_t *right = eval_expression(e->value.infix->right);
    object_t *out = eval_infix_expression(e->value.infix->op, left, right);
    object_free(left);
    object_free(right);
    return out;
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
