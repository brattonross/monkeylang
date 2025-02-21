#pragma once

#include "ast.c"
#include "env.c"
#include "mem.c"
#include "object.c"
#include "string.c"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void eval_statement(Arena *arena, Arena *env_arena, Environment *env,
                    Object *result, Statement *statement);
void eval_expression(Arena *arena, Arena *env_arena, Environment *env,
                     Object *result, Expression *expression);
void eval_prefix_expression(Arena *arena, Object *result, String op);
void eval_bang_operator_expression(Object *result);
void eval_minus_prefix_operator_expression(Arena *arena, Object *result);
void eval_infix_expression(Arena *arena, Object *result, String op, Object left,
                           Object right);
void eval_integer_infix_expression(Arena *arena, Object *result, String op,
                                   Object left, Object right);
void eval_boolean_infix_expression(Arena *arena, Object *result, String op,
                                   Object left, Object right);
void eval_null_infix_expression(Arena *arena, Object *result, String op);
void eval_block_statement(Arena *arena, Arena *env_arena, Environment *env,
                          Object *result, BlockStatement *block);
void error_object(Object *result, String message);

bool object_is_truthy(Object o);

void eval_program(Program *program, Arena *arena, Arena *env_arena,
                  Environment *env, Object *result) {
  StatementIterator iter = {0};
  statement_iterator_init(&iter, program->first_chunk);

  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    eval_statement(arena, env_arena, env, result, s);
    if (result->type == OBJECT_RETURN) {
      memcpy(result, result->data.return_object.value, sizeof(Object));
      break;
    } else if (result->type == OBJECT_ERROR) {
      break;
    }
  }
}

void eval_statement(Arena *arena, Arena *env_arena, Environment *env,
                    Object *result, Statement *statement) {
  switch (statement->type) {
  case STATEMENT_EXPRESSION:
    eval_expression(arena, env_arena, env, result,
                    statement->data.expression_statement.expression);
    break;
  case STATEMENT_RETURN: {
    Object *value = arena_alloc(arena, sizeof(Object));
    eval_expression(arena, env_arena, env, value,
                    statement->data.return_statement.return_value);
    if (value->type == OBJECT_ERROR) {
      memcpy(result, value, sizeof(Object));
    } else {
      result->type = OBJECT_RETURN;
      result->data.return_object.value = value;
    }
  } break;
  case STATEMENT_LET: {
    eval_expression(arena, env_arena, env, result,
                    statement->data.let_statement.value);
    if (result->type == OBJECT_ERROR) {
      break;
    }
    environment_set(env, env_arena, statement->data.let_statement.name->value,
                    result);
  } break;
  default:
    fprintf(stderr, "eval_statement: unhandled statement type %.*s\n",
            (int)statement_type_strings[statement->type].length,
            statement_type_strings[statement->type].buffer);
    break;
  }
}

void eval_expression(Arena *arena, Arena *env_arena, Environment *env,
                     Object *result, Expression *expression) {
  switch (expression->type) {
  case EXPRESSION_INTEGER: {
    result->type = OBJECT_INTEGER;
    result->data.integer_object.value = expression->data.integer.value;
  } break;
  case EXPRESSION_BOOLEAN: {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = expression->data.boolean.value;
  } break;
  case EXPRESSION_PREFIX: {
    eval_expression(arena, env_arena, env, result,
                    expression->data.prefix.right);
    if (result->type == OBJECT_ERROR) {
      break;
    }
    eval_prefix_expression(arena, result, expression->data.prefix.op);
  } break;
  case EXPRESSION_INFIX: {
    Object left = {0};
    eval_expression(arena, env_arena, env, &left, expression->data.infix.left);
    if (left.type == OBJECT_ERROR) {
      memcpy(result, &left, sizeof(Object));
      break;
    }

    Object right = {0};
    eval_expression(arena, env_arena, env, &right,
                    expression->data.infix.right);
    if (right.type == OBJECT_ERROR) {
      memcpy(result, &right, sizeof(Object));
      break;
    }

    eval_infix_expression(arena, result, expression->data.infix.op, left,
                          right);
  } break;
  case EXPRESSION_IF: {
    IfExpression ie = expression->data.if_expression;
    Object condition = {0};
    eval_expression(arena, env_arena, env, &condition, ie.condition);
    if (condition.type == OBJECT_ERROR) {
      memcpy(result, &condition, sizeof(Object));
      break;
    }

    if (object_is_truthy(condition)) {
      eval_block_statement(arena, env_arena, env, result, ie.consequence);
    } else if (ie.alternative) {
      eval_block_statement(arena, env_arena, env, result, ie.alternative);
    } else {
      null_object(result);
    }
  } break;
  case EXPRESSION_IDENTIFIER: {
    Object *value = environment_get(env, expression->data.identifier.value);
    if (value) {
      memcpy(result, value, sizeof(Object));
    } else {
      error_object(result,
                   string_fmt(arena, "identifier not found: %.*s",
                              expression->data.identifier.value.length,
                              expression->data.identifier.value.buffer));
    }
  } break;
  default:
    fprintf(stderr, "eval_expression: unhandled expression type %.*s\n",
            (int)expression_type_strings[expression->type].length,
            expression_type_strings[expression->type].buffer);
    break;
  }
}

void eval_prefix_expression(Arena *arena, Object *result, String op) {
  if (string_cmp(op, String("!"))) {
    eval_bang_operator_expression(result);
  } else if (string_cmp(op, String("-"))) {
    eval_minus_prefix_operator_expression(arena, result);
  } else {
    String right_type = object_type_strings[result->type];
    error_object(result,
                 string_fmt(arena, "unknown operator: %.*s%.*s", op.length,
                            op.buffer, right_type.length, right_type.length));
  }
}

void eval_bang_operator_expression(Object *result) {
  switch (result->type) {
  case OBJECT_BOOLEAN:
    result->data.boolean_object.value = !result->data.boolean_object.value;
    break;
  case OBJECT_NULL:
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = true;
    break;
  default:
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = false;
    break;
  }
}

void eval_minus_prefix_operator_expression(Arena *arena, Object *result) {
  if (result->type != OBJECT_INTEGER) {
    String type_str = object_type_strings[result->type];
    error_object(result, string_fmt(arena, "unknown operator: -%.*s",
                                    type_str.length, type_str.buffer));
  } else {
    result->data.integer_object.value = -result->data.integer_object.value;
  }
}

void eval_infix_expression(Arena *arena, Object *result, String op, Object left,
                           Object right) {
  if (left.type == right.type) {
    // exhaustive switch covers all object types
    switch (left.type) {
    case OBJECT_INTEGER:
      eval_integer_infix_expression(arena, result, op, left, right);
      break;
    case OBJECT_BOOLEAN:
      eval_boolean_infix_expression(arena, result, op, left, right);
      break;
    case OBJECT_NULL:
      eval_null_infix_expression(arena, result, op);
      break;
    default: {
      String left_type = object_type_strings[left.type];
      String right_type = object_type_strings[right.type];
      error_object(result,
                   string_fmt(arena, "unknown operator: %.*s %.*s %.*s",
                              left_type.length, left_type.buffer, op.length,
                              op.buffer, right_type.length, right_type.buffer));
    } break;
    }
  } else {
    String left_type = object_type_strings[left.type];
    String right_type = object_type_strings[right.type];
    error_object(result,
                 string_fmt(arena, "type mismatch: %.*s %.*s %.*s",
                            left_type.length, left_type.buffer, op.length,
                            op.buffer, right_type.length, right_type.buffer));
  }
}

void eval_integer_infix_expression(Arena *arena, Object *result, String op,
                                   Object left, Object right) {
  int64_t left_value = left.data.integer_object.value;
  int64_t right_value = right.data.integer_object.value;

  if (string_cmp(op, String("+"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer_object.value = left_value + right_value;
  } else if (string_cmp(op, String("-"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer_object.value = left_value - right_value;
  } else if (string_cmp(op, String("*"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer_object.value = left_value * right_value;
  } else if (string_cmp(op, String("/"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer_object.value = left_value / right_value;
  } else if (string_cmp(op, String("<"))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value < right_value;
  } else if (string_cmp(op, String(">"))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value > right_value;
  } else if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value == right_value;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value != right_value;
  } else {
    String left_type = object_type_strings[left.type];
    String right_type = object_type_strings[right.type];
    error_object(result,
                 string_fmt(arena, "unknown operator: %.*s %.*s %.*s",
                            left_type.length, left_type.buffer, op.length,
                            op.buffer, right_type.length, right_type.buffer));
  }
}

void eval_boolean_infix_expression(Arena *arena, Object *result, String op,
                                   Object left, Object right) {
  bool left_value = left.data.boolean_object.value;
  bool right_value = right.data.boolean_object.value;

  if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value == right_value;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = left_value != right_value;
  } else {
    String left_type = object_type_strings[left.type];
    String right_type = object_type_strings[right.type];
    error_object(result,
                 string_fmt(arena, "unknown operator: %.*s %.*s %.*s",
                            left_type.length, left_type.buffer, op.length,
                            op.buffer, right_type.length, right_type.buffer));
  }
}

void eval_null_infix_expression(Arena *arena, Object *result, String op) {
  if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = true;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean_object.value = false;
  } else {
    error_object(result, string_fmt(arena, "unknown operator: null %.*s null",
                                    op.length, op.buffer));
  }
}

void eval_block_statement(Arena *arena, Arena *env_arena, Environment *env,
                          Object *result, BlockStatement *block) {
  StatementIterator iter = {0};
  statement_iterator_init(&iter, block->first_chunk);

  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    eval_statement(arena, env_arena, env, result, s);
    if (result->type == OBJECT_RETURN || result->type == OBJECT_ERROR) {
      break;
    }
  }
}

bool object_is_truthy(Object o) {
  switch (o.type) {
  case OBJECT_NULL:
    return false;
  case OBJECT_BOOLEAN:
    return o.data.boolean_object.value;
  default:
    return true;
  }
}

void error_object(Object *result, String message) {
  result->type = OBJECT_ERROR;
  result->data.error_object.message = message;
}
