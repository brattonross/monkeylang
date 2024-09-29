#include "token.h"
#include <stdlib.h>
#include <string.h>

void token_free(token_t *t) {
  free(t->literal);
  t->literal = NULL;
  free(t);
  t = NULL;
}

char *token_type_humanize(token_type_t t) {
  switch (t) {
  case TOKEN_EOF:
    return strdup("EOF");
  case TOKEN_ILLEGAL:
    return strdup("ILLEGAL");

  case TOKEN_IDENTIFIER:
    return strdup("IDENTIFIER");
  case TOKEN_INT:
    return strdup("INT");
  case TOKEN_STRING:
    return strdup("STRING");

  case TOKEN_ASSIGN:
    return strdup("ASSIGN");
  case TOKEN_PLUS:
    return strdup("PLUS");
  case TOKEN_MINUS:
    return strdup("MINUS");
  case TOKEN_BANG:
    return strdup("BANG");
  case TOKEN_ASTERISK:
    return strdup("ASTERISK");
  case TOKEN_SLASH:
    return strdup("SLASH");

  case TOKEN_LESS_THAN:
    return strdup("LESS_THAN");
  case TOKEN_GREATER_THAN:
    return strdup("GREATER_THAN");

  case TOKEN_EQUAL:
    return strdup("EQUAL");
  case TOKEN_NOT_EQUAL:
    return strdup("NOT_EQUAL");

  case TOKEN_COMMA:
    return strdup("COMMA");
  case TOKEN_SEMICOLON:
    return strdup("SEMICOLON");

  case TOKEN_LEFT_PAREN:
    return strdup("LEFT_PAREN");
  case TOKEN_RIGHT_PAREN:
    return strdup("RIGHT_PAREN");
  case TOKEN_LEFT_BRACE:
    return strdup("LEFT_BRACE");
  case TOKEN_RIGHT_BRACE:
    return strdup("RIGHT_BRACE");
  case TOKEN_LEFT_BRACKET:
    return strdup("LEFT_BRACKET");
  case TOKEN_RIGHT_BRACKET:
    return strdup("RIGHT_BRACKET");

  case TOKEN_FUNCTION:
    return strdup("FUNCTION");
  case TOKEN_LET:
    return strdup("LET");
  case TOKEN_TRUE:
    return strdup("TRUE");
  case TOKEN_FALSE:
    return strdup("FALSE");
  case TOKEN_IF:
    return strdup("IF");
  case TOKEN_ELSE:
    return strdup("ELSE");
  case TOKEN_RETURN:
    return strdup("RETURN");
  }
}
