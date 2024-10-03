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

object_t *eval_string_infix_expression(const char *operator,
                                       const object_t * left,
                                       const object_t *right) {
  if (strncmp(operator, "+", 1) != 0) {
    return new_error_object("unknown operator: %s %s %s",
                            object_type_to_string(left->type), operator,
                            object_type_to_string(right->type));
  }

  char *left_value = left->value.string->value;
  char *right_value = right->value.string->value;
  int len = strlen(left_value) + strlen(right_value) + 1;
  char *buf = malloc(len + 1);
  if (buf == NULL) {
    return NULL;
  }
  snprintf(buf, len, "%s%s", left_value, right_value);
  return new_string_object(buf);
}

object_t *eval_infix_expression(const char *operator, const object_t * left,
                                const object_t *right) {
  if (left->type == OBJECT_INTEGER && right->type == OBJECT_INTEGER) {
    return eval_integer_infix_expression(operator, left, right);
  }

  if (left->type == OBJECT_STRING && right->type == OBJECT_STRING) {
    return eval_string_infix_expression(operator, left, right);
  }

  if (strncmp(operator, "==", 2) == 0) {
    return left->type == right->type && left->type == OBJECT_BOOLEAN
               ? new_boolean_object(left->value.boolean->value ==
                                    right->value.boolean->value)
               : new_boolean_object(true); // both objects are null
  }

  if (strncmp(operator, "!=", 2) == 0) {
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
  return new_null_object();
}

object_t *eval_identifier(identifier_t *ident, environment_t *env) {
  object_t *val = environment_get(env, ident->value);
  if (val != NULL) {
    return val;
  }
  object_t *builtin = lookup_builtin(ident->value);
  if (builtin != NULL) {
    return builtin;
  }
  return new_error_object("identifier not found: %s", ident->value);
}

object_t **eval_expressions(size_t argc, expression_t **argv,
                            environment_t *env) {
  object_t **args = calloc(argc, sizeof(object_t));
  if (args == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < argc; ++i) {
    object_t *evaluated = eval_expression(argv[i], env);
    if (is_error(evaluated)) {
      args = realloc(args, 1 * sizeof(object_t));
      args[0] = evaluated;
    }
    args[i] = evaluated;
  }
  return args;
}

environment_t *extend_function_env(function_object_t *fn, size_t argc,
                                   object_t **argv) {
  environment_t *env = new_enclosed_environment(fn->env);
  for (size_t i = 0; i < argc; ++i) {
    environment_set(env, fn->parameters[i]->value, argv[i]);
  }
  return env;
}

object_t *unwrap_return_value(object_t *obj) {
  if (obj == NULL) {
    return NULL;
  }
  if (obj->type == OBJECT_RETURN) {
    return obj->value.return_value->value;
  }
  return obj;
}

object_t *apply_function(object_t *fn, size_t argc, object_t **argv) {
  if (fn->type == OBJECT_FUNCTION) {
    environment_t *extended_env = extend_function_env(fn->value.fn, argc, argv);
    object_t *evaluated =
        eval_block_statement(fn->value.fn->body, extended_env);
    return unwrap_return_value(evaluated);
  }

  if (fn->type == OBJECT_BUILTIN) {
    return fn->value.builtin->fn(argc, argv);
  }

  return new_error_object("not a function: %s",
                          object_type_to_string(fn->type));
}

object_t *eval_array_index_expression(object_t *left, object_t *index) {
  array_object_t *arr = left->value.array;
  integer_object_t *idx = index->value.integer;
  // TODO: cast here could cause mischief?
  if (idx->value < 0 || idx->value > (int64_t)(arr->len - 1)) {
    return new_null_object();
  }
  return arr->elements[idx->value];
}

object_t *eval_index_expression(object_t *left, object_t *index) {
  if (left->type == OBJECT_ARRAY && index->type == OBJECT_INTEGER) {
    return eval_array_index_expression(left, index);
  }
  return new_error_object("index operator not supported: %s",
                          object_type_to_string(left->type));
}

object_t *eval_hash_literal(hash_literal_t *hash, environment_t *env) {
  hash_object_item_t **pairs = calloc(hash->len, sizeof(hash_object_item_t));
  if (pairs == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < hash->len; ++i) {
    object_t *key = eval_expression(hash->pairs[i]->key, env);
    if (is_error(key)) {
      hash_object_items_free(hash->len, pairs);
      return key;
    }

    hash_key_t *hash_key = object_hash_key(key);
    if (hash_key == NULL) {
      hash_object_items_free(hash->len, pairs);
      object_free(key);
      return new_error_object("unusable as hash key: %s",
                              object_type_to_string(key->type));
    }

    object_t *value = eval_expression(hash->pairs[i]->value, env);
    if (is_error(value)) {
      hash_object_items_free(hash->len, pairs);
      object_free(key);
      return value;
    }

    hash_object_item_t *item = malloc(sizeof(hash_object_item_t));
    if (item == NULL) {
      hash_object_items_free(hash->len, pairs);
      object_free(key);
      hash_key_free(hash_key);
      object_free(value);
      return NULL;
    }

    item->hash_key = hash_key;
    item->key = key;
    item->value = value;

    pairs[i] = item;
  }

  return new_hash_object(hash->len, pairs);
}

object_t *eval_expression(expression_t *e, environment_t *env) {
  switch (e->type) {
  case EXPRESSION_INTEGER_LITERAL:
    return new_integer_object(e->value.integer->value);
  case EXPRESSION_BOOLEAN_LITERAL:
    return new_boolean_object(e->value.boolean->value);
  case EXPRESSION_FUNCTION_LITERAL:
    return new_function_object(e->value.fn->parameters_len,
                               e->value.fn->parameters, env, e->value.fn->body);
  case EXPRESSION_STRING:
    return new_string_object(e->value.string->value);
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
  case EXPRESSION_CALL: {
    object_t *fn = eval_expression(e->value.call->fn, env);
    if (is_error(fn)) {
      return fn;
    }
    // NOTE: This assumes that we always return either 1 error arg, or a
    // matching number of evaluated args.
    object_t **argv =
        eval_expressions(e->value.call->argc, e->value.call->argv, env);
    if (argv == NULL) {
      return NULL;
    }
    if (e->value.call->argc > 0 && is_error(argv[0])) {
      return argv[0];
    }
    return apply_function(fn, e->value.call->argc, argv);
  }
  case EXPRESSION_ARRAY_LITERAL: {
    object_t **elements =
        eval_expressions(e->value.arr->len, e->value.arr->elements, env);
    if (is_error(elements[0])) {
      return elements[0];
    }
    return new_array_object(e->value.arr->len, elements);
  }
  case EXPRESSION_INDEX: {
    object_t *left = eval_expression(e->value.index->left, env);
    if (is_error(left)) {
      return left;
    }
    object_t *index = eval_expression(e->value.index->index, env);
    if (is_error(index)) {
      free(left);
      return index;
    }
    return eval_index_expression(left, index);
  }
  case EXPRESSION_HASH:
    return eval_hash_literal(e->value.hash, env);
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
