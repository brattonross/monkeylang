#pragma once

#include "ast.c"
#include "lexer.c"
#include "mem.c"
#include "token.c"
#include <stdbool.h>

typedef struct Parser {
  Lexer *lexer;
  Token current_token;
  Token peek_token;
} Parser;

void parser_next_token(Parser *parser);
void parser_parse_statement(Parser *parser, Statement *statement);
void parser_parse_let_statement(Parser *parser, Statement *statement);
bool parser_expect_peek(Parser *parser, TokenType token_type);

void parser_init(Parser *parser, Lexer *lexer) {
  parser->lexer = lexer;
  parser_next_token(parser);
  parser_next_token(parser);
}

void parser_next_token(Parser *parser) {
  parser->current_token = parser->peek_token;
  parser->peek_token = lexer_next_token(parser->lexer);
}

Program *parser_parse_program(Parser *parser, Arena *arena) {
  Program *program = create_program(arena);

  while (parser->current_token.type != TOKEN_EOF) {
    Statement s = {0};
    parser_parse_statement(parser, &s);
    program_append_statement(program, s);
    parser_next_token(parser);
  }

  return program;
}

void parser_parse_statement(Parser *parser, Statement *statement) {
  switch (parser->current_token.type) {
  case TOKEN_LET:
    parser_parse_let_statement(parser, statement);
    break;
  default:
    break;
  }
}

void parser_parse_let_statement(Parser *parser, Statement *statement) {
  LetStatement let = {0};
  let.token = parser->current_token;

  if (!parser_expect_peek(parser, TOKEN_IDENT)) {
    return;
  }

  let.name.token = parser->current_token;
  let.name.value = parser->current_token.literal;

  if (!parser_expect_peek(parser, TOKEN_ASSIGN)) {
    return;
  }

  // TODO: skipping expressions
  while (parser->current_token.type != TOKEN_SEMICOLON) {
    parser_next_token(parser);
  }

  statement->type = STATEMENT_LET;
  statement->value.let = let;
}

bool parser_expect_peek(Parser *parser, TokenType token_type) {
  if (parser->peek_token.type == token_type) {
    parser_next_token(parser);
    return true;
  }
  return false;
}
