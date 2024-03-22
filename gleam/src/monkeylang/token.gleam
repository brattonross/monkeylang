pub type Token {
  EOF
  Illegal

  Identifier(String)
  Int(Int)

  Assign
  Plus

  Comma
  Semicolon

  LParen
  RParen
  LBrace
  RBrace

  Function
  Let
}
