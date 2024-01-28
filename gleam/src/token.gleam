pub type Token {
  Illegal
  EOF

  // Identifiers + literals
  Identifier(literal: String)
  Integer(literal: String)

  // Operators
  Assign
  Plus

  // Delimiters
  Comma
  Semicolon

  LParen
  RParen
  LBrace
  RBrace

  // Keywords
  Function
  Let
}

pub fn to_string(token: Token) -> String {
  case token {
    Illegal -> "ILLEGAL"
    EOF -> "EOF"

    Identifier(literal) -> literal
    Integer(literal) -> literal

    Assign -> "="
    Plus -> "+"

    Comma -> ","
    Semicolon -> ";"

    LParen -> "("
    RParen -> ")"
    LBrace -> "{"
    RBrace -> "}"

    Function -> "FUNCTION"
    Let -> "LET"
  }
}

pub fn lookup_identifier(str: String) -> Token {
  case str {
    "fn" -> Function
    "let" -> Let
    _ -> Identifier(str)
  }
}
