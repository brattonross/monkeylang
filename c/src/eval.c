#pragma once

#include "ast.c"
#include "object.c"
#include "string.c"
#include <stdio.h>

void eval_statement(Statement *statement, Object *result);
void eval_expression(Expression *expression, Object *result);
void eval_prefix_expression(String op, Object *result);
void eval_bang_operator_expression(Object *result);
void eval_minus_prefix_operator_expression(Object *result);

void eval_program(Program *program, Object *result) {
  StatementIterator iter = {0};
  statement_iterator_init(&iter, program->first_chunk);

  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    eval_statement(s, result);
  }
}

void eval_statement(Statement *statement, Object *result) {
  switch (statement->type) {
  case STATEMENT_EXPRESSION:
    eval_expression(statement->data.expression_statement.expression, result);
    break;
  default:
    fprintf(stderr, "eval_statement: unhandled statement type %.*s\n",
            (int)statement_type_strings[statement->type].length,
            statement_type_strings[statement->type].buffer);
    break;
  }
}

void eval_expression(Expression *expression, Object *result) {
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
    eval_expression(expression->data.prefix.right, result);
    eval_prefix_expression(expression->data.prefix.op, result);
  } break;
  default:
    fprintf(stderr, "eval_expression: unhandled expression type %.*s\n",
            (int)expression_type_strings[expression->type].length,
            expression_type_strings[expression->type].buffer);
    break;
  }
}

void eval_prefix_expression(String op, Object *result) {
  if (string_cmp(op, String("!"))) {
    eval_bang_operator_expression(result);
  } else if (string_cmp(op, String("-"))) {
    eval_minus_prefix_operator_expression(result);
  } else {
    result->type = OBJECT_NULL;
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
    result->type = OBJECT_NULL;
    result->data = (ObjectData){0};
  }

  result->data.integer.value = -result->data.integer.value;
}
