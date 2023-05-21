import { expect, test } from "vitest";
import { evaluate } from "./evaluator.ts";
import { Lexer } from "./lexer.ts";
import type { IntegerObject } from "./object.ts";
import { Parser } from "./parser.ts";

function runEval(input: string) {
  const lexer = new Lexer(input);
  const parser = new Parser(lexer);
  const program = parser.parseProgram();
  return evaluate(program);
}

function testIntegerObject(obj: IntegerObject, expected: number) {
  expect(obj.type).toBe("INTEGER");
  expect(obj.value).toBe(expected);
}

test("eval integer expression", () => {
  const tests = [
    ["5", 5],
    ["10", 10],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testIntegerObject(value as IntegerObject, expected);
  }
});
