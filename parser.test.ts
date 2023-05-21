import { expect, test } from "vitest";
import type {
  BooleanExpression,
  ExpressionStatement,
  IdentifierExpression,
  IfExpression,
  InfixExpression,
  IntegerExpression,
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
  const identifier = statement.expression as IdentifierExpression;

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
  const literal = statement.expression as IntegerExpression;
  expect(literal.value).toBe(5);
  expect(literal.tokenLiteral()).toBe("5");
});

test("prefix expression", () => {
  const tests = [
    ["!5;", "!", 5],
    ["-15;", "-", 15],
    ["!true;", "!", true],
    ["!false;", "!", false],
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

    const integerLiteral = expression.right as IntegerExpression;
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
    ["true == true", true, "==", true],
    ["true != false", true, "!=", false],
    ["false == false", false, "==", false],
  ] as const;

  for (const [input, leftValue, operator, rightValue] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.statements.length).toBe(1);

    const statement = program.statements[0] as ExpressionStatement;
    const expression = statement.expression as InfixExpression;

    const left = expression.left as IntegerExpression;
    expect(left.value).toBe(leftValue);
    expect(left.tokenLiteral()).toBe(`${leftValue}`);

    expect(expression.operator).toBe(operator);

    const right = expression.right as IntegerExpression;
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
    ["true", "true"],
    ["false", "false"],
    ["3 > 5 == false", "((3 > 5) == false)"],
    ["3 < 5 == true", "((3 < 5) == true)"],
    ["1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"],
    ["(5 + 5) * 2", "((5 + 5) * 2)"],
    ["2 / (5 + 5)", "(2 / (5 + 5))"],
    ["-(5 + 5)", "(-(5 + 5))"],
    ["!(true == true)", "(!(true == true))"],
  ] as const;

  for (const [input, expected] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.toString()).toBe(expected);
  }
});

test("boolean expression", () => {
  const tests = [
    ["true;", true],
    ["false;", false],
  ] as const;

  for (const [input, value] of tests) {
    const lexer = new Lexer(input);
    const parser = new Parser(lexer);

    const program = parser.parseProgram();
    expect(parser.errors.length).toBe(0);
    expect(program.statements.length).toBe(1);

    const statement = program.statements[0] as ExpressionStatement;
    const boolean = statement.expression as BooleanExpression;

    expect(boolean.value).toBe(value);
    expect(boolean.tokenLiteral()).toBe(`${value}`);
  }
});

test("if expression", () => {
  const input = "if (x < y) { x }";

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(1);

  const statement = program.statements[0] as ExpressionStatement;
  const expression = statement.expression as IfExpression;

  expect(expression.condition?.toString()).toBe("(x < y)");
  expect(expression.consequence?.statements.length).toBe(1);

  const consequence = expression.consequence
    ?.statements[0] as ExpressionStatement;
  const identifier = consequence.expression as IdentifierExpression;
  expect(identifier.value).toBe("x");
  expect(identifier.tokenLiteral()).toBe("x");

  expect(expression.alternative).toBeNull();
});

test("if else expression", () => {
  const input = "if (x < y) { x } else { y }";

  const lexer = new Lexer(input);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  expect(parser.errors.length).toBe(0);
  expect(program.statements.length).toBe(1);

  const statement = program.statements[0] as ExpressionStatement;
  const expression = statement.expression as IfExpression;

  expect(expression.condition?.toString()).toBe("(x < y)");
  expect(expression.consequence?.statements.length).toBe(1);

  const consequence = expression.consequence
    ?.statements[0] as ExpressionStatement;
  const identifier = consequence.expression as IdentifierExpression;
  expect(identifier.value).toBe("x");
  expect(identifier.tokenLiteral()).toBe("x");

  expect(expression.alternative?.statements.length).toBe(1);

  const alternative = expression.alternative
    ?.statements[0] as ExpressionStatement;
  const identifier2 = alternative.expression as IdentifierExpression;
  expect(identifier2.value).toBe("y");
  expect(identifier2.tokenLiteral()).toBe("y");
});
