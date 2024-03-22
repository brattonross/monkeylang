import gleam/int
import gleam/list
import gleam/option.{type Option, None, Some}
import gleam/pair
import gleam/result
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
  let lexer =
    lexer
    |> skip_whitespace

  case
    lexer
    |> current_grapheme
  {
    Some("=") -> #(
      lexer
        |> advance,
      token.Assign,
    )
    Some(";") -> #(
      lexer
        |> advance,
      token.Semicolon,
    )
    Some("(") -> #(
      lexer
        |> advance,
      token.LParen,
    )
    Some(")") -> #(
      lexer
        |> advance,
      token.RParen,
    )
    Some(",") -> #(
      lexer
        |> advance,
      token.Comma,
    )
    Some("+") -> #(
      lexer
        |> advance,
      token.Plus,
    )
    Some("{") -> #(
      lexer
        |> advance,
      token.LBrace,
    )
    Some("}") -> #(
      lexer
        |> advance,
      token.RBrace,
    )
    Some(grapheme) ->
      case
        grapheme
        |> is_letter
      {
        True ->
          lexer
          |> read_identifier
          |> pair.map_second(map_identifier_to_token)
        False ->
          case
            lexer
            |> try_parse_int
          {
            Ok(tuple) ->
              tuple
              |> pair.map_second(token.Int)
            _ -> #(
              lexer
                |> advance,
              token.Illegal,
            )
          }
      }
    None -> #(lexer, token.EOF)
  }
}

/// Get the grapheme at the position of the given lexer.
///
fn current_grapheme(lexer: Lexer) -> Option(String) {
  lexer.graphemes
  |> list.at(lexer.pos)
  |> option.from_result
}

/// Return a lexer with the position advanced to the next grapheme.
///
fn advance(lexer: Lexer) -> Lexer {
  Lexer(..lexer, pos: lexer.pos + 1)
}

const whitespace_graphemes: List(String) = [" ", "\t", "\n", "\r"]

/// Return whether the given grapheme is whitespace.
///
fn is_whitespace(grapheme: String) -> Bool {
  whitespace_graphemes
  |> list.contains(grapheme)
}

/// Advance the lexer until the next non-whitespace grapheme.
///
fn skip_whitespace(lexer: Lexer) -> Lexer {
  case
    lexer
    |> current_grapheme
    |> option.map(is_whitespace)
    |> option.unwrap(False)
  {
    True ->
      lexer
      |> advance
      |> skip_whitespace
    False -> lexer
  }
}

/// Return whether the given grapheme is a letter.
/// Assumes the provided string is a single grapheme.
///
fn is_letter(grapheme: String) -> Bool {
  grapheme
  |> string.to_utf_codepoints
  |> list.first
  |> result.map(string.utf_codepoint_to_int)
  |> result.map(is_letter_codepoint)
  |> result.unwrap(False)
}

fn is_letter_codepoint(codepoint: Int) -> Bool {
  codepoint >= 65 && codepoint <= 90 || codepoint >= 97 && codepoint <= 122
}

fn is_digit(grapheme: String) -> Bool {
  grapheme
  |> string.to_utf_codepoints
  |> list.first
  |> result.map(string.utf_codepoint_to_int)
  |> result.map(is_digit_codepoint)
  |> result.unwrap(False)
}

fn is_digit_codepoint(codepoint: Int) -> Bool {
  codepoint >= 48 && codepoint <= 57
}

/// Read an identifier from the given lexer.
///
fn read_identifier(lexer: Lexer) -> #(Lexer, String) {
  lexer
  |> do_read_identifier("")
}

fn do_read_identifier(lexer: Lexer, literal: String) -> #(Lexer, String) {
  case
    lexer
    |> current_grapheme
  {
    Some(grapheme) ->
      case
        grapheme
        |> is_letter
      {
        True ->
          lexer
          |> advance
          |> do_read_identifier(literal <> grapheme)
        False -> #(lexer, literal)
      }
    None -> #(lexer, literal)
  }
}

fn map_identifier_to_token(identifier: String) -> Token {
  case identifier {
    "fn" -> token.Function
    "let" -> token.Let
    _ -> token.Identifier(identifier)
  }
}

fn try_parse_int(lexer: Lexer) -> Result(#(Lexer, Int), Nil) {
  let #(lexer, literal) =
    lexer
    |> do_try_parse_int("")

  literal
  |> int.parse
  |> result.map(pair.new(lexer, _))
}

fn do_try_parse_int(lexer: Lexer, literal: String) -> #(Lexer, String) {
  case
    lexer
    |> current_grapheme
  {
    Some(grapheme) ->
      case
        grapheme
        |> is_digit
      {
        True ->
          lexer
          |> advance
          |> do_try_parse_int(literal <> grapheme)
        False -> #(lexer, literal)
      }
    None -> #(lexer, literal)
  }
}
