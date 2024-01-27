import gleam/list
import gleeunit
import gleeunit/should
import lexer
import token

pub fn main() {
  gleeunit.main()
}

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

  let l = lexer.new(input)
  let actual = read_all_tokens(l, [])

  actual
  |> should.equal(expected)
}

fn read_all_tokens(
  lexer: lexer.Lexer,
  tokens: List(token.Token),
) -> List(token.Token) {
  let #(lexer, token) = lexer.next_token(lexer)
  case token {
    token.EOF -> list.reverse([token, ..tokens])
    token -> read_all_tokens(lexer, [token, ..tokens])
  }
}
