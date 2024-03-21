import gleam/list
import gleeunit/should
import monkeylang/lexer.{type Lexer}
import monkeylang/token.{type Token}

pub fn next_token_test() {
  let input = "=+(){},;"
  let expected = [
    token.Assign,
    token.Plus,
    token.LParen,
    token.RParen,
    token.LBrace,
    token.RBrace,
    token.Comma,
    token.Semicolon,
    token.EOF,
  ]

  lexer.new(input)
  |> read_to_end
  |> should.equal(expected)
}

fn read_to_end(lexer: Lexer) -> List(Token) {
  do_read_to_end(lexer, [])
  |> list.reverse
}

fn do_read_to_end(lexer: Lexer, tokens: List(Token)) {
  let #(lexer, token) = lexer.next_token(lexer)
  case token {
    token.EOF -> [token.EOF, ..tokens]
    _ -> do_read_to_end(lexer, [token, ..tokens])
  }
}
