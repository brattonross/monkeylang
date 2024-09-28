#ifndef __TOKEN_H__
#define __TOKEN_H__

typedef enum {
  TOKEN_EOF = -1,
  TOKEN_ILLEGAL,

  TOKEN_IDENTIFIER,
  TOKEN_INT,
  TOKEN_STRING,

  TOKEN_ASSIGN,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_BANG,
  TOKEN_ASTERISK,
  TOKEN_SLASH,

  TOKEN_LESS_THAN,
  TOKEN_GREATER_THAN,

  TOKEN_EQUAL,
  TOKEN_NOT_EQUAL,

  TOKEN_COMMA,
  TOKEN_SEMICOLON,

  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,

  TOKEN_FUNCTION,
  TOKEN_LET,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_RETURN,
} token_type_t;

typedef struct {
  token_type_t type;
  char *literal;
} token_t;

char *token_type_humanize(token_type_t t);
void token_free(token_t *t);

#endif // __TOKEN_H__
