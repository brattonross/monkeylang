import { expect, test } from "vitest";
import { Lexer } from "./lexer.ts";
import type { TokenType } from "./token.ts";

test("nextToken", () => {
  const input = `let five = 5;
let ten = 10;

let add = fn(x, y) {
    x + y;
};

let result = add(five, ten);
!-/*5;
5 < 10 > 5;

if (5 < 10) {
    return true;
} else {
    return false;
}

10 == 10;
10 != 9;`;

  const tests: Array<{ expectedType: TokenType; expectedLiteral: string }> = [
    { expectedType: "LET", expectedLiteral: "let" },
    { expectedType: "IDENT", expectedLiteral: "five" },
    { expectedType: "ASSIGN", expectedLiteral: "=" },
    { expectedType: "INT", expectedLiteral: "5" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "LET", expectedLiteral: "let" },
    { expectedType: "IDENT", expectedLiteral: "ten" },
    { expectedType: "ASSIGN", expectedLiteral: "=" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "LET", expectedLiteral: "let" },
    { expectedType: "IDENT", expectedLiteral: "add" },
    { expectedType: "ASSIGN", expectedLiteral: "=" },
    { expectedType: "FUNCTION", expectedLiteral: "fn" },
    { expectedType: "LPAREN", expectedLiteral: "(" },
    { expectedType: "IDENT", expectedLiteral: "x" },
    { expectedType: "COMMA", expectedLiteral: "," },
    { expectedType: "IDENT", expectedLiteral: "y" },
    { expectedType: "RPAREN", expectedLiteral: ")" },
    { expectedType: "LBRACE", expectedLiteral: "{" },
    { expectedType: "IDENT", expectedLiteral: "x" },
    { expectedType: "PLUS", expectedLiteral: "+" },
    { expectedType: "IDENT", expectedLiteral: "y" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "RBRACE", expectedLiteral: "}" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "LET", expectedLiteral: "let" },
    { expectedType: "IDENT", expectedLiteral: "result" },
    { expectedType: "ASSIGN", expectedLiteral: "=" },
    { expectedType: "IDENT", expectedLiteral: "add" },
    { expectedType: "LPAREN", expectedLiteral: "(" },
    { expectedType: "IDENT", expectedLiteral: "five" },
    { expectedType: "COMMA", expectedLiteral: "," },
    { expectedType: "IDENT", expectedLiteral: "ten" },
    { expectedType: "RPAREN", expectedLiteral: ")" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "BANG", expectedLiteral: "!" },
    { expectedType: "MINUS", expectedLiteral: "-" },
    { expectedType: "SLASH", expectedLiteral: "/" },
    { expectedType: "ASTERISK", expectedLiteral: "*" },
    { expectedType: "INT", expectedLiteral: "5" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "INT", expectedLiteral: "5" },
    { expectedType: "LT", expectedLiteral: "<" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "GT", expectedLiteral: ">" },
    { expectedType: "INT", expectedLiteral: "5" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "IF", expectedLiteral: "if" },
    { expectedType: "LPAREN", expectedLiteral: "(" },
    { expectedType: "INT", expectedLiteral: "5" },
    { expectedType: "LT", expectedLiteral: "<" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "RPAREN", expectedLiteral: ")" },
    { expectedType: "LBRACE", expectedLiteral: "{" },
    { expectedType: "RETURN", expectedLiteral: "return" },
    { expectedType: "TRUE", expectedLiteral: "true" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "RBRACE", expectedLiteral: "}" },
    { expectedType: "ELSE", expectedLiteral: "else" },
    { expectedType: "LBRACE", expectedLiteral: "{" },
    { expectedType: "RETURN", expectedLiteral: "return" },
    { expectedType: "FALSE", expectedLiteral: "false" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "RBRACE", expectedLiteral: "}" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "EQ", expectedLiteral: "==" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "INT", expectedLiteral: "10" },
    { expectedType: "NOT_EQ", expectedLiteral: "!=" },
    { expectedType: "INT", expectedLiteral: "9" },
    { expectedType: "SEMICOLON", expectedLiteral: ";" },
    { expectedType: "EOF", expectedLiteral: "" },
  ];

  const l = new Lexer(input);

  for (const test of tests) {
    const tok = l.nextToken();

    expect(tok.type).toBe(test.expectedType);
    expect(tok.literal).toBe(test.expectedLiteral);
  }
});
