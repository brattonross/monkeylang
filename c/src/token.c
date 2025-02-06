#pragma once

#include "string.c"

typedef enum TokenType {
  TOKEN_ILLEGAL,
  TOKEN_EOF,

  // identifiers and literals
  TOKEN_IDENT,
  TOKEN_INT,

  // operators
  TOKEN_ASSIGN,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_BANG,
  TOKEN_ASTERISK,
  TOKEN_SLASH,

  TOKEN_LT,
  TOKEN_GT,

  TOKEN_EQ,
  TOKEN_NOT_EQ,

  // delimiters
  TOKEN_COMMA,
  TOKEN_SEMICOLON,

  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,

  // keywords
  TOKEN_FUNCTION,
  TOKEN_LET,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_RETURN,
} TokenType;

const String token_type_strings[] = {
    String("ILLEGAL"),  String("EOF"),    String("IDENT"),  String("INT"),
    String("ASSIGN"),   String("PLUS"),   String("MINUS"),  String("BANG"),
    String("ASTERISK"), String("SLASH"),  String("LT"),     String("GT"),
    String("EQ"),       String("NOT_EQ"), String("COMMA"),  String("SEMICOLON"),
    String("LPAREN"),   String("RPAREN"), String("LBRACE"), String("RBRACE"),
    String("FUNCTION"), String("LET"),    String("TRUE"),   String("FALSE"),
    String("IF"),       String("ELSE"),   String("RETURN"),
};

typedef struct Token {
  TokenType type;
  String literal;
} Token;

TokenType token_type_from_ident(String ident) {
  if (string_cmp(ident, String("fn"))) {
    return TOKEN_FUNCTION;
  } else if (string_cmp(ident, String("let"))) {
    return TOKEN_LET;
  } else if (string_cmp(ident, String("true"))) {
    return TOKEN_TRUE;
  } else if (string_cmp(ident, String("false"))) {
    return TOKEN_FALSE;
  } else if (string_cmp(ident, String("if"))) {
    return TOKEN_IF;
  } else if (string_cmp(ident, String("else"))) {
    return TOKEN_ELSE;
  } else if (string_cmp(ident, String("return"))) {
    return TOKEN_RETURN;
  }
  return TOKEN_IDENT;
}
