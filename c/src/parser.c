#pragma once

#include "ast.c"
#include "lexer.c"
#include "mem.c"
#include "strconv.c"
#include "token.c"
#include <c++/14.2.1/bits/fs_dir.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Error {
  String message;
} Error;

typedef struct ErrorList {
  Error *errors;
  size_t capacity;
  size_t length;
} ErrorList;

void error_list_init(ErrorList *list, Arena *arena) {
  list->capacity = 8;
  list->length = 0;
  list->errors = arena_alloc(arena, list->capacity * sizeof(Error));
}

void error_list_append(ErrorList *list, Arena *arena, String message) {
  if (list->length >= list->capacity) {
    size_t new_capacity = list->capacity * 2;
    Error *new_errors = arena_alloc(arena, new_capacity * sizeof(Error));

    for (size_t i = 0; i < list->length; ++i) {
      new_errors[i] = list->errors[i];
    }

    list->errors = new_errors;
    list->capacity = new_capacity;
  }

  list->errors[list->length++] = (Error){
      .message = message,
  };
}

typedef struct Parser {
  Lexer *lexer;
  Token current_token;
  Token peek_token;
  ErrorList errors;
} Parser;

void parser_next_token(Parser *parser);
void parser_parse_statement(Parser *parser, Arena *arena, Statement *statement);
void parser_parse_let_statement(Parser *parser, Arena *arena,
                                Statement *statement);
void parser_parse_return_statement(Parser *parser, Statement *statement);
void parser_parse_expression_statement(Parser *parser, Arena *arena,
                                       Statement *statement);
void parser_parse_identifier(Parser *parser, Expression *expression);
void parser_parse_integer_literal(Parser *parser, Arena *arena,
                                  Expression *expression);
bool parser_expect_peek(Parser *parser, Arena *arena, TokenType token_type);

void parser_init(Parser *parser, Arena *arena, Lexer *lexer) {
  parser->lexer = lexer;
  error_list_init(&parser->errors, arena);
  parser_next_token(parser);
  parser_next_token(parser);
}

void parser_next_token(Parser *parser) {
  parser->current_token = parser->peek_token;
  parser->peek_token = lexer_next_token(parser->lexer);
}

Program *parser_parse_program(Parser *parser, Arena *arena) {
  Program *program = program_create(arena);

  while (parser->current_token.type != TOKEN_EOF) {
    Statement s = {0};
    parser_parse_statement(parser, arena, &s);
    program_append_statement(program, arena, s);
    parser_next_token(parser);
  }

  return program;
}

void parser_parse_statement(Parser *parser, Arena *arena,
                            Statement *statement) {
  switch (parser->current_token.type) {
  case TOKEN_LET:
    parser_parse_let_statement(parser, arena, statement);
    break;
  case TOKEN_RETURN:
    parser_parse_return_statement(parser, statement);
    break;
  default:
    parser_parse_expression_statement(parser, arena, statement);
    break;
  }
}

void parser_parse_let_statement(Parser *parser, Arena *arena,
                                Statement *statement) {
  LetStatement let = {0};
  let.token = parser->current_token;

  if (!parser_expect_peek(parser, arena, TOKEN_IDENT)) {
    return;
  }

  let.name = arena_alloc(arena, sizeof(Identifier));
  let.name->token = parser->current_token;
  let.name->value = parser->current_token.literal;

  if (!parser_expect_peek(parser, arena, TOKEN_ASSIGN)) {
    return;
  }

  // TODO: skipping expressions
  while (parser->current_token.type != TOKEN_SEMICOLON) {
    parser_next_token(parser);
  }

  statement->type = STATEMENT_LET;
  statement->data.let_statement = let;
}

void parser_parse_return_statement(Parser *parser, Statement *statement) {
  ReturnStatement ret = {0};
  ret.token = parser->current_token;

  parser_next_token(parser);

  // TODO: skipping expressions
  while (parser->current_token.type != TOKEN_SEMICOLON) {
    parser_next_token(parser);
  }

  statement->type = STATEMENT_RETURN;
  statement->data.return_statement = ret;
}

typedef enum Precedence {
  PRECEDENCE_LOWEST,
  PRECEDENCE_EQUALS,
  PRECEDENCE_LESSGREATER,
  PRECEDENCE_SUM,
  PRECEDENCE_PRODUCT,
  PRECEDENCE_PREFIX,
  PRECEDENCE_CALL,
} Precedence;

void parser_parse_expression(Parser *parser, Arena *arena,
                             Expression *expression, Precedence precedence) {
  switch (parser->current_token.type) {
  case TOKEN_IDENT:
    parser_parse_identifier(parser, expression);
    break;
  case TOKEN_INT:
    parser_parse_integer_literal(parser, arena, expression);
    break;
  default:
    break;
  }

  if (precedence) { // TODO: remove
  }
}

void parser_parse_expression_statement(Parser *parser, Arena *arena,
                                       Statement *statement) {
  statement->type = STATEMENT_EXPRESSION;
  statement->data.expression_statement = (ExpressionStatement){
      .token = parser->current_token,
      .expression = arena_alloc(arena, sizeof(Expression)),
  };
  parser_parse_expression(parser, arena,
                          statement->data.expression_statement.expression,
                          PRECEDENCE_LOWEST);

  if (parser->peek_token.type == TOKEN_SEMICOLON) {
    parser_next_token(parser);
  }
}

void parser_parse_identifier(Parser *parser, Expression *expression) {
  expression->type = EXPRESSION_IDENTIFIER;
  expression->data.identifier = (Identifier){
      .token = parser->current_token,
      .value = parser->current_token.literal,
  };
}

void parser_parse_integer_literal(Parser *parser, Arena *arena,
                                  Expression *expression) {
  size_t end_ptr;
  int64_t value = string_to_int64(parser->current_token.literal, &end_ptr);
  if (end_ptr != parser->current_token.literal.length) {
    String message = string_fmt(arena, "could not parse %.*s as integer",
                                parser->current_token.literal.length,
                                parser->current_token.literal.buffer);
    error_list_append(&parser->errors, arena, message);
    return;
  }

  expression->type = EXPRESSION_INTEGER;
  expression->data.integer = (IntegerLiteral){
      .token = parser->current_token,
      .value = value,
  };
}

void parser_peek_error(Parser *parser, Arena *arena, TokenType token_type) {
  String message =
      string_fmt(arena, "expected next token to be %.*s, got %.*s instead",
                 token_type_strings[token_type].length,
                 token_type_strings[token_type].buffer,
                 token_type_strings[parser->peek_token.type].length,
                 token_type_strings[parser->peek_token.type].buffer);
  error_list_append(&parser->errors, arena, message);
}

bool parser_expect_peek(Parser *parser, Arena *arena, TokenType token_type) {
  if (parser->peek_token.type == token_type) {
    parser_next_token(parser);
    return true;
  }
  parser_peek_error(parser, arena, token_type);
  return false;
}
