import gleam/string
import token

pub opaque type Lexer {
  Lexer(input: String, position: Int, read_position: Int, ch: String)
}

pub fn new(input: String) -> Lexer {
  let lexer = Lexer(input, 0, 0, "")
  read_char(lexer)
}

pub fn next_token(lexer: Lexer) -> #(Lexer, token.Token) {
  case lexer.ch {
    "=" -> #(read_char(lexer), token.Assign)
    ";" -> #(read_char(lexer), token.Semicolon)
    "(" -> #(read_char(lexer), token.LParen)
    ")" -> #(read_char(lexer), token.RParen)
    "," -> #(read_char(lexer), token.Comma)
    "+" -> #(read_char(lexer), token.Plus)
    "{" -> #(read_char(lexer), token.LBrace)
    "}" -> #(read_char(lexer), token.RBrace)
    _ -> #(read_char(lexer), token.EOF)
  }
}

fn read_char(lexer: Lexer) -> Lexer {
  case is_end_of_input(lexer) {
    True ->
      Lexer(
        lexer.input,
        position: lexer.read_position,
        read_position: lexer.read_position + 1,
        ch: "",
      )
    False -> {
      let ch = string.slice(lexer.input, lexer.read_position, 1)
      Lexer(lexer.input, lexer.read_position, lexer.read_position + 1, ch)
    }
  }
}

fn is_end_of_input(lexer: Lexer) -> Bool {
  lexer.read_position >= string.length(lexer.input)
}
