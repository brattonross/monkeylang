#pragma once

#include "ast.c"
#include "object.c"
#include "string.c"
#include <stdint.h>
#include <stdio.h>

void eval_statement(Object *result, Statement *statement);
void eval_expression(Object *result, Expression *expression);
void eval_prefix_expression(Object *result, String op);
void eval_bang_operator_expression(Object *result);
void eval_minus_prefix_operator_expression(Object *result);
void eval_infix_expression(Object *result, String op, Object left,
                           Object right);
void eval_integer_infix_expression(Object *result, String op, Object left,
                                   Object right);
void eval_boolean_infix_expression(Object *result, String op, Object left,
                                   Object right);
void eval_null_infix_expression(Object *result, String op);
void eval_block_statement(Object *result, BlockStatement *block);

bool object_is_truthy(Object o);

void eval_program(Program *program, Object *result) {
  StatementIterator iter = {0};
  statement_iterator_init(&iter, program->first_chunk);

  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    eval_statement(result, s);
  }
}

void eval_statement(Object *result, Statement *statement) {
  switch (statement->type) {
  case STATEMENT_EXPRESSION:
    eval_expression(result, statement->data.expression_statement.expression);
    break;
  default:
    fprintf(stderr, "eval_statement: unhandled statement type %.*s\n",
            (int)statement_type_strings[statement->type].length,
            statement_type_strings[statement->type].buffer);
    break;
  }
}

void eval_expression(Object *result, Expression *expression) {
  switch (expression->type) {
  case EXPRESSION_INTEGER: {
    result->type = OBJECT_INTEGER;
    result->data.integer.value = expression->data.integer.value;
  } break;
  case EXPRESSION_BOOLEAN: {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = expression->data.boolean.value;
  } break;
  case EXPRESSION_PREFIX: {
    eval_expression(result, expression->data.prefix.right);
    eval_prefix_expression(result, expression->data.prefix.op);
  } break;
  case EXPRESSION_INFIX: {
    Object left = {0};
    eval_expression(&left, expression->data.infix.left);
    Object right = {0};
    eval_expression(&right, expression->data.infix.right);
    eval_infix_expression(result, expression->data.infix.op, left, right);
  } break;
  case EXPRESSION_IF: {
    IfExpression ie = expression->data.if_expression;
    Object condition = {0};
    eval_expression(&condition, ie.condition);

    if (object_is_truthy(condition)) {
      eval_block_statement(result, ie.consequence);
    } else if (ie.alternative) {
      eval_block_statement(result, ie.alternative);
    } else {
      null_object(result);
    }
  } break;
  default:
    fprintf(stderr, "eval_expression: unhandled expression type %.*s\n",
            (int)expression_type_strings[expression->type].length,
            expression_type_strings[expression->type].buffer);
    break;
  }
}

void eval_prefix_expression(Object *result, String op) {
  if (string_cmp(op, String("!"))) {
    eval_bang_operator_expression(result);
  } else if (string_cmp(op, String("-"))) {
    eval_minus_prefix_operator_expression(result);
  } else {
    null_object(result);
  }
}

void eval_bang_operator_expression(Object *result) {
  switch (result->type) {
  case OBJECT_BOOLEAN:
    result->data.boolean.value = !result->data.boolean.value;
    break;
  case OBJECT_NULL:
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = true;
    break;
  case OBJECT_INTEGER:
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = false;
    break;
  }
}

void eval_minus_prefix_operator_expression(Object *result) {
  if (result->type != OBJECT_INTEGER) {
    null_object(result);
  } else {
    result->data.integer.value = -result->data.integer.value;
  }
}

void eval_infix_expression(Object *result, String op, Object left,
                           Object right) {
  if (left.type == right.type) {
    // exhaustive switch covers all object types
    switch (left.type) {
    case OBJECT_INTEGER:
      eval_integer_infix_expression(result, op, left, right);
      break;
    case OBJECT_BOOLEAN:
      eval_boolean_infix_expression(result, op, left, right);
      break;
    case OBJECT_NULL:
      eval_null_infix_expression(result, op);
      break;
    }
  } else {
    // mismatching types
    if (string_cmp(op, String("=="))) {
      result->type = OBJECT_BOOLEAN;
      result->data.boolean.value = false;
    } else if (string_cmp(op, String("!="))) {
      result->type = OBJECT_BOOLEAN;
      result->data.boolean.value = true;
    } else {
      null_object(result);
    }
  }
}

void eval_integer_infix_expression(Object *result, String op, Object left,
                                   Object right) {
  int64_t left_value = left.data.integer.value;
  int64_t right_value = right.data.integer.value;

  if (string_cmp(op, String("+"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer.value = left_value + right_value;
  } else if (string_cmp(op, String("-"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer.value = left_value - right_value;
  } else if (string_cmp(op, String("*"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer.value = left_value * right_value;
  } else if (string_cmp(op, String("/"))) {
    result->type = OBJECT_INTEGER;
    result->data.integer.value = left_value / right_value;
  } else if (string_cmp(op, String("<"))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value < right_value;
  } else if (string_cmp(op, String(">"))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value > right_value;
  } else if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value == right_value;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value != right_value;
  } else {
    null_object(result);
  }
}

void eval_boolean_infix_expression(Object *result, String op, Object left,
                                   Object right) {
  bool left_value = left.data.boolean.value;
  bool right_value = right.data.boolean.value;

  if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value == right_value;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = left_value != right_value;
  } else {
    null_object(result);
  }
}

void eval_null_infix_expression(Object *result, String op) {
  if (string_cmp(op, String("=="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = true;
  } else if (string_cmp(op, String("!="))) {
    result->type = OBJECT_BOOLEAN;
    result->data.boolean.value = false;
  } else {
    null_object(result);
  }
}

void eval_block_statement(Object *result, BlockStatement *block) {
  StatementIterator iter = {0};
  statement_iterator_init(&iter, block->first_chunk);

  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    eval_statement(result, s);
  }
}

bool object_is_truthy(Object o) {
  switch (o.type) {
  case OBJECT_NULL:
    return false;
  case OBJECT_BOOLEAN:
    return o.data.boolean.value;
  default:
    return true;
  }
}
