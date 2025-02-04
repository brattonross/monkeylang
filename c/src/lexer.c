#pragma once

#include "string.c"
#include "token.c"
#include <stdbool.h>
#include <stddef.h>

typedef struct Lexer {
  String buffer;
  size_t pos;
} Lexer;

Token lexer_next_token(Lexer *lexer);
String lexer_read_identifier(Lexer *lexer);
String lexer_read_number(Lexer *lexer);
char lexer_current_char(const Lexer *lexer);
char lexer_peek_char(const Lexer *lexer);
void lexer_advance(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
bool is_letter(char c);
bool is_digit(char c);

void lexer_init(Lexer *lexer, char *input) {
  lexer->buffer = String(input);
  lexer->pos = 0;
}

Token lexer_next_token(Lexer *lexer) {
  lexer_skip_whitespace(lexer);

  Token token = {.type = TOKEN_ILLEGAL,
                 .literal =
                     string_slice(lexer->buffer, lexer->pos, lexer->pos + 1)};

  char current = lexer_current_char(lexer);
  switch (current) {
  case '=':
    if (lexer_peek_char(lexer) == '=') {
      token.type = TOKEN_EQ;
      token.literal = string_slice(lexer->buffer, lexer->pos, lexer->pos + 2);
      lexer_advance(lexer);
    } else {
      token.type = TOKEN_ASSIGN;
    }
    break;
  case '+':
    token.type = TOKEN_PLUS;
    break;
  case '-':
    token.type = TOKEN_MINUS;
    break;
  case '!':
    if (lexer_peek_char(lexer) == '=') {
      token.type = TOKEN_NOT_EQ;
      token.literal = string_slice(lexer->buffer, lexer->pos, lexer->pos + 2);
      lexer_advance(lexer);
    } else {
      token.type = TOKEN_BANG;
    }
    break;
  case '/':
    token.type = TOKEN_SLASH;
    break;
  case '*':
    token.type = TOKEN_ASTERISK;
    break;
  case '<':
    token.type = TOKEN_LT;
    break;
  case '>':
    token.type = TOKEN_GT;
    break;
  case ';':
    token.type = TOKEN_SEMICOLON;
    break;
  case '(':
    token.type = TOKEN_LPAREN;
    break;
  case ')':
    token.type = TOKEN_RPAREN;
    break;
  case ',':
    token.type = TOKEN_COMMA;
    break;
  case '{':
    token.type = TOKEN_LBRACE;
    break;
  case '}':
    token.type = TOKEN_RBRACE;
    break;
  case 0:
    token.type = TOKEN_EOF;
    token.literal = String("");
    break;
  default:
    if (is_letter(current)) {
      token.literal = lexer_read_identifier(lexer);
      token.type = token_type_from_ident(token.literal);
      return token;
    } else if (is_digit(current)) {
      token.type = TOKEN_INT;
      token.literal = lexer_read_number(lexer);
      return token;
    }
    break;
  }

  lexer_advance(lexer);
  return token;
}

String lexer_read_identifier(Lexer *lexer) {
  size_t pos = lexer->pos;
  while (is_letter(lexer_current_char(lexer))) {
    lexer_advance(lexer);
  }
  return string_slice(lexer->buffer, pos, lexer->pos);
}

String lexer_read_number(Lexer *lexer) {
  size_t pos = lexer->pos;
  while (is_digit(lexer_current_char(lexer))) {
    lexer_advance(lexer);
  }
  return string_slice(lexer->buffer, pos, lexer->pos);
}

char lexer_char_at_pos(const Lexer *lexer, size_t pos) {
  if (pos >= lexer->buffer.len) {
    return 0;
  }
  return lexer->buffer.buffer[pos];
}

char lexer_current_char(const Lexer *lexer) {
  return lexer_char_at_pos(lexer, lexer->pos);
}

char lexer_peek_char(const Lexer *lexer) {
  return lexer_char_at_pos(lexer, lexer->pos + 1);
}

void lexer_advance(Lexer *lexer) { ++lexer->pos; }

void lexer_skip_whitespace(Lexer *lexer) {
  for (char c = lexer_current_char(lexer);
       c == ' ' || c == '\t' || c == '\n' || c == '\r';
       c = lexer_current_char(lexer)) {
    lexer_advance(lexer);
  }
}

bool is_letter(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_digit(char c) { return '0' <= c && c <= '9'; }
