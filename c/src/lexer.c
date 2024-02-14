#include "lexer.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void lexer_read_char(lexer_t *l) {
  if (l->read_position >= strlen(l->input)) {
    l->ch = 0;
  } else {
    l->ch = l->input[l->read_position];
  }
  l->position = l->read_position;
  l->read_position += 1;
}

lexer_t *lexer_new(char *input) {
  lexer_t *l = malloc(sizeof(lexer_t));
  l->input = input;
  l->position = 0;
  l->read_position = 0;
  l->ch = 0;
  lexer_read_char(l);
  return l;
}

bool is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

bool is_digit(char ch) { return '0' <= ch && ch <= '9'; }

char *lexer_read_identifier(lexer_t *l) {
  int position = l->position;
  while (is_letter(l->ch)) {
    lexer_read_char(l);
  }
  return strndup(l->input + position, l->position - position);
}

char *lexer_read_number(lexer_t *l) {
  int position = l->position;
  while (is_digit(l->ch)) {
    lexer_read_char(l);
  }
  return strndup(l->input + position, l->position - position);
}

token_type_t lookup_ident(char *ident) {
  if (strcmp(ident, "let") == 0) {
    return LET;
  } else if (strcmp(ident, "fn") == 0) {
    return FUNCTION;
  } else {
    return IDENT;
  }
}

void lexer_skip_whitespace(lexer_t *lexer) {
  for (; lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' ||
         lexer->ch == '\r';
       lexer_read_char(lexer))
    ;
}

token_t lexer_next_token(lexer_t *l) {
  token_t token;

  lexer_skip_whitespace(l);

  switch (l->ch) {
  case '=':
    token = (token_t){.token_type = ASSIGN, .literal = "="};
    break;
  case ';':
    token = (token_t){.token_type = SEMICOLON, .literal = ";"};
    break;
  case '(':
    token = (token_t){.token_type = LPAREN, .literal = "("};
    break;
  case ')':
    token = (token_t){.token_type = RPAREN, .literal = ")"};
    break;
  case ',':
    token = (token_t){.token_type = COMMA, .literal = ","};
    break;
  case '+':
    token = (token_t){.token_type = PLUS, .literal = "+"};
    break;
  case '{':
    token = (token_t){.token_type = LBRACE, .literal = "{"};
    break;
  case '}':
    token = (token_t){.token_type = RBRACE, .literal = "}"};
    break;
  case 0:
    token = (token_t){.token_type = END_OF_FILE, .literal = ""};
    break;
  default:
    if (is_letter(l->ch)) {
      char *literal = lexer_read_identifier(l);
      token_type_t type = lookup_ident(literal);
      return (token_t){.token_type = type, .literal = literal};
    } else if (is_digit(l->ch)) {
      return (token_t){.token_type = INT, .literal = lexer_read_number(l)};
    } else {
      token = (token_t){.token_type = ILLEGAL, .literal = &l->ch};
    }
  }

  lexer_read_char(l);
  return token;
}
