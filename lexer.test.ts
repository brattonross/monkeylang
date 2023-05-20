import { expect, test } from "vitest";
import { Lexer } from "./lexer";
import { TokenType } from "./token";

test("nextToken", () => {
    const input = `let five = 5;
let ten = 10;

let add = fn(x, y) {
    x + y;
};

let result = add(five, ten);
`;

    const tests: Array<{ expectedType: TokenType; expectedLiteral: string }> = [
        { expectedType: "LET", expectedLiteral: "let" },
        { expectedType: "IDENT", expectedLiteral: "five" },
        { expectedType: "=", expectedLiteral: "=" },
        { expectedType: "INT", expectedLiteral: "5" },
        { expectedType: ";", expectedLiteral: ";" },
        { expectedType: "LET", expectedLiteral: "let" },
        { expectedType: "IDENT", expectedLiteral: "ten" },
        { expectedType: "=", expectedLiteral: "=" },
        { expectedType: "INT", expectedLiteral: "10" },
        { expectedType: ";", expectedLiteral: ";" },
        { expectedType: "LET", expectedLiteral: "let" },
        { expectedType: "IDENT", expectedLiteral: "add" },
        { expectedType: "=", expectedLiteral: "=" },
        { expectedType: "FUNCTION", expectedLiteral: "fn" },
        { expectedType: "(", expectedLiteral: "(" },
        { expectedType: "IDENT", expectedLiteral: "x" },
        { expectedType: ",", expectedLiteral: "," },
        { expectedType: "IDENT", expectedLiteral: "y" },
        { expectedType: ")", expectedLiteral: ")" },
        { expectedType: "{", expectedLiteral: "{" },
        { expectedType: "IDENT", expectedLiteral: "x" },
        { expectedType: "+", expectedLiteral: "+" },
        { expectedType: "IDENT", expectedLiteral: "y" },
        { expectedType: ";", expectedLiteral: ";" },
        { expectedType: "}", expectedLiteral: "}" },
        { expectedType: ";", expectedLiteral: ";" },
        { expectedType: "LET", expectedLiteral: "let" },
        { expectedType: "IDENT", expectedLiteral: "result" },
        { expectedType: "=", expectedLiteral: "=" },
        { expectedType: "IDENT", expectedLiteral: "add" },
        { expectedType: "(", expectedLiteral: "(" },
        { expectedType: "IDENT", expectedLiteral: "five" },
        { expectedType: ",", expectedLiteral: "," },
        { expectedType: "IDENT", expectedLiteral: "ten" },
        { expectedType: ")", expectedLiteral: ")" },
        { expectedType: ";", expectedLiteral: ";" },
        { expectedType: "EOF", expectedLiteral: "" },
    ];

    const l = new Lexer(input);

    for (const test of tests) {
        const tok = l.nextToken();

        expect(tok.type).toBe(test.expectedType);
        expect(tok.literal).toBe(test.expectedLiteral);
    }
});
