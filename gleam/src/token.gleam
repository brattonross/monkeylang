pub type Token {
  Illegal
  EOF

  // Identifiers + literals
  Identifier(literal: String)
  Integer(literal: String)

  // Operators
  Assign
  Plus
  Minus
  Bang
  Asterisk
  Slash
  LessThan
  GreaterThan
  Equal
  NotEqual

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
  True
  False
  If
  Else
  Return
}

pub fn to_string(token: Token) -> String {
  case token {
    Illegal -> "ILLEGAL"
    EOF -> "EOF"

    Identifier(literal) -> literal
    Integer(literal) -> literal

    Assign -> "="
    Plus -> "+"
    Minus -> "-"
    Bang -> "!"
    Asterisk -> "*"
    Slash -> "/"
    LessThan -> "<"
    GreaterThan -> ">"
    Equal -> "=="
    NotEqual -> "!="

    Comma -> ","
    Semicolon -> ";"

    LParen -> "("
    RParen -> ")"
    LBrace -> "{"
    RBrace -> "}"

    Function -> "FUNCTION"
    Let -> "LET"
    True -> "TRUE"
    False -> "FALSE"
    If -> "IF"
    Else -> "ELSE"
    Return -> "RETURN"
  }
}

pub fn lookup_identifier(str: String) -> Token {
  case str {
    "fn" -> Function
    "let" -> Let
    "true" -> True
    "false" -> False
    "if" -> If
    "else" -> Else
    "return" -> Return
    _ -> Identifier(str)
  }
}
