import gleam/list
import gleeunit/should
import monkeylang/lexer.{type Lexer}
import monkeylang/token.{type Token}

pub fn next_token_test() {
  [
    #("=+(){},;", [
      token.Assign,
      token.Plus,
      token.LParen,
      token.RParen,
      token.LBrace,
      token.RBrace,
      token.Comma,
      token.Semicolon,
      token.EOF,
    ]),
    #(
      "
      let five = 5;
      let ten = 10;

      let add = fn(x, y) {
        x + y;
      };

      let result = add(five, ten);
",
      [
        token.Let,
        token.Identifier("five"),
        token.Assign,
        token.Int(5),
        token.Semicolon,
        token.Let,
        token.Identifier("ten"),
        token.Assign,
        token.Int(10),
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
        token.EOF,
      ],
    ),
  ]
  |> list.each(fn(args) {
    let #(input, expected) = args
    lexer.new(input)
    |> read_to_end
    |> should.equal(expected)
  })
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
