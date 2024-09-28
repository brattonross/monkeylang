#include "eval.h"
#include "ast.h"
#include "object.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool is_error(object_t *o) {
  if (o != NULL) {
    return o->type == OBJECT_ERROR;
  }
  return false;
}

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

object_t *new_error_object(char *fmt, ...) {
  object_t *o = malloc(sizeof(object_t));
  if (o == NULL) {
    return NULL;
  }
  o->type = OBJECT_ERROR;
  o->value.err = malloc(sizeof(error_object_t));
  if (o->value.err == NULL) {
    free(o);
    return NULL;
  }
  va_list args;
  va_start(args, fmt);
  size_t len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  va_start(args, fmt);
  char *buf = malloc(len + 1);
  vsnprintf(buf, len + 1, fmt, args);
  va_end(args);
  o->value.err->message = buf;
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
    return new_error_object("unknown operator: -%s",
                            object_type_to_string(right->type));
  }
  return new_integer_object(-right->value.integer->value);
}

object_t *eval_prefix_expression(const char *operator, const object_t * right) {
  if (strncmp(operator, "!", 1) == 0) {
    return eval_bang_operator_expression(right);
  } else if (strncmp(operator, "-", 1) == 0) {
    return eval_minus_prefix_operator_expression(right);
  }
  return new_error_object("unknown operator: %s%s", operator,
                          object_type_to_string(right->type));
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
  return new_error_object("unknown operator: %s %s %s",
                          object_type_to_string(left->type), operator,
                          object_type_to_string(right->type));
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

  if (left->type != right->type) {
    return new_error_object("type mismatch: %s %s %s",
                            object_type_to_string(left->type), operator,
                            object_type_to_string(right->type));
  }
  return new_error_object("unknown operator: %s %s %s",
                          object_type_to_string(left->type), operator,
                          object_type_to_string(right->type));
}

bool is_truthy(object_t *condition) {
  if (condition->type == OBJECT_NULL) {
    return false;
  }
  if (condition->type == OBJECT_BOOLEAN) {
    return condition->value.boolean->value;
  }
  return true;
}

object_t *eval_block_statement(block_statement_t *block);
object_t *eval_if_expression(if_expression_t *e) {
  object_t *condition = eval_expression(e->condition);
  if (is_error(condition)) {
    return condition;
  }
  if (is_truthy(condition)) {
    return eval_block_statement(e->consequence);
  }
  if (e->alternative != NULL) {
    return eval_block_statement(e->alternative);
  }
  object_t *null_object = malloc(sizeof(object_t));
  if (null_object == NULL) {
    return NULL;
  }
  null_object->type = OBJECT_NULL;
  null_object->value.boolean = NULL;
  null_object->value.integer = NULL;
  return null_object;
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
    if (is_error(right)) {
      return right;
    }
    object_t *out = eval_prefix_expression(e->value.prefix->op, right);
    object_free(right);
    return out;
  }
  case EXPRESSION_INFIX: {
    object_t *left = eval_expression(e->value.infix->left);
    if (is_error(left)) {
      return left;
    }
    object_t *right = eval_expression(e->value.infix->right);
    if (is_error(right)) {
      object_free(left);
      return right;
    }
    object_t *out = eval_infix_expression(e->value.infix->op, left, right);
    object_free(left);
    object_free(right);
    return out;
  }
  case EXPRESSION_IF: {
    return eval_if_expression(e->value.if_);
  }
  default:
    return NULL;
  }
}

object_t *eval_block_statement(block_statement_t *block) {
  object_t *result = NULL;
  for (size_t i = 0; i < block->statements_len; ++i) {
    result = eval_statement(block->statements[i]);
    if (result != NULL &&
        (result->type == OBJECT_RETURN || result->type == OBJECT_ERROR)) {
      return result;
    }
  }
  return result;
}

object_t *eval_statement(statement_t *s) {
  switch (s->type) {
  case STATEMENT_EXPRESSION:
    return eval_expression(s->value.exp->expression);
  case STATEMENT_BLOCK:
    return eval_block_statement(s->value.block);
  case STATEMENT_RETURN:
    object_t *val = eval_expression(s->value.ret->value);
    if (is_error(val)) {
      return val;
    }
    object_t *ret = malloc(sizeof(object_t));
    if (ret == NULL) {
      free(val);
      return NULL;
    }
    ret->type = OBJECT_RETURN;
    ret->value.return_value = malloc(sizeof(return_value_t));
    if (ret->value.return_value == NULL) {
      free(val);
      free(ret);
      return NULL;
    }
    ret->value.return_value->value = val;
    return ret;
  default:
    return NULL;
  }
}

object_t *eval_program(program_t *p) {
  object_t *result = NULL;
  for (size_t i = 0; i < p->statements_len; ++i) {
    result = eval_statement(p->statements[i]);
    if (result->type == OBJECT_RETURN) {
      return result->value.return_value->value;
    } else if (result->type == OBJECT_ERROR) {
      return result;
    }
  }
  return result;
}
