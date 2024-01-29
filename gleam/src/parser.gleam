import gleam/list
import ast
import lexer
import token

pub opaque type Parser {
  Parser(
    lexer: lexer.Lexer,
    current_token: token.Token,
    peek_token: token.Token,
    errors: List(String),
  )
}

pub fn new(lexer: lexer.Lexer) -> Parser {
  Parser(lexer, token.EOF, token.EOF, [])
  |> next_token
  |> next_token
}

pub fn parse_program(parser: Parser) -> ast.Program {
  parse_statements(parser, [])
  |> ast.Program
}

pub fn errors(parser: Parser) -> List(String) {
  parser.errors
}

fn parse_statements(
  parser: Parser,
  statements: List(ast.Statement),
) -> List(ast.Statement) {
  case parser.current_token {
    token.EOF -> list.reverse(statements)
    _ -> {
      case parse_statement(parser) {
        #(parser, Ok(statement)) ->
          parse_statements(next_token(parser), [statement, ..statements])
        #(parser, Error(Nil)) ->
          parse_statements(next_token(parser), statements)
      }
    }
  }
}

fn parse_statement(parser: Parser) -> #(Parser, Result(ast.Statement, Nil)) {
  case parser.current_token {
    token.Let -> parse_let_statement(parser)
    _ -> panic("unimplemented")
  }
}

fn parse_let_statement(parser: Parser) -> #(Parser, Result(ast.Statement, Nil)) {
  case parser.peek_token {
    token.Identifier(name) -> {
      let parser = next_token(parser)
      case parser.peek_token {
        token.Assign -> {
          // TODO: we are skipping the expressions until we encounter a semicolon
          let parser =
            next_token(parser)
            |> read_until_semicolon
          #(parser, Ok(ast.LetStatement(name, ast.Expression)))
        }
        _ -> #(
          Parser(
            ..parser,
            errors: [
              "expected next token to be Assign, got "
              <> parser.peek_token
              |> token.to_string
              <> " instead",
              ..parser.errors
            ],
          ),
          Error(Nil),
        )
      }
    }
    _ -> #(
      Parser(
        ..parser,
        errors: [
          "expected next token to be Identifier, got "
          <> parser.peek_token
          |> token.to_string
          <> " instead",
          ..parser.errors
        ],
      ),
      Error(Nil),
    )
  }
}

fn read_until_semicolon(parser: Parser) -> Parser {
  case parser.current_token {
    token.Semicolon -> parser
    _ -> read_until_semicolon(next_token(parser))
  }
}

fn next_token(parser: Parser) -> Parser {
  let #(lexer, token) = lexer.next_token(parser.lexer)
  Parser(
    ..parser,
    lexer: lexer,
    current_token: parser.peek_token,
    peek_token: token,
  )
}
