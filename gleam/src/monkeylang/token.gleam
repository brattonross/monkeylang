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
  Equal
  NotEqual

  Comma
  Semicolon

  LParen
  RParen
  LBrace
  RBrace

  Function
  Let
  True
  False
  If
  Else
  Return
}
