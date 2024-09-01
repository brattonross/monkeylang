#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

lexer_t *new_lexer(const char *input) {
  lexer_t *l = malloc(sizeof(lexer_t));
  if (l != NULL) {
    l->input = input;
  }
  return l;
}

char current_char(lexer_t *l) {
  size_t len = strlen(l->input);
  if (l->pos >= len) {
    return EOF;
  }
  return l->input[l->pos];
}

void read_char(lexer_t *l) { l->pos++; }

bool is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

bool is_digit(char ch) { return '0' <= ch && ch <= '9'; }

char *read_identifier(lexer_t *l) {
  size_t pos = l->pos;
  while (is_letter(current_char(l))) {
    read_char(l);
  }
  return strndup(l->input + pos, l->pos - pos);
}

char *read_number(lexer_t *l) {
  int pos = l->pos;
  while (is_digit(current_char(l))) {
    read_char(l);
  }
  return strndup(l->input + pos, l->pos - pos);
}

static const token_t identifier_table[] = {
    {.type = TOKEN_LET, .literal = "let"},
    {.type = TOKEN_FUNCTION, .literal = "fn"},
    {.type = TOKEN_TRUE, .literal = "true"},
    {.type = TOKEN_FALSE, .literal = "false"},
    {.type = TOKEN_IF, .literal = "if"},
    {.type = TOKEN_ELSE, .literal = "else"},
    {.type = TOKEN_RETURN, .literal = "return"},
};
static const size_t total_identifiers =
    sizeof identifier_table / sizeof *identifier_table;

token_type_t lookup_ident(char *ident) {
  token_type_t t = TOKEN_IDENTIFIER;
  for (int i = 0; i < total_identifiers; ++i) {
    if (strcmp(ident, identifier_table[i].literal) == 0) {
      t = identifier_table[i].type;
      break;
    }
  }
  return t;
}

bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void skip_whitespace(lexer_t *l) {
  while (is_whitespace(current_char(l))) {
    read_char(l);
  }
}

char peek_char(lexer_t *l) {
  size_t peek_pos = l->pos + 1;
  if (peek_pos >= strlen(l->input)) {
    return EOF;
  }
  return l->input[peek_pos];
}

token_t next_token(lexer_t *l) {
  skip_whitespace(l);

  token_t *t = malloc(sizeof(token_t));
  switch (current_char(l)) {
  case '=':
    if (peek_char(l) == '=') {
      read_char(l);
      t = &(token_t){.type = TOKEN_EQUAL, .literal = "=="};
    } else {
      t = &(token_t){.type = TOKEN_ASSIGN, .literal = "="};
    }
    break;
  case ';':
    t = &(token_t){.type = TOKEN_SEMICOLON, .literal = ";"};
    break;
  case '(':
    t = &(token_t){.type = TOKEN_LEFT_PAREN, .literal = "("};
    break;
  case ')':
    t = &(token_t){.type = TOKEN_RIGHT_PAREN, .literal = ")"};
    break;
  case ',':
    t = &(token_t){.type = TOKEN_COMMA, .literal = ","};
    break;
  case '+':
    t = &(token_t){.type = TOKEN_PLUS, .literal = "+"};
    break;
  case '-':
    t = &(token_t){.type = TOKEN_MINUS, .literal = "-"};
    break;
  case '!':
    if (peek_char(l) == '=') {
      read_char(l);
      t = &(token_t){.type = TOKEN_NOT_EQUAL, .literal = "!="};
    } else {
      t = &(token_t){.type = TOKEN_BANG, .literal = "!"};
    }
    break;
  case '/':
    t = &(token_t){.type = TOKEN_SLASH, .literal = "/"};
    break;
  case '*':
    t = &(token_t){.type = TOKEN_ASTERISK, .literal = "*"};
    break;
  case '<':
    t = &(token_t){.type = TOKEN_LESS_THAN, .literal = "<"};
    break;
  case '>':
    t = &(token_t){.type = TOKEN_GREATER_THAN, .literal = ">"};
    break;
  case '{':
    t = &(token_t){.type = TOKEN_LEFT_BRACE, .literal = "{"};
    break;
  case '}':
    t = &(token_t){.type = TOKEN_RIGHT_BRACE, .literal = "}"};
    break;
  case EOF:
    t = &(token_t){.type = TOKEN_EOF, .literal = ""};
    break;
  default:
    if (is_letter(current_char(l))) {
      char *literal = read_identifier(l);
      token_type_t type = lookup_ident(literal);
      return (token_t){.type = type, .literal = literal};
    } else if (is_digit(current_char(l))) {
      return (token_t){.type = TOKEN_INT, .literal = read_number(l)};
    } else {
      t = &(token_t){.type = TOKEN_ILLEGAL,
                     .literal = &(char){current_char(l)}};
    }
  }

  read_char(l);
  return *t;
}
