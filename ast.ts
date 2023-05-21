import type { Token } from "./token.ts";

export type Node = {
  tokenLiteral(): string;
};

/* ----------------------------------------------------------------------------
 * Expressions
 * -------------------------------------------------------------------------- */

export class IdentifierExpression implements Node {
  public constructor(public token: Token, public value: string) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.value;
  }
}

export class IntegerExpression implements Node {
  public constructor(public token: Token, public value: number) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.token.literal;
  }
}

export class PrefixExpression implements Node {
  public constructor(
    public token: Token,
    public operator: string,
    public right: Expression | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return `(${this.operator}${this.right?.toString()})`;
  }
}

export class InfixExpression implements Node {
  public constructor(
    public token: Token,
    public left: Expression | null,
    public operator: string,
    public right: Expression | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return `(${this.left?.toString()} ${this.operator
      } ${this.right?.toString()})`;
  }
}

export class BooleanExpression implements Node {
  public constructor(public token: Token, public value: boolean) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.token.literal;
  }
}

export class IfExpression implements Node {
  public constructor(
    public token: Token,
    public condition: Expression | null,
    public consequence: BlockStatement | null,
    public alternative: BlockStatement | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    let result = `if ${this.condition?.toString()} ${this.consequence?.toString()}`;
    if (this.alternative) {
      result += `else ${this.alternative.toString()}`;
    }
    return result;
  }
}

export type Expression =
  | IdentifierExpression
  | IntegerExpression
  | PrefixExpression
  | InfixExpression
  | BooleanExpression
  | IfExpression;

/* ----------------------------------------------------------------------------
 * Statements
 * -------------------------------------------------------------------------- */

export class LetStatement implements Node {
  public readonly type = "LetStatement";

  public constructor(
    public token: Token,
    public name: IdentifierExpression,
    public value: Expression | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const value = this.value ? this.value.toString() : "";
    return `${this.tokenLiteral()} ${this.name.toString()} = ${value};`;
  }
}

export class ReturnStatement implements Node {
  public readonly type = "ReturnStatement";

  public constructor(
    public token: Token,
    public returnValue: Expression | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const value = this.returnValue ? this.returnValue.toString() : "";
    return `${this.tokenLiteral()} ${value};`;
  }
}

export class ExpressionStatement implements Node {
  public readonly type = "ExpressionStatement";

  public constructor(
    public token: Token,
    public expression: Expression | null
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.expression ? this.expression.toString() : "";
  }
}

export class BlockStatement implements Node {
  public readonly type = "BlockStatement";

  public constructor(
    public token: Token,
    public statements: Array<Statement>
  ) { }

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    let out = "";
    for (let i = 0; i < this.statements.length; i++) {
      out += this.statements[i]!.toString();
    }
    return out;
  }
}

export type Statement =
  | LetStatement
  | ReturnStatement
  | ExpressionStatement
  | BlockStatement;

/* -------------------------------------------------------------------------- */

export class Program implements Node {
  public constructor(public statements: Array<Statement> = []) { }

  public tokenLiteral(): string {
    if (this.statements[0]) {
      return this.statements[0].tokenLiteral();
    }
    return "";
  }

  public toString(): string {
    let out = "";
    for (let i = 0; i < this.statements.length; i++) {
      out += this.statements[i]!.toString();
    }
    return out;
  }
}
