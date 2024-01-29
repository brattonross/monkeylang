import gleam/list
import ast
import lexer
import token

pub opaque type Parser {
  Parser(
    lexer: lexer.Lexer,
    current_token: token.Token,
    peek_token: token.Token,
  )
}

pub fn new(lexer: lexer.Lexer) -> Parser {
  Parser(lexer, token.EOF, token.EOF)
  |> next_token
  |> next_token
}

pub fn parse_program(parser: Parser) -> ast.Program {
  parse_statements(parser, [])
  |> ast.Program
}

fn parse_statements(
  parser: Parser,
  statements: List(ast.Statement),
) -> List(ast.Statement) {
  case parser.current_token {
    token.EOF -> list.reverse(statements)
    _ -> {
      case parse_statement(parser) {
        Ok(#(parser, statement)) ->
          parse_statements(next_token(parser), [statement, ..statements])
        _ -> parse_statements(next_token(parser), statements)
      }
    }
  }
}

fn parse_statement(parser: Parser) -> Result(#(Parser, ast.Statement), Nil) {
  case parser.current_token {
    token.Let -> parse_let_statement(parser)
    _ -> panic("unimplemented")
  }
}

fn parse_let_statement(parser: Parser) -> Result(#(Parser, ast.Statement), Nil) {
  case parser.peek_token {
    token.Identifier(name) -> {
      let parser = next_token(parser)
      case parser.peek_token {
        token.Assign -> {
          // TODO: we are skipping the expressions until we encounter a semicolon
          let parser =
            next_token(parser)
            |> read_until_semicolon
          Ok(#(parser, ast.LetStatement(name, ast.Expression)))
        }
        _ -> Error(Nil)
      }
    }
    _ -> Error(Nil)
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
  Parser(lexer, parser.peek_token, token)
}
