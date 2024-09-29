#include "parser.h"
#include "array_list.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <string.h>

expression_t *parser_parse_expression(parser_t *p,
                                      parser_precedence_t precedence);
parser_error_t parser_register_prefix(parser_t *p, token_type_t t,
                                      prefix_parse_fn fn);
parser_error_t parser_register_infix(parser_t *p, token_type_t t,
                                     infix_parse_fn fn);
expression_t *parser_parse_identifier(parser_t *p);
expression_t *parser_parse_integer_literal(parser_t *p);
expression_t *parser_parse_prefix_expression(parser_t *p);
expression_t *parser_parse_infix_expression(parser_t *p, expression_t *e);
expression_t *parser_parse_boolean_literal(parser_t *p);
expression_t *parser_parse_grouped_expression(parser_t *p);
expression_t *parser_parse_if_expression(parser_t *p);
expression_t *parser_parse_function_literal(parser_t *p);
expression_t *parser_parse_call_expression(parser_t *p, expression_t *e);
expression_t *parser_parse_string_literal(parser_t *p);
expression_t *parser_parse_array_literal(parser_t *p);

void parser_next_token(parser_t *p) {
  p->current_token = p->peek_token;
  p->peek_token = lexer_next_token(p->lexer);
}

parser_t *parser_init(lexer_t *l) {
  if (l == NULL) {
    return NULL;
  }
  parser_t *p = malloc(sizeof(parser_t));
  if (p == NULL) {
    return NULL;
  }
  p->lexer = l;
  p->current_token = NULL;
  p->peek_token = NULL;
  p->errors = array_list_create(1);

  parser_next_token(p);
  parser_next_token(p);

  return p;
}

void parser_free(parser_t *p) {
  lexer_free(p->lexer);
  array_list_free(p->errors);
  token_free(p->current_token);
  token_free(p->peek_token);
}

prefix_parse_fn parser_prefix_fn(token_type_t t) {
  switch (t) {
  case TOKEN_IDENTIFIER:
    return parser_parse_identifier;
  case TOKEN_INT:
    return parser_parse_integer_literal;
  case TOKEN_BANG:
  case TOKEN_MINUS:
    return parser_parse_prefix_expression;
  case TOKEN_TRUE:
  case TOKEN_FALSE:
    return parser_parse_boolean_literal;
  case TOKEN_LEFT_PAREN:
    return parser_parse_grouped_expression;
  case TOKEN_IF:
    return parser_parse_if_expression;
  case TOKEN_FUNCTION:
    return parser_parse_function_literal;
  case TOKEN_STRING:
    return parser_parse_string_literal;
  case TOKEN_LEFT_BRACKET:
    return parser_parse_array_literal;
  default:
    return NULL;
  }
}

infix_parse_fn parser_infix_fn(token_type_t t) {
  switch (t) {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_SLASH:
  case TOKEN_ASTERISK:
  case TOKEN_EQUAL:
  case TOKEN_NOT_EQUAL:
  case TOKEN_LESS_THAN:
  case TOKEN_GREATER_THAN:
    return parser_parse_infix_expression;
  case TOKEN_LEFT_PAREN:
    return parser_parse_call_expression;
  default:
    return NULL;
  }
}

parser_precedence_t token_type_to_precedence(token_type_t t) {
  switch (t) {
  case TOKEN_EQUAL:
  case TOKEN_NOT_EQUAL:
    return PARSER_PRECEDENCE_EQUALS;
  case TOKEN_LESS_THAN:
  case TOKEN_GREATER_THAN:
    return PARSER_PRECEDENCE_LESSGREATER;
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    return PARSER_PRECEDENCE_SUM;
  case TOKEN_SLASH:
  case TOKEN_ASTERISK:
    return PARSER_PRECEDENCE_PRODUCT;
  case TOKEN_LEFT_PAREN:
    return PARSER_PRECEDENCE_CALL;
  default:
    return PARSER_PRECEDENCE_LOWEST;
  }
}

parser_precedence_t parser_peek_precedence(parser_t *p) {
  return token_type_to_precedence(p->peek_token->type);
}

parser_precedence_t parser_current_precedence(parser_t *p) {
  return token_type_to_precedence(p->current_token->type);
}

parser_error_t parser_peek_error(parser_t *p, token_type_t t) {
  if (p == NULL) {
    return PARSER_INVALID_ARGUMENT_ERROR;
  }

  char *msg;
  if (asprintf(&msg, "expected next token to be %s, got %s",
               token_type_humanize(t),
               token_type_humanize(p->peek_token->type)) < 0) {
    return PARSER_ALLOC_ERROR;
  }
  if (array_list_push(p->errors, msg) != ARRAY_LIST_SUCCESS) {
    return PARSER_ALLOC_ERROR;
  }
  return PARSER_SUCCESS;
}

parser_error_t parser_no_prefix_parser_fn_error(parser_t *p, token_type_t t) {
  if (p == NULL) {
    return PARSER_INVALID_ARGUMENT_ERROR;
  }

  char *msg;
  if (asprintf(&msg, "no prefix parse function found for token type %s",
               token_type_humanize(t))) {
    return PARSER_ALLOC_ERROR;
  }
  if (array_list_push(p->errors, msg) != ARRAY_LIST_SUCCESS) {
    return PARSER_ALLOC_ERROR;
  }
  return PARSER_SUCCESS;
}

bool parser_current_token_is(const parser_t *p, token_type_t t) {
  return p->current_token->type == t;
}

bool parser_peek_token_is(const parser_t *p, token_type_t t) {
  return p->peek_token->type == t;
}

bool parser_expect_peek(parser_t *p, token_type_t t) {
  if (parser_peek_token_is(p, t)) {
    parser_next_token(p);
    return true;
  }
  parser_peek_error(p, t);
  return false;
}

statement_t *parse_let_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_LET;
  s->value.let = malloc(sizeof(let_statement_t));
  if (s->value.let == NULL) {
    statement_free(s);
    return NULL;
  }

  s->value.let->token = malloc(sizeof(token_t));
  if (s->value.let->token == NULL) {
    statement_free(s);
    return NULL;
  }
  memcpy(s->value.let->token, p->current_token, sizeof(token_t));

  if (!parser_expect_peek(p, TOKEN_IDENTIFIER)) {
    statement_free(s);
    return NULL;
  }

  s->value.let->ident = malloc(sizeof(identifier_t));
  if (s->value.let->ident == NULL) {
    statement_free(s);
    return NULL;
  }
  s->value.let->ident->token = malloc(sizeof(token_t));
  if (s->value.let->ident->token == NULL) {
    statement_free(s);
    return NULL;
  }
  memcpy(s->value.let->ident->token, p->current_token, sizeof(token_t));
  s->value.let->ident->value = strdup(p->current_token->literal);

  if (!parser_expect_peek(p, TOKEN_ASSIGN)) {
    statement_free(s);
    return NULL;
  }

  parser_next_token(p);
  s->value.let->value = parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);

  while (!parser_current_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

statement_t *parse_return_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_RETURN;
  s->value.ret = malloc(sizeof(return_statement_t));
  if (s->value.ret == NULL) {
    statement_free(s);
    return NULL;
  }

  s->value.ret->token = malloc(sizeof(token_t));
  if (s->value.ret->token == NULL) {
    statement_free(s);
    return NULL;
  }
  memcpy(s->value.ret->token, p->current_token, sizeof(token_t));

  parser_next_token(p);

  s->value.ret->value = parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);

  if (parser_peek_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

expression_t *parser_parse_expression(parser_t *p,
                                      parser_precedence_t precedence) {
  prefix_parse_fn prefix = parser_prefix_fn(p->current_token->type);
  if (prefix == NULL) {
    parser_no_prefix_parser_fn_error(p, p->current_token->type);
    return NULL;
  }
  expression_t *left = prefix(p);

  while (!parser_peek_token_is(p, TOKEN_SEMICOLON) &&
         precedence < parser_peek_precedence(p)) {
    infix_parse_fn infix = parser_infix_fn(p->peek_token->type);
    if (infix == NULL) {
      return left;
    }

    parser_next_token(p);
    left = infix(p, left);
  }

  return left;
}

statement_t *parse_expression_statement(parser_t *p) {
  statement_t *s = malloc(sizeof(statement_t));
  if (s == NULL) {
    return NULL;
  }

  s->type = STATEMENT_EXPRESSION;
  s->value.exp = malloc(sizeof(expression_statement_t));
  if (s->value.exp == NULL) {
    statement_free(s);
    return NULL;
  }

  s->value.exp->token = malloc(sizeof(token_t));
  if (s->value.exp->token == NULL) {
    statement_free(s);
    return NULL;
  }
  memcpy(s->value.exp->token, p->current_token, sizeof(token_t));
  s->value.exp->token->literal = strdup(p->current_token->literal);

  s->value.exp->expression =
      parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);
  if (s->value.exp->expression == NULL) {
    statement_free(s);
    return NULL;
  }

  if (parser_peek_token_is(p, TOKEN_SEMICOLON)) {
    parser_next_token(p);
  }

  return s;
}

statement_t *parser_parse_statement(parser_t *p) {
  if (p == NULL) {
    return NULL;
  }
  switch (p->current_token->type) {
  case TOKEN_LET:
    return parse_let_statement(p);
  case TOKEN_RETURN:
    return parse_return_statement(p);
  default:
    return parse_expression_statement(p);
  }
}

expression_t *parser_parse_identifier(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_IDENTIFIER;
  e->value.ident = malloc(sizeof(identifier_t));
  if (e->value.ident == NULL) {
    expression_free(e);
    return NULL;
  }

  e->value.ident->token = malloc(sizeof(token_t));
  if (e->value.ident->token == NULL) {
    expression_free(e);
    return NULL;
  }

  e->value.ident->token->type = TOKEN_IDENTIFIER;
  e->value.ident->token->literal = strdup(p->current_token->literal);
  e->value.ident->value = strdup(p->current_token->literal);

  return e;
}

expression_t *parser_parse_integer_literal(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_INTEGER_LITERAL;
  e->value.integer = malloc(sizeof(integer_literal_t));
  if (e->value.integer == NULL) {
    expression_free(e);
    return NULL;
  }

  e->value.integer->token = malloc(sizeof(token_t));
  if (e->value.integer->token == NULL) {
    expression_free(e);
    return NULL;
  }

  e->value.integer->token->type = TOKEN_INT;
  e->value.integer->token->literal = strdup(p->current_token->literal);
  e->value.integer->value = strtol(p->current_token->literal, NULL, 10);

  return e;
}

expression_t *parser_parse_boolean_literal(parser_t *p) {
  expression_t *e = malloc(sizeof(expression_t));
  if (e == NULL) {
    return NULL;
  }

  e->type = EXPRESSION_BOOLEAN_LITERAL;
  e->value.boolean = malloc(sizeof(boolean_literal_t));
  if (e->value.boolean == NULL) {
    expression_free(e);
    return NULL;
  }

  e->value.boolean->token = malloc(sizeof(token_t));
  if (e->value.boolean->token == NULL) {
    expression_free(e);
    return NULL;
  }
  memcpy(e->value.boolean->token, p->current_token, sizeof(token_t));
  e->value.boolean->value = parser_current_token_is(p, TOKEN_TRUE);
  return e;
}

expression_t *parser_parse_grouped_expression(parser_t *p) {
  parser_next_token(p);
  expression_t *exp = parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);
  if (!parser_expect_peek(p, TOKEN_RIGHT_PAREN)) {
    expression_free(exp);
    return NULL;
  }
  return exp;
}

block_statement_t *parser_parse_block_statement(parser_t *p) {
  block_statement_t *block = malloc(sizeof(block_statement_t));
  if (block == NULL) {
    return NULL;
  }

  block->token = malloc(sizeof(token_t));
  if (block->token == NULL) {
    block_statement_free(block);
    return NULL;
  }
  memcpy(block->token, p->current_token, sizeof(token_t));

  block->statements_len = 0;
  block->statements = NULL;

  parser_next_token(p);

  while (!parser_current_token_is(p, TOKEN_RIGHT_BRACE) &&
         !parser_current_token_is(p, TOKEN_EOF)) {
    statement_t *s = parser_parse_statement(p);
    if (s != NULL) {
      block->statements_len++;
      if (block->statements == NULL) {
        block->statements = calloc(1, sizeof(statement_t));
      } else {
        block->statements = realloc(block->statements, block->statements_len *
                                                           sizeof(statement_t));
      }
      block->statements[block->statements_len - 1] = s;
    }
    parser_next_token(p);
  }

  return block;
}

expression_t *parser_parse_if_expression(parser_t *p) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_IF;
  exp->value.if_ = malloc(sizeof(if_expression_t));
  if (exp->value.if_ == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.if_->token = malloc(sizeof(token_t));
  if (exp->value.if_->token == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.if_->token, p->current_token, sizeof(token_t));

  if (!parser_expect_peek(p, TOKEN_LEFT_PAREN)) {
    expression_free(exp);
    return NULL;
  }

  parser_next_token(p);
  exp->value.if_->condition =
      parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);

  if (!parser_expect_peek(p, TOKEN_RIGHT_PAREN)) {
    expression_free(exp);
    return NULL;
  }

  if (!parser_expect_peek(p, TOKEN_LEFT_BRACE)) {
    expression_free(exp);
    return NULL;
  }

  exp->value.if_->consequence = parser_parse_block_statement(p);
  exp->value.if_->alternative = NULL;

  if (parser_peek_token_is(p, TOKEN_ELSE)) {
    parser_next_token(p);

    if (!parser_expect_peek(p, TOKEN_LEFT_BRACE)) {
      expression_free(exp);
      return NULL;
    }

    exp->value.if_->alternative = parser_parse_block_statement(p);
  }

  return exp;
}

void parser_parse_function_parameters(parser_t *p, function_literal_t *fn) {
  if (parser_peek_token_is(p, TOKEN_RIGHT_PAREN)) {
    parser_next_token(p);
    return;
  }

  parser_next_token(p);

  identifier_t *ident = malloc(sizeof(identifier_t));
  if (ident == NULL) {
    return;
  }

  ident->token = malloc(sizeof(token_t));
  if (ident->token == NULL) {
    identifier_free(ident);
    return;
  }
  memcpy(ident->token, p->current_token, sizeof(token_t));

  ident->value = strdup(p->current_token->literal);

  fn->parameters_len++;
  if (fn->parameters == NULL) {
    fn->parameters = calloc(1, sizeof(identifier_t));
  } else {
    fn->parameters =
        realloc(fn->parameters, fn->parameters_len * sizeof(identifier_t));
  }
  fn->parameters[fn->parameters_len - 1] = ident;

  while (parser_peek_token_is(p, TOKEN_COMMA)) {
    parser_next_token(p);
    parser_next_token(p);

    identifier_t *ident = malloc(sizeof(identifier_t));
    if (ident == NULL) {
      return;
    }

    ident->token = malloc(sizeof(token_t));
    if (ident->token == NULL) {
      identifier_free(ident);
      return;
    }
    memcpy(ident->token, p->current_token, sizeof(token_t));

    ident->value = strdup(p->current_token->literal);

    fn->parameters_len++;
    if (fn->parameters == NULL) {
      fn->parameters = calloc(1, sizeof(identifier_t));
    } else {
      fn->parameters =
          realloc(fn->parameters, fn->parameters_len * sizeof(identifier_t));
    }
    fn->parameters[fn->parameters_len - 1] = ident;
  }

  if (!parser_expect_peek(p, TOKEN_RIGHT_PAREN)) {
    for (size_t i = 0; i < fn->parameters_len; ++i) {
      identifier_free(fn->parameters[i]);
    }
    free(fn->parameters);
    fn->parameters_len = 0;
    return;
  }
}

expression_t *parser_parse_function_literal(parser_t *p) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_FUNCTION_LITERAL;
  exp->value.fn = malloc(sizeof(function_literal_t));
  if (exp->value.fn == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.fn->token = malloc(sizeof(token_t));
  if (exp->value.fn->token == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.fn->token, p->current_token, sizeof(token_t));

  if (!parser_expect_peek(p, TOKEN_LEFT_PAREN)) {
    expression_free(exp);
    return NULL;
  }

  exp->value.fn->parameters = NULL;
  exp->value.fn->parameters_len = 0;
  parser_parse_function_parameters(p, exp->value.fn);

  if (!parser_expect_peek(p, TOKEN_LEFT_BRACE)) {
    expression_free(exp);
    return NULL;
  }

  exp->value.fn->body = parser_parse_block_statement(p);
  return exp;
}

void parser_parse_expression_list(parser_t *p, size_t *argc,
                                  expression_t ***argv, token_type_t end) {
  *argc = 0;
  *argv = NULL;

  if (parser_peek_token_is(p, end)) {
    parser_next_token(p);
    return;
  }

  parser_next_token(p);
  *argc = 1;
  *argv = calloc(1, sizeof(expression_t));
  if (argv == NULL) {
    *argc = 0;
    *argv = NULL;
    return;
  }

  (*argv)[0] = parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);

  while (parser_peek_token_is(p, TOKEN_COMMA)) {
    parser_next_token(p);
    parser_next_token(p);

    (*argc)++;
    *argv = realloc(*argv, (*argc) * sizeof(expression_t));
    if (*argv == NULL) {
      *argc = 0;
      *argv = NULL;
      return;
    }

    (*argv)[(*argc) - 1] = parser_parse_expression(p, PARSER_PRECEDENCE_LOWEST);
  }

  if (!parser_expect_peek(p, end)) {
    *argc = 0;
    *argv = NULL;
  }
}

expression_t *parser_parse_call_expression(parser_t *p, expression_t *fn) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_CALL;
  exp->value.call = malloc(sizeof(call_expression_t));
  if (exp->value.call == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.call->token = malloc(sizeof(token_t));
  if (exp->value.call->token == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.call->token, p->current_token, sizeof(token_t));

  exp->value.call->fn = malloc(sizeof(expression_t));
  if (exp->value.call->fn == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.call->fn, fn, sizeof(expression_t));

  parser_parse_expression_list(p, &exp->value.call->argc,
                               &exp->value.call->argv, TOKEN_RIGHT_PAREN);
  return exp;
}

expression_t *parser_parse_string_literal(parser_t *p) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_STRING;
  exp->value.string = malloc(sizeof(string_literal_t));
  if (exp->value.string == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.string->token = malloc(sizeof(token_t));
  if (exp->value.string->token == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.string->token, p->current_token, sizeof(token_t));
  exp->value.string->value = strdup(p->current_token->literal);
  return exp;
}

expression_t *parser_parse_prefix_expression(parser_t *p) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_PREFIX;
  exp->value.prefix = malloc(sizeof(prefix_expression_t));
  if (exp->value.prefix == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.prefix->token = malloc(sizeof(token_t));
  if (exp->value.prefix->token == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.prefix->token->type = p->current_token->type;
  exp->value.prefix->token->literal = strdup(p->current_token->literal);
  exp->value.prefix->op = strdup(p->current_token->literal);

  parser_next_token(p);

  exp->value.prefix->right =
      parser_parse_expression(p, PARSER_PRECEDENCE_PREFIX);

  return exp;
}

expression_t *parser_parse_array_literal(parser_t *p) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_ARRAY_LITERAL;
  exp->value.arr = malloc(sizeof(array_literal_t));
  if (exp->value.arr == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.arr->token = malloc(sizeof(token_t));
  if (exp->value.arr->token == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.arr->token, p->current_token, sizeof(token_t));

  parser_parse_expression_list(p, &exp->value.arr->len,
                               &exp->value.arr->elements, TOKEN_RIGHT_BRACKET);
  return exp;
}

expression_t *parser_parse_infix_expression(parser_t *p, expression_t *left) {
  expression_t *exp = malloc(sizeof(expression_t));
  if (exp == NULL) {
    return NULL;
  }

  exp->type = EXPRESSION_INFIX;
  exp->value.infix = malloc(sizeof(infix_expression_t));
  if (exp->value.infix == NULL) {
    expression_free(exp);
    return NULL;
  }

  exp->value.infix->token = malloc(sizeof(token_t));
  if (exp->value.infix->token == NULL) {
    expression_free(exp);
    return NULL;
  }

  memcpy(exp->value.infix->token, p->current_token, sizeof(token_t));
  exp->value.infix->op = strdup(p->current_token->literal);

  exp->value.infix->left = malloc(sizeof(expression_t));
  if (exp->value.infix->left == NULL) {
    expression_free(exp);
    return NULL;
  }
  memcpy(exp->value.infix->left, left, sizeof(expression_t));

  parser_precedence_t precedence = parser_current_precedence(p);
  parser_next_token(p);
  exp->value.infix->right = parser_parse_expression(p, precedence);
  if (exp->value.infix->right == NULL) {
    expression_free(exp);
    return NULL;
  }

  return exp;
}

program_t *parser_parse_program(parser_t *p) {
  program_t *prg = malloc(sizeof(program_t));
  if (prg == NULL) {
    return NULL;
  }
  prg->statements_len = 0;
  prg->statements_cap = 128;
  prg->statements = calloc(prg->statements_cap, sizeof(statement_t));
  if (prg->statements == NULL) {
    program_free(prg);
    return NULL;
  }

  while (p->current_token->type != TOKEN_EOF) {
    statement_t *statement = parser_parse_statement(p);
    if (prg->statements_len == prg->statements_cap) {
      prg->statements_cap *= 2;
      prg->statements =
          realloc(prg->statements, prg->statements_cap * sizeof(statement_t));
      if (prg->statements == NULL) {
        program_free(prg);
        return NULL;
      }
    }
    prg->statements[prg->statements_len] = statement;
    prg->statements_len++;
    parser_next_token(p);
  }
  return prg;
}
