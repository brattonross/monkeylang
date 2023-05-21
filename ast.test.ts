import { expect, test } from "vitest";
import { IdentifierExpression, LetStatement, Program } from "./ast.ts";
import { Token } from "./token.ts";

test("toString", () => {
  const program = new Program([
    new LetStatement(
      new Token("LET", "let"),
      new IdentifierExpression(new Token("IDENT", "myVar"), "myVar"),
      new IdentifierExpression(new Token("IDENT", "anotherVar"), "anotherVar")
    ),
  ]);

  expect(program.toString()).toBe("let myVar = anotherVar;");
});
