export type TokenType =
  | "ILLEGAL"
  | "EOF"
  | "IDENT"
  | "INT"
  | "="
  | "+"
  | ","
  | ";"
  | "("
  | ")"
  | "{"
  | "}"
  | "FUNCTION"
  | "LET";

export class Token {
  public constructor(public type: TokenType, public literal: string) {}
}

const KEYWORDS: Record<string, TokenType> = {
  fn: "FUNCTION",
  let: "LET",
} as const;

export function lookupIdent(ident: string): TokenType {
  return KEYWORDS[ident] ?? "IDENT";
}
