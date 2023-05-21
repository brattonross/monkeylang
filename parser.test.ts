import { expect, test } from "vitest";
import type {
  ExpressionStatement,
  Identifier,
  InfixExpression,
  IntegerLiteral,
  LetStatement,
  PrefixExpression,
} from "./ast.ts";
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
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(3);

  const tests = ["x", "y", "foobar"];

  for (let i = 0; i < tests.length; i++) {
    const statement = program.statements[i]!;
    expect(statement.tokenLiteral()).toBe("let");

    const letStatement = statement as LetStatement;
    expect(letStatement.name.value).toBe(tests[i]);
    expect(letStatement.name.tokenLiteral()).toBe(tests[i]);
  }
});

test("return statements", () => {
  const input = `
return 5;
return 10;
return 993322;
`;

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(3);

  for (const statement of program.statements) {
    expect(statement.tokenLiteral()).toBe("return");
  }
});

test("identifier expression", () => {
  const input = "foobar;";

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(1);

  const statement = program.statements[0] as ExpressionStatement;
  const identifier = statement.expression as Identifier;

  expect(identifier.value).toBe("foobar");
  expect(identifier.tokenLiteral()).toBe("foobar");
});

test("integer literal expression", () => {
  const input = "5;";

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(1);

  const statement = program.statements[0] as ExpressionStatement;
  const literal = statement.expression as IntegerLiteral;
  expect(literal.value).toBe(5);
  expect(literal.tokenLiteral()).toBe("5");
});

test("prefix expression", () => {
  const tests = [
    ["!5;", "!", 5],
    ["-15;", "-", 15],
  ] as const;

  for (const [input, operator, value] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.statements.length).toBe(1);

    const statement = program.statements[0] as ExpressionStatement;
    const expression = statement.expression as PrefixExpression;

    expect(expression.operator).toBe(operator);

    const integerLiteral = expression.right as IntegerLiteral;
    expect(integerLiteral.value).toBe(value);
    expect(integerLiteral.tokenLiteral()).toBe(`${value}`);
  }
});

test("infix expressions", () => {
  const tests = [
    ["5 + 5;", 5, "+", 5],
    ["5 - 5;", 5, "-", 5],
    ["5 * 5;", 5, "*", 5],
    ["5 / 5;", 5, "/", 5],
    ["5 > 5;", 5, ">", 5],
    ["5 < 5;", 5, "<", 5],
    ["5 == 5;", 5, "==", 5],
    ["5 != 5;", 5, "!=", 5],
  ] as const;

  for (const [input, leftValue, operator, rightValue] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.statements.length).toBe(1);

    const statement = program.statements[0] as ExpressionStatement;
    const expression = statement.expression as InfixExpression;

    const left = expression.left as IntegerLiteral;
    expect(left.value).toBe(leftValue);
    expect(left.tokenLiteral()).toBe(`${leftValue}`);

    expect(expression.operator).toBe(operator);

    const right = expression.right as IntegerLiteral;
    expect(right.value).toBe(rightValue);
    expect(right.tokenLiteral()).toBe(`${rightValue}`);
  }
});

test("operator precedence", () => {
  const tests = [
    ["-a * b", "((-a) * b)"],
    ["!-a", "(!(-a))"],
    ["a + b + c", "((a + b) + c)"],
    ["a + b - c", "((a + b) - c)"],
    ["a * b * c", "((a * b) * c)"],
    ["a * b / c", "((a * b) / c)"],
    ["a + b / c", "(a + (b / c))"],
    ["a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"],
    ["3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"],
    ["5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"],
    ["5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"],
    ["3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"],
  ] as const;

  for (const [input, expected] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.toString()).toBe(expected);
  }
});
