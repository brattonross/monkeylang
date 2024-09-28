#ifndef __EVAL_H__
#define __EVAL_H__

#include "ast.h"
#include "object.h"

object_t *eval_prefix_expression(const char *operator, const object_t * right);
object_t *eval_expression(expression_t *e);
object_t *eval_statement(statement_t *s);
object_t *eval_program(program_t *p);

#endif
