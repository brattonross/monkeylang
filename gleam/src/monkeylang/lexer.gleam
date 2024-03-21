import gleam/list
import gleam/option.{type Option, None, Some}
import gleam/string
import monkeylang/token.{type Token}

pub opaque type Lexer {
  Lexer(graphemes: List(String), pos: Int)
}

/// Create a new lexer from the given input string.
///
pub fn new(input: String) -> Lexer {
  Lexer(
    input
      |> string.to_graphemes,
    0,
  )
}

/// Read the next token and advance the lexer.
///
pub fn next_token(lexer: Lexer) -> #(Lexer, Token) {
  let #(lexer, token) = case
    lexer
    |> current_grapheme
  {
    Some("=") -> #(lexer, token.Assign)
    Some(";") -> #(lexer, token.Semicolon)
    Some("(") -> #(lexer, token.LParen)
    Some(")") -> #(lexer, token.RParen)
    Some(",") -> #(lexer, token.Comma)
    Some("+") -> #(lexer, token.Plus)
    Some("{") -> #(lexer, token.LBrace)
    Some("}") -> #(lexer, token.RBrace)
    _ -> #(lexer, token.EOF)
  }
  #(
    lexer
      |> advance,
    token,
  )
}

/// Get the grapheme at the position of the given lexer.
///
fn current_grapheme(lexer: Lexer) -> Option(String) {
  case
    lexer.graphemes
    |> list.at(lexer.pos)
  {
    Ok(grapheme) -> Some(grapheme)
    _ -> None
  }
}

/// Return a lexer with the position advanced to the next grapheme.
///
fn advance(lexer: Lexer) -> Lexer {
  Lexer(..lexer, pos: lexer.pos + 1)
}
