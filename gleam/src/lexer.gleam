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
  let lexer = skip_whitespace(lexer)
  case lexer.ch {
    "=" -> #(read_char(lexer), token.Assign)
    ";" -> #(read_char(lexer), token.Semicolon)
    "(" -> #(read_char(lexer), token.LParen)
    ")" -> #(read_char(lexer), token.RParen)
    "," -> #(read_char(lexer), token.Comma)
    "+" -> #(read_char(lexer), token.Plus)
    "{" -> #(read_char(lexer), token.LBrace)
    "}" -> #(read_char(lexer), token.RBrace)
    "\\0" -> #(lexer, token.EOF)
    ch ->
      case is_letter(ch) {
        True -> read_identifier(lexer)
        False ->
          case is_digit(ch) {
            True -> read_number(lexer)
            False -> #(read_char(lexer), token.Illegal)
          }
      }
  }
}

fn skip_whitespace(lexer: Lexer) -> Lexer {
  case lexer.ch {
    " " -> skip_whitespace(read_char(lexer))
    "\t" -> skip_whitespace(read_char(lexer))
    "\n" -> skip_whitespace(read_char(lexer))
    "\r" -> skip_whitespace(read_char(lexer))
    _ -> lexer
  }
}

fn read_char(lexer: Lexer) -> Lexer {
  case is_end_of_input(lexer) {
    True ->
      Lexer(
        lexer.input,
        position: lexer.read_position,
        read_position: lexer.read_position + 1,
        ch: "\\0",
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

fn read_identifier(lexer: Lexer) -> #(Lexer, token.Token) {
  let position = lexer.position
  let lexer = do_read_identifier(lexer)
  #(
    lexer,
    token.lookup_identifier(string.slice(
      lexer.input,
      position,
      lexer.position - position,
    )),
  )
}

fn do_read_identifier(lexer: Lexer) -> Lexer {
  case is_letter(lexer.ch) {
    True ->
      read_char(lexer)
      |> do_read_identifier
    False -> lexer
  }
}

fn is_letter(ch: String) -> Bool {
  is_between(ch, "a", "z") || is_between(ch, "A", "Z") || ch == "_"
}

fn is_between(ch: String, min_ch: String, max_ch: String) -> Bool {
  let assert [code] = string.to_utf_codepoints(ch)
  let assert [min_code] = string.to_utf_codepoints(min_ch)
  let assert [max_code] = string.to_utf_codepoints(max_ch)
  let i = string.utf_codepoint_to_int(code)
  let min = string.utf_codepoint_to_int(min_code)
  let max = string.utf_codepoint_to_int(max_code)
  i >= min && i <= max
}

fn is_digit(ch: String) -> Bool {
  let assert [code] = string.to_utf_codepoints(ch)
  let i = string.utf_codepoint_to_int(code)
  i >= 48 && i <= 57
}

fn read_number(lexer: Lexer) -> #(Lexer, token.Token) {
  let position = lexer.position
  let lexer = do_read_number(lexer)
  #(
    lexer,
    token.Integer(string.slice(lexer.input, position, lexer.position - position)),
  )
}

fn do_read_number(lexer: Lexer) -> Lexer {
  case is_digit(lexer.ch) {
    True ->
      read_char(lexer)
      |> do_read_number
    False -> lexer
  }
}
