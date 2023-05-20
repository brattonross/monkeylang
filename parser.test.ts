import { expect, test } from "vitest";
import type { LetStatement } from "./ast.ts";
import { Lexer } from "./lexer.ts";
import { Parser } from "./parser.ts";

test("let statements", () => {
  const input = `
  let x = 5;
  let y = 10;
  let foobar = 838383;
  `;

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();

  expect(program.statements.length).toBe(3);

  const tests = ["x", "y", "foobar"];

  for (let i = 0; i < tests.length; i++) {
    const statement = program.statements[i];
    expect(statement.tokenLiteral()).toBe("let");

    const letStatement = statement as LetStatement;
    expect(letStatement.name.value).toBe(tests[i]);
    expect(letStatement.name.tokenLiteral()).toBe(tests[i]);
  }
});
