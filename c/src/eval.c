#include "eval.h"
#include "ast.h"
#include "object.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_error(object_t *o) {
  if (o != NULL) {
    return o->type == OBJECT_ERROR;
  }
  return false;
}

object_t *new_boolean_object(bool value) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_BOOLEAN;
  obj->value.boolean = malloc(sizeof(boolean_object_t));
  if (obj->value.boolean == NULL) {
    object_free(obj);
    return NULL;
  }
  obj->value.boolean->value = value;
  return obj;
}

object_t *new_integer_object(int64_t value) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_INTEGER;
  obj->value.integer = malloc(sizeof(integer_object_t));
  if (obj->value.integer == NULL) {
    object_free(obj);
    return NULL;
  }
  obj->value.integer->value = value;
  return obj;
}

object_t *new_error_object(char *fmt, ...) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_ERROR;
  obj->value.err = malloc(sizeof(error_object_t));
  if (obj->value.err == NULL) {
    object_free(obj);
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
  obj->value.err->message = buf;
  return obj;
}

object_t *new_function_object(size_t parameters_len, identifier_t **parameters,
                              environment_t *env, block_statement_t *body) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }

  obj->type = OBJECT_FUNCTION;
  obj->value.fn = malloc(sizeof(function_object_t));
  if (obj->value.fn == NULL) {
    object_free(obj);
    return NULL;
  }

  obj->value.fn->parameters_len = parameters_len;
  obj->value.fn->parameters = parameters;
  obj->value.fn->env = env;
  obj->value.fn->body = body;
  return obj;
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

object_t *eval_block_statement(block_statement_t *block, environment_t *env);
object_t *eval_if_expression(if_expression_t *e, environment_t *env) {
  object_t *condition = eval_expression(e->condition, env);
  if (is_error(condition)) {
    return condition;
  }
  if (is_truthy(condition)) {
    return eval_block_statement(e->consequence, env);
  }
  if (e->alternative != NULL) {
    return eval_block_statement(e->alternative, env);
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

object_t *eval_identifier(identifier_t *ident, environment_t *env) {
  object_t *val = environment_get(env, ident->value);
  if (val == NULL) {
    return new_error_object("identifier not found: %s", ident->value);
  }
  return val;
}

object_t *eval_expression(expression_t *e, environment_t *env) {
  switch (e->type) {
  case EXPRESSION_INTEGER_LITERAL: {
    return new_integer_object(e->value.integer->value);
  }
  case EXPRESSION_BOOLEAN_LITERAL: {
    return new_boolean_object(e->value.boolean->value);
  }
  case EXPRESSION_FUNCTION_LITERAL: {
    return new_function_object(e->value.fn->parameters_len,
                               e->value.fn->parameters, env, e->value.fn->body);
  }
  case EXPRESSION_PREFIX: {
    object_t *right = eval_expression(e->value.prefix->right, env);
    if (is_error(right)) {
      return right;
    }
    object_t *out = eval_prefix_expression(e->value.prefix->op, right);
    object_free(right);
    return out;
  }
  case EXPRESSION_INFIX: {
    object_t *left = eval_expression(e->value.infix->left, env);
    if (is_error(left)) {
      return left;
    }
    object_t *right = eval_expression(e->value.infix->right, env);
    if (is_error(right)) {
      object_free(left);
      return right;
    }
    object_t *out = eval_infix_expression(e->value.infix->op, left, right);
    // TODO: This causes a double free?
    // object_free(left);
    // object_free(right);
    return out;
  }
  case EXPRESSION_IF: {
    return eval_if_expression(e->value.if_, env);
  }
  case EXPRESSION_IDENTIFIER:
    return eval_identifier(e->value.ident, env);
  default:
    return NULL;
  }
}

object_t *eval_block_statement(block_statement_t *block, environment_t *env) {
  object_t *result = NULL;
  for (size_t i = 0; i < block->statements_len; ++i) {
    result = eval_statement(block->statements[i], env);
    if (result != NULL &&
        (result->type == OBJECT_RETURN || result->type == OBJECT_ERROR)) {
      return result;
    }
  }
  return result;
}

object_t *eval_statement(statement_t *s, environment_t *env) {
  switch (s->type) {
  case STATEMENT_EXPRESSION:
    return eval_expression(s->value.exp->expression, env);
  case STATEMENT_BLOCK:
    return eval_block_statement(s->value.block, env);
  case STATEMENT_RETURN: {
    object_t *val = eval_expression(s->value.ret->value, env);
    if (is_error(val)) {
      return val;
    }
    object_t *ret = malloc(sizeof(object_t));
    if (ret == NULL) {
      object_free(val);
      return NULL;
    }
    ret->type = OBJECT_RETURN;
    ret->value.return_value = malloc(sizeof(return_value_t));
    if (ret->value.return_value == NULL) {
      object_free(val);
      object_free(ret);
      return NULL;
    }
    ret->value.return_value->value = val;
    return ret;
  } break;
  case STATEMENT_LET: {
    object_t *val = eval_expression(s->value.let->value, env);
    if (is_error(val)) {
      return val;
    }
    environment_set(env, s->value.let->ident->value, val);
    return NULL;
  } break;
  default:
    return NULL;
  }
}

object_t *eval_program(program_t *p, environment_t *env) {
  object_t *result = NULL;
  for (size_t i = 0; i < p->statements_len; ++i) {
    result = eval_statement(p->statements[i], env);
    if (result == NULL) {
      continue;
    }

    if (result->type == OBJECT_RETURN) {
      return result->value.return_value->value;
    } else if (result->type == OBJECT_ERROR) {
      return result;
    }
  }
  return result;
}
