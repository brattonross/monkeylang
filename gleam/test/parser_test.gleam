import ast
import gleam/list
import gleeunit
import gleeunit/should
import lexer
import parser

pub fn main() {
  gleeunit.main()
}

pub fn let_statements_test() {
  let program =
    "let x = 5;
     let y = 10;
     let foobar = 838383;"
    |> lexer.new
    |> parser.new
    |> parser.parse_program

  program.statements
  |> list.length
  |> should.equal(3)

  list.each(list.zip(program.statements, ["x", "y", "foobar"]), fn(test_case) {
    let #(statement, expected_name) = test_case
    test_let_statement(statement, expected_name)
    |> should.be_ok
  })
}

fn test_let_statement(
  statement: ast.Statement,
  name: String,
) -> Result(Nil, String) {
  case statement {
    ast.LetStatement(n, _) -> {
      case n == name {
        True -> Ok(Nil)
        False -> Error("statement name not " <> name <> ". got=" <> n)
      }
    }
  }
}
