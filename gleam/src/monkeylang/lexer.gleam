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

pub fn new(input: String) -> Lexer {
  Lexer(
    input
      |> string.to_graphemes,
    0,
  )
}

pub fn next_token(lexer: Lexer) -> #(Lexer, Token) {
  let lexer =
    lexer
    |> skip_whitespace

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
    Some("-") -> #(lexer, token.Minus)
    Some("!") -> #(lexer, token.Bang)
    Some("*") -> #(lexer, token.Asterisk)
    Some("/") -> #(lexer, token.Slash)
    Some("<") -> #(lexer, token.LessThan)
    Some(">") -> #(lexer, token.GreaterThan)
    Some("{") -> #(lexer, token.LBrace)
    Some("}") -> #(lexer, token.RBrace)
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
            _ -> #(lexer, token.Illegal)
          }
      }
    None -> #(lexer, token.EOF)
  }
  #(
    lexer
      |> advance,
    token,
  )
}

fn current_grapheme(lexer: Lexer) -> Option(String) {
  lexer.graphemes
  |> list.at(lexer.pos)
  |> option.from_result
}

fn peek_grapheme(lexer: Lexer) -> Option(String) {
  lexer.graphemes
  |> list.at(lexer.pos + 1)
  |> option.from_result
}

fn advance(lexer: Lexer) -> Lexer {
  Lexer(..lexer, pos: lexer.pos + 1)
}

const whitespace_graphemes: List(String) = [" ", "\t", "\n", "\r"]

fn is_whitespace(grapheme: String) -> Bool {
  whitespace_graphemes
  |> list.contains(grapheme)
}

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

fn is_letter(grapheme: String) -> Bool {
  grapheme
  |> string.to_utf_codepoints
  |> list.first
  |> result.map(string.utf_codepoint_to_int)
  |> result.map(is_letter_codepoint)
  |> result.unwrap(False)
}

fn is_letter_codepoint(codepoint: Int) -> Bool {
  codepoint >= 65
  && codepoint <= 90
  || codepoint >= 97
  && codepoint <= 122
  || codepoint == 95
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

fn read_identifier(lexer: Lexer) -> #(Lexer, String) {
  lexer
  |> do_read_identifier("")
}

fn do_read_identifier(lexer: Lexer, literal: String) -> #(Lexer, String) {
  case
    lexer
    |> current_grapheme
  {
    Some(current) -> {
      let literal = literal <> current
      case
        lexer
        |> peek_grapheme
      {
        Some(peek) -> {
          case
            peek
            |> is_letter
          {
            True ->
              lexer
              |> advance
              |> do_read_identifier(literal)
            False -> #(lexer, literal)
          }
        }
        None -> #(lexer, literal)
      }
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
    Some(current) -> {
      let literal = literal <> current
      case
        lexer
        |> peek_grapheme
      {
        Some(peek) -> {
          case
            peek
            |> is_digit
          {
            True ->
              lexer
              |> advance
              |> do_try_parse_int(literal)
            False -> #(lexer, literal)
          }
        }
        None -> #(lexer, literal)
      }
    }
    None -> #(lexer, literal)
  }
}
