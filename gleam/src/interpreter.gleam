import gleam/erlang
import gleam/io
import lexer
import token

pub fn main() {
  let assert Ok(input) = erlang.get_line("> ")
  input
  |> lexer.new
  |> print_all_tokens
  main()
}

fn print_all_tokens(lexer: lexer.Lexer) {
  let #(lexer, token) =
    lexer
    |> lexer.next_token
  case token {
    token.EOF -> Nil
    token -> {
      io.println(
        token
        |> token.to_string,
      )
      lexer
      |> print_all_tokens
    }
  }
}
