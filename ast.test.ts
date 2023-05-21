import { expect, test } from "vitest";
import { Identifier, LetStatement, Program } from "./ast.ts";
import { Token } from "./token.ts";

test("toString", () => {
  const program = new Program([
    new LetStatement(
      new Token("LET", "let"),
      new Identifier(new Token("IDENT", "myVar"), "myVar"),
      new Identifier(new Token("IDENT", "anotherVar"), "anotherVar")
    ),
  ]);

  expect(program.toString()).toBe("let myVar = anotherVar;");
});
