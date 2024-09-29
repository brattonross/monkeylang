#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

lexer_t *lexer_init(const char *input) {
  if (input == NULL) {
    return NULL;
  }
  lexer_t *l = malloc(sizeof(lexer_t));
  if (l == NULL) {
    return NULL;
  }
  l->input = input;
  l->pos = 0;
  return l;
}

void lexer_free(lexer_t *l) {
  free(l);
  l = NULL;
}

char lexer_current_char(lexer_t *l) {
  size_t len = strlen(l->input);
  if (l->pos >= len) {
    return EOF;
  }
  return l->input[l->pos];
}

void lexer_advance(lexer_t *l) { l->pos++; }

bool is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

bool is_digit(char ch) { return '0' <= ch && ch <= '9'; }

char *lexer_read_identifier(lexer_t *l) {
  size_t pos = l->pos;
  while (is_letter(lexer_current_char(l))) {
    lexer_advance(l);
  }
  return strndup(l->input + pos, l->pos - pos);
}

char *lexer_read_number(lexer_t *l) {
  int pos = l->pos;
  while (is_digit(lexer_current_char(l))) {
    lexer_advance(l);
  }
  return strndup(l->input + pos, l->pos - pos);
}

static const token_t identifier_table[] = {
    {TOKEN_LET, "let"},       {TOKEN_FUNCTION, "fn"}, {TOKEN_TRUE, "true"},
    {TOKEN_FALSE, "false"},   {TOKEN_IF, "if"},       {TOKEN_ELSE, "else"},
    {TOKEN_RETURN, "return"},
};
static const size_t total_identifiers =
    sizeof identifier_table / sizeof *identifier_table;

token_type_t lookup_ident(const char *ident) {
  token_type_t t = TOKEN_IDENTIFIER;
  for (size_t i = 0; i < total_identifiers; ++i) {
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

void lexer_skip_whitespace(lexer_t *l) {
  while (is_whitespace(lexer_current_char(l))) {
    lexer_advance(l);
  }
}

char lexer_peek_char(lexer_t *l) {
  size_t peek_pos = l->pos + 1;
  if (peek_pos >= strlen(l->input)) {
    return EOF;
  }
  return l->input[peek_pos];
}

char *lexer_read_string(lexer_t *l) {
  size_t pos = l->pos + 1;
  while (true) {
    lexer_advance(l);
    char cur = lexer_current_char(l);
    if (cur == '"' || cur == 0) {
      break;
    }
  }
  size_t len = l->pos - pos;
  char *buf = malloc(len + 1);
  return strncpy(buf, l->input + pos, len);
}

token_t *lexer_next_token(lexer_t *l) {
  if (l == NULL) {
    return NULL;
  }

  lexer_skip_whitespace(l);

  token_t *t = malloc(sizeof(token_t));
  if (t == NULL) {
    return NULL;
  }

  t->literal = NULL;

  switch (lexer_current_char(l)) {
  case '=':
    if (lexer_peek_char(l) == '=') {
      lexer_advance(l);
      t->type = TOKEN_EQUAL;
      t->literal = strdup("==");
    } else {
      t->type = TOKEN_ASSIGN;
      t->literal = strdup("=");
    }
    break;
  case ';':
    t->type = TOKEN_SEMICOLON;
    t->literal = strdup(";");
    break;
  case '(':
    t->type = TOKEN_LEFT_PAREN;
    t->literal = strdup("(");
    break;
  case ')':
    t->type = TOKEN_RIGHT_PAREN;
    t->literal = strdup(")");
    break;
  case ',':
    t->type = TOKEN_COMMA;
    t->literal = strdup(",");
    break;
  case '+':
    t->type = TOKEN_PLUS;
    t->literal = strdup("+");
    break;
  case '-':
    t->type = TOKEN_MINUS;
    t->literal = strdup("-");
    break;
  case '!':
    if (lexer_peek_char(l) == '=') {
      lexer_advance(l);
      t->type = TOKEN_NOT_EQUAL;
      t->literal = strdup("!=");
    } else {
      t->type = TOKEN_BANG;
      t->literal = strdup("!");
    }
    break;
  case '/':
    t->type = TOKEN_SLASH;
    t->literal = strdup("/");
    break;
  case '*':
    t->type = TOKEN_ASTERISK;
    t->literal = strdup("*");
    break;
  case '<':
    t->type = TOKEN_LESS_THAN;
    t->literal = strdup("<");
    break;
  case '>':
    t->type = TOKEN_GREATER_THAN;
    t->literal = strdup(">");
    break;
  case '{':
    t->type = TOKEN_LEFT_BRACE;
    t->literal = strdup("{");
    break;
  case '}':
    t->type = TOKEN_RIGHT_BRACE;
    t->literal = strdup("}");
    break;
  case '"':
    t->type = TOKEN_STRING;
    t->literal = lexer_read_string(l);
    break;
  case EOF:
    t->type = TOKEN_EOF;
    t->literal = strdup("");
    break;
  default:
    if (is_letter(lexer_current_char(l))) {
      t->literal = lexer_read_identifier(l);
      t->type = lookup_ident(t->literal);
      return t;
    } else if (is_digit(lexer_current_char(l))) {
      t->type = TOKEN_INT;
      t->literal = lexer_read_number(l);
      return t;
    } else {
      t->type = TOKEN_ILLEGAL;
      t->literal = malloc(2 * sizeof(char));
      if (t->literal == NULL) {
        token_free(t);
        return NULL;
      }
      t->literal[0] = lexer_current_char(l);
      t->literal[1] = '\0';
    }
  }

  lexer_advance(l);

  if (t->literal == NULL) {
    token_free(t);
    return NULL;
  }

  return t;
}
