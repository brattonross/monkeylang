#pragma once

#include "ast.c"
#include "object.c"
#include <stdio.h>

void eval_statement(Statement *statement, Object *result);
void eval_expression(Expression *expression, Object *result);

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
  default:
    fprintf(stderr, "eval_expression: unhandled expression type %.*s\n",
            (int)expression_type_strings[expression->type].length,
            expression_type_strings[expression->type].buffer);
    break;
  }
}
