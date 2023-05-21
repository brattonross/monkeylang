import {
  Identifier,
  LetStatement,
  Program,
  ReturnStatement,
  type Expression,
  type Statement,
  ExpressionStatement,
  IntegerLiteral,
  PrefixExpression,
  InfixExpression,
} from "./ast.ts";
import type { Lexer } from "./lexer.ts";
import type { Token, TokenType } from "./token.ts";

type PrefixParseFn = () => Expression | null;
type InfixParseFn = (expression: Expression | null) => Expression;

const Precedence = {
  LOWEST: 1,
  EQUALS: 2, // ==
  LESSGREATER: 3, // > or <
  SUM: 4, // +
  PRODUCT: 5, // *
  PREFIX: 6, // -X or !X
  CALL: 7, // myFunction(X)
} as const;

const TokenPrecedence = new Map<TokenType, number>([
  ["EQ", Precedence.EQUALS],
  ["NOT_EQ", Precedence.EQUALS],
  ["LT", Precedence.LESSGREATER],
  ["GT", Precedence.LESSGREATER],
  ["PLUS", Precedence.SUM],
  ["MINUS", Precedence.SUM],
  ["SLASH", Precedence.PRODUCT],
  ["ASTERISK", Precedence.PRODUCT],
]);

export class Parser {
  #lexer: Lexer;
  #errors: Array<string>;

  #currentToken: Token;
  #peekToken: Token;

  #prefixParseFns: Map<TokenType, PrefixParseFn>;
  #infixParseFns: Map<TokenType, InfixParseFn>;

  public constructor(lexer: Lexer) {
    this.#lexer = lexer;
    this.#errors = [];

    this.#currentToken = this.#lexer.nextToken();
    this.#peekToken = this.#lexer.nextToken();

    this.#prefixParseFns = new Map<TokenType, PrefixParseFn>([
      ["IDENT", this.#parseIdentifier.bind(this)],
      ["INT", this.#parseIntegerLiteral.bind(this)],
      ["BANG", this.#parsePrefixExpression.bind(this)],
      ["MINUS", this.#parsePrefixExpression.bind(this)],
    ]);
    this.#infixParseFns = new Map<TokenType, InfixParseFn>([
      ["PLUS", this.#parseInfixExpression.bind(this)],
      ["MINUS", this.#parseInfixExpression.bind(this)],
      ["SLASH", this.#parseInfixExpression.bind(this)],
      ["ASTERISK", this.#parseInfixExpression.bind(this)],
      ["EQ", this.#parseInfixExpression.bind(this)],
      ["NOT_EQ", this.#parseInfixExpression.bind(this)],
      ["LT", this.#parseInfixExpression.bind(this)],
      ["GT", this.#parseInfixExpression.bind(this)],
    ]);
  }

  public parseProgram(): Program {
    const program = new Program();

    while (this.#currentToken.type !== "EOF") {
      const statement = this.#parseStatement();
      if (statement !== null) {
        program.statements.push(statement);
      }
      this.#nextToken();
    }

    return program;
  }

  public get errors(): Array<string> {
    return this.#errors;
  }

  #nextToken(): void {
    this.#currentToken = this.#peekToken;
    this.#peekToken = this.#lexer.nextToken();
  }

  #parseStatement(): Statement | null {
    switch (this.#currentToken.type) {
      case "LET":
        return this.#parseLetStatement();
      case "RETURN":
        return this.#parseReturnStatement();
      default:
        return this.#parseExpressionStatement();
    }
  }

  #parseLetStatement(): LetStatement | null {
    const token = this.#currentToken;

    if (!this.#expectPeek("IDENT")) {
      return null;
    }

    const name = new Identifier(this.#currentToken, this.#currentToken.literal);

    if (!this.#expectPeek("ASSIGN")) {
      return null;
    }

    while (this.#currentToken.type !== "SEMICOLON") {
      this.#nextToken();
    }

    return new LetStatement(token, name, null);
  }

  #parseReturnStatement(): Statement | null {
    const token = this.#currentToken;

    this.#nextToken();

    while (this.#currentToken.type !== "SEMICOLON") {
      this.#nextToken();
    }

    return new ReturnStatement(token, null);
  }

  #parseExpressionStatement(): Statement {
    const token = this.#currentToken;
    const expression = this.#parseExpression(Precedence.LOWEST);

    if (this.#peekToken.type === "SEMICOLON") {
      this.#nextToken();
    }

    return new ExpressionStatement(token, expression);
  }

  #noPrefixParseFnError(type: TokenType): void {
    this.#errors.push(`no prefix parse function for ${type} found`);
  }

  #parseExpression(precedence: number): Expression | null {
    const prefix = this.#prefixParseFns.get(this.#currentToken.type);
    if (prefix === undefined) {
      this.#noPrefixParseFnError(this.#currentToken.type);
      return null;
    }

    let leftExpression = prefix();

    while (
      this.#peekToken.type !== "SEMICOLON" &&
      precedence < this.#peekPrecedence()
    ) {
      const infix = this.#infixParseFns.get(this.#peekToken.type);
      if (infix === undefined) {
        return leftExpression;
      }

      this.#nextToken();

      leftExpression = infix(leftExpression);
    }

    return leftExpression;
  }

  #parseIdentifier(): Expression {
    return new Identifier(this.#currentToken, this.#currentToken.literal);
  }

  #parseIntegerLiteral(): Expression | null {
    const token = this.#currentToken;
    const value = Number(this.#currentToken.literal);
    if (Number.isNaN(value)) {
      this.#errors.push(`could not parse ${token.literal} as integer`);
      return null;
    }
    return new IntegerLiteral(token, value);
  }

  #parsePrefixExpression(): Expression {
    const token = this.#currentToken;
    const operator = token.literal;

    this.#nextToken();

    return new PrefixExpression(
      token,
      operator,
      this.#parseExpression(Precedence.PREFIX)
    );
  }

  #parseInfixExpression(left: Expression | null): Expression {
    const token = this.#currentToken;
    const operator = token.literal;

    const precedence = this.#currentPrecedence();
    this.#nextToken();
    const right = this.#parseExpression(precedence);

    return new InfixExpression(token, left, operator, right);
  }

  #peekPrecedence(): number {
    return TokenPrecedence.get(this.#peekToken.type) ?? Precedence.LOWEST;
  }

  #currentPrecedence(): number {
    return TokenPrecedence.get(this.#currentToken.type) ?? Precedence.LOWEST;
  }

  #expectPeek(type: string): boolean {
    if (this.#peekToken.type === type) {
      this.#nextToken();
      return true;
    } else {
      this.#peekError(type);
      return false;
    }
  }

  #peekError(type: string): void {
    this.#errors.push(
      `expected next token to be ${type}, got ${this.#peekToken.type} instead`
    );
  }
}
