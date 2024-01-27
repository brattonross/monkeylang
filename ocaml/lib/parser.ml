type t =
  { lexer : Lexer.t
  ; current : Token.t option
  ; peek : Token.t option
  }
