#pragma once

#include "string.c"
#include <string.h>

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
  if (strncmp(ident.buffer, "fn", ident.len) == 0) {
    return TOKEN_FUNCTION;
  } else if (strncmp(ident.buffer, "let", ident.len) == 0) {
    return TOKEN_LET;
  } else if (strncmp(ident.buffer, "true", ident.len) == 0) {
    return TOKEN_TRUE;
  } else if (strncmp(ident.buffer, "false", ident.len) == 0) {
    return TOKEN_FALSE;
  } else if (strncmp(ident.buffer, "if", ident.len) == 0) {
    return TOKEN_IF;
  } else if (strncmp(ident.buffer, "else", ident.len) == 0) {
    return TOKEN_ELSE;
  } else if (strncmp(ident.buffer, "return", ident.len) == 0) {
    return TOKEN_RETURN;
  }
  return TOKEN_IDENT;
}
