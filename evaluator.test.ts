import { expect, test } from "vitest";
import { evaluate } from "./evaluator.ts";
import { Lexer } from "./lexer.ts";
import type { BooleanObject, IntegerObject } from "./object.ts";
import { Parser } from "./parser.ts";

function runEval(input: string) {
  const lexer = new Lexer(input);
  const parser = new Parser(lexer);
  const program = parser.parseProgram();
  return evaluate(program);
}

function testIntegerObject(obj: unknown, expected: number) {
  const integer = obj as IntegerObject;
  expect(integer.type).toBe("INTEGER");
  expect(integer.value).toBe(expected);
}

function testBooleanObject(obj: unknown, expected: boolean) {
  const b = obj as BooleanObject;
  expect(b.type).toBe("BOOLEAN");
  expect(b.value).toBe(expected);
}

test("eval integer expression", () => {
  const tests = [
    ["5", 5],
    ["10", 10],
    ["-5", -5],
    ["-10", -10],
    ["5 + 5 + 5 + 5 - 10", 10],
    ["2 * 2 * 2 * 2 * 2", 32],
    ["-50 + 100 + -50", 0],
    ["5 * 2 + 10", 20],
    ["5 + 2 * 10", 25],
    ["20 + 2 * -10", 0],
    ["50 / 2 * 2 + 10", 60],
    ["2 * (5 + 10)", 30],
    ["3 * 3 * 3 + 10", 37],
    ["3 * (3 * 3) + 10", 37],
    ["(5 + 10 * 2 + 15 / 3) * 2 + -10", 50],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testIntegerObject(value, expected);
  }
});

test("eval boolean expression", () => {
  const tests = [
    ["true", true],
    ["false", false],
    ["1 < 2", true],
    ["1 > 2", false],
    ["1 < 1", false],
    ["1 > 1", false],
    ["1 == 1", true],
    ["1 != 1", false],
    ["1 == 2", false],
    ["1 != 2", true],
    ["true == true", true],
    ["false == false", true],
    ["true == false", false],
    ["true != false", true],
    ["false != true", true],
    ["(1 < 2) == true", true],
    ["(1 < 2) == false", false],
    ["(1 > 2) == true", false],
    ["(1 > 2) == false", true],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testBooleanObject(value, expected);
  }
});

test("bang operator", () => {
  const tests = [
    ["!true", false],
    ["!false", true],
    ["!5", false],
    ["!!true", true],
    ["!!false", false],
    ["!!5", true],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testBooleanObject(value, expected);
  }
});
