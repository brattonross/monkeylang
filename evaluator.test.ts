import { expect, test } from "vitest";
import { NULL, evaluate } from "./evaluator.ts";
import { Lexer } from "./lexer.ts";
import {
  Environment,
  type BooleanObject,
  type ErrorObject,
  type IntegerObject,
  FunctionObject,
  StringObject,
} from "./object.ts";
import { Parser } from "./parser.ts";

function runEval(input: string) {
  const env = new Environment();
  const lexer = new Lexer(input);
  const parser = new Parser(lexer);
  const program = parser.parseProgram();
  return evaluate(program, env);
}

function testIntegerObject(obj: unknown, expected: number) {
  const integer = obj as IntegerObject;
  expect(integer.type).toBe("INTEGER");
  expect(integer.value).toBe(expected);
}

function testStringObject(obj: unknown, expected: string) {
  const str = obj as StringObject;
  expect(str.type).toBe("STRING");
  expect(str.value).toBe(expected);
}

function testBooleanObject(obj: unknown, expected: boolean) {
  const b = obj as BooleanObject;
  expect(b.type).toBe("BOOLEAN");
  expect(b.value).toBe(expected);
}

function testNullObject(obj: unknown) {
  expect(obj).toBe(NULL);
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

test("eval string expression", () => {
  const input = `"Hello World!"`;
  const value = runEval(input)!;
  testStringObject(value, "Hello World!");
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

test("if else expressions", () => {
  const tests = [
    ["if (true) { 10 }", 10],
    ["if (false) { 10 }", null],
    ["if (1) { 10 }", 10],
    ["if (1 < 2) { 10 }", 10],
    ["if (1 > 2) { 10 }", null],
    ["if (1 > 2) { 10 } else { 20 }", 20],
    ["if (1 < 2) { 10 } else { 20 }", 10],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    if (expected === null) {
      testNullObject(value);
    } else {
      testIntegerObject(value, expected);
    }
  }
});

test("return statements", () => {
  const tests = [
    ["return 10;", 10],
    ["return 10; 9;", 10],
    ["return 2 * 5; 9;", 10],
    ["9; return 2 * 5; 9;", 10],
    [
      `
if (10 > 1) {
  if (10 > 1) {
    return 10;
  }

  return 1;
}
`,
      10,
    ],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testIntegerObject(value, expected);
  }
});

test("error handling", () => {
  const tests = [
    ["5 + true;", "type mismatch: INTEGER + BOOLEAN"],
    ["5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"],
    ["-true", "unknown operator: -BOOLEAN"],
    ["true + false;", "unknown operator: BOOLEAN + BOOLEAN"],
    ["5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"],
    ["if (10 > 1) { true + false; }", "unknown operator: BOOLEAN + BOOLEAN"],
    [
      `
if (10 > 1) {
  if (10 > 1) {
    return true + false;
  }

  return 1;
}
`,
      "unknown operator: BOOLEAN + BOOLEAN",
    ],
    ["foobar", "identifier not found: foobar"],
    ['"Hello" - "World"', "unknown operator: STRING - STRING"],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input)!;
    expect(value.type).toBe("ERROR");

    const error = value as ErrorObject;
    expect(error.message).toBe(expected);
  }
});

test("let statements", () => {
  const tests = [
    ["let a = 5; a;", 5],
    ["let a = 5 * 5; a;", 25],
    ["let a = 5; let b = a; b;", 5],
    ["let a = 5; let b = a; let c = a + b + 5; c;", 15],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testIntegerObject(value, expected);
  }
});

test("function object", () => {
  const input = "fn(x) { x + 2; };";
  const value = runEval(input)!;

  expect(value.type).toBe("FUNCTION");

  const fn = value as FunctionObject;
  expect(fn.parameters.length).toBe(1);
  expect(fn.parameters[0]!.toString()).toBe("x");
  expect(fn.body!.toString()).toBe("(x + 2)");
});

test("function application", () => {
  const tests = [
    ["let identity = fn(x) { x; }; identity(5);", 5],
    ["let identity = fn(x) { return x; }; identity(5);", 5],
    ["let double = fn(x) { x * 2; }; double(5);", 10],
    ["let add = fn(x, y) { x + y; }; add(5, 5);", 10],
    ["let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20],
    ["fn(x) { x; }(5)", 5],
  ] as const;

  for (const [input, expected] of tests) {
    const value = runEval(input);
    testIntegerObject(value, expected);
  }
});

test("closures", () => {
  const input = `
let newAdder = fn(x) {
  fn(y) { x + y };
};

let addTwo = newAdder(2);
addTwo(2);`;

  const value = runEval(input);
  testIntegerObject(value, 4);
});

test("string concatenation", () => {
  const input = `"Hello" + " " + "World!"`;
  const value = runEval(input);
  testStringObject(value, "Hello World!");
});
