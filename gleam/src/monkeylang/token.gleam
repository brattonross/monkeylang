pub type Token {
  EOF
  Illegal

  Identifier(String)
  Int(Int)

  Assign
  Plus
  Minus
  Bang
  Asterisk
  Slash

  LessThan
  GreaterThan

  Comma
  Semicolon

  LParen
  RParen
  LBrace
  RBrace

  Function
  Let
}
