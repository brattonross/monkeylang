export type TokenType =
  | "ILLEGAL"
  | "EOF"
  // Identifiers + literals
  | "IDENT"
  | "INT"
  | "STRING"
  // Operators
  | "ASSIGN"
  | "PLUS"
  | "BANG"
  | "MINUS"
  | "SLASH"
  | "ASTERISK"
  | "LT"
  | "GT"
  | "EQ"
  | "NOT_EQ"
  // Delimiters
  | "COMMA"
  | "SEMICOLON"
  | "LPAREN"
  | "RPAREN"
  | "LBRACE"
  | "RBRACE"
  | "LBRACKET"
  | "RBRACKET"
  | "COLON"
  // Keywords
  | "FUNCTION"
  | "LET"
  | "TRUE"
  | "FALSE"
  | "IF"
  | "ELSE"
  | "RETURN";

export class Token {
  public constructor(public type: TokenType, public literal: string) {}
}

const KEYWORDS: Record<string, TokenType> = {
  fn: "FUNCTION",
  let: "LET",
  true: "TRUE",
  false: "FALSE",
  if: "IF",
  else: "ELSE",
  return: "RETURN",
} as const;

export function lookupIdent(ident: string): TokenType {
  return KEYWORDS[ident] ?? "IDENT";
}
