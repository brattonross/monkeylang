import type { Token } from "./token.ts";

export type Node = {
  tokenLiteral(): string;
};

export type Statement = Node;

export type Expression = Node;

export class Program implements Node {
  public statements: Array<Statement> = [];

  public tokenLiteral(): string {
    if (this.statements[0]) {
      return this.statements[0].tokenLiteral();
    }
    return "";
  }
}

export class Identifier implements Expression {
  public constructor(public token: Token, public value: string) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }
}

export class LetStatement implements Statement {
  public constructor(
    public token: Token,
    public name: Identifier,
    public value: Expression
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }
}
