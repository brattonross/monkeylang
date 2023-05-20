import { Identifier, LetStatement, Program, type Statement } from "./ast.ts";
import type { Lexer } from "./lexer.ts";
import type { Token } from "./token.ts";

export class Parser {
  #lexer: Lexer;
  #currentToken: Token;
  #peekToken: Token;
  #errors: Array<string> = [];

  public constructor(lexer: Lexer) {
    this.#lexer = lexer;
    this.#currentToken = this.#lexer.nextToken();
    this.#peekToken = this.#lexer.nextToken();
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
      default:
        return null;
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
