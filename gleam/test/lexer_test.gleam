import gleam/list
import gleeunit
import gleeunit/should
import lexer
import token

pub fn main() {
  gleeunit.main()
}

pub fn next_token_test() {
  let input =
    "let five = 5;
let ten = 10;

let add = fn(x, y) {
  x + y;
};

let result = add(five, ten);
!-/*5;
5 < 10 > 5;

if (5 < 10) {
  return true;
} else {
  return false;
}

10 == 10;
10 != 9;
"
  let expected = [
    token.Let,
    token.Identifier("five"),
    token.Assign,
    token.Integer("5"),
    token.Semicolon,
    token.Let,
    token.Identifier("ten"),
    token.Assign,
    token.Integer("10"),
    token.Semicolon,
    token.Let,
    token.Identifier("add"),
    token.Assign,
    token.Function,
    token.LParen,
    token.Identifier("x"),
    token.Comma,
    token.Identifier("y"),
    token.RParen,
    token.LBrace,
    token.Identifier("x"),
    token.Plus,
    token.Identifier("y"),
    token.Semicolon,
    token.RBrace,
    token.Semicolon,
    token.Let,
    token.Identifier("result"),
    token.Assign,
    token.Identifier("add"),
    token.LParen,
    token.Identifier("five"),
    token.Comma,
    token.Identifier("ten"),
    token.RParen,
    token.Semicolon,
    token.Bang,
    token.Minus,
    token.Slash,
    token.Asterisk,
    token.Integer("5"),
    token.Semicolon,
    token.Integer("5"),
    token.LessThan,
    token.Integer("10"),
    token.GreaterThan,
    token.Integer("5"),
    token.Semicolon,
    token.If,
    token.LParen,
    token.Integer("5"),
    token.LessThan,
    token.Integer("10"),
    token.RParen,
    token.LBrace,
    token.Return,
    token.True,
    token.Semicolon,
    token.RBrace,
    token.Else,
    token.LBrace,
    token.Return,
    token.False,
    token.Semicolon,
    token.RBrace,
    token.Integer("10"),
    token.Equal,
    token.Integer("10"),
    token.Semicolon,
    token.Integer("10"),
    token.NotEqual,
    token.Integer("9"),
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
