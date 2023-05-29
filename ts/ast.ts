import type { Token } from "./token.ts";

export type Node = {
  readonly type: string;
  tokenLiteral(): string;
};

/* ----------------------------------------------------------------------------
 * Expressions
 * -------------------------------------------------------------------------- */

export class IdentifierExpression implements Node {
  public readonly type = "IDENTIFIER_EXPRESSION";

  public constructor(public token: Token, public value: string) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.value;
  }
}

export class IntegerExpression implements Node {
  public readonly type = "INTEGER_EXPRESSION";

  public constructor(public token: Token, public value: number) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.token.literal;
  }
}

export class StringExpression implements Node {
  public readonly type = "STRING_EXPRESSION";

  public constructor(public token: Token, public value: string) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.token.literal;
  }
}

export class PrefixExpression implements Node {
  public readonly type = "PREFIX_EXPRESSION";

  public constructor(
    public token: Token,
    public operator: string,
    public right: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return `(${this.operator}${this.right?.toString()})`;
  }
}

export class InfixExpression implements Node {
  public readonly type = "INFIX_EXPRESSION";

  public constructor(
    public token: Token,
    public left: Expression | null,
    public operator: string,
    public right: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return `(${this.left?.toString()} ${
      this.operator
    } ${this.right?.toString()})`;
  }
}

export class BooleanExpression implements Node {
  public readonly type = "BOOLEAN_EXPRESSION";

  public constructor(public token: Token, public value: boolean) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.token.literal;
  }
}

export class IfExpression implements Node {
  public readonly type = "IF_EXPRESSION";

  public constructor(
    public token: Token,
    public condition: Expression | null,
    public consequence: BlockStatement | null,
    public alternative: BlockStatement | null
  ) {}

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

export class FunctionExpression implements Node {
  public readonly type = "FUNCTION_EXPRESSION";

  public constructor(
    public token: Token,
    public parameters: Array<IdentifierExpression>,
    public body: BlockStatement | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const params: Array<string> = [];
    for (let i = 0; i < this.parameters.length; i++) {
      params.push(this.parameters[i]!.toString());
    }
    return `${this.tokenLiteral()}(${params.join(
      ", "
    )}) ${this.body?.toString()}`;
  }
}

export class CallExpression implements Node {
  public readonly type = "CALL_EXPRESSION";

  public constructor(
    public token: Token,
    public func: Expression | null,
    public args: Array<Expression>
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const args: Array<string> = [];
    for (let i = 0; i < this.args.length; i++) {
      args.push(this.args[i]!.toString());
    }
    return `${this.func?.toString()}(${args.join(", ")})`;
  }
}

export class ArrayExpression implements Node {
  public readonly type = "ARRAY_EXPRESSION";

  public constructor(public token: Token, public elements: Array<Expression>) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const elements: Array<string> = [];
    for (let i = 0; i < this.elements.length; i++) {
      elements.push(this.elements[i]!.toString());
    }
    return `[${elements.join(", ")}]`;
  }
}

export class IndexExpression implements Node {
  public readonly type = "INDEX_EXPRESSION";

  public constructor(
    public token: Token,
    public left: Expression | null,
    public index: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return `(${this.left?.toString()}[${this.index?.toString()}])`;
  }
}

export class HashExpression implements Node {
  public readonly type = "HASH_EXPRESSION";

  public constructor(
    public token: Token,
    public pairs: Map<Expression, Expression>
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const pairs: Array<string> = [];
    this.pairs.forEach((value, key) => {
      pairs.push(`${key.toString()}:${value.toString()}`);
    });
    return `{${pairs.join(", ")}}`;
  }
}

export type Expression =
  | IdentifierExpression
  | IntegerExpression
  | StringExpression
  | PrefixExpression
  | InfixExpression
  | BooleanExpression
  | IfExpression
  | FunctionExpression
  | CallExpression
  | ArrayExpression
  | IndexExpression
  | HashExpression;

/* ----------------------------------------------------------------------------
 * Statements
 * -------------------------------------------------------------------------- */

export class LetStatement implements Node {
  public readonly type = "LET_STATEMENT";

  public constructor(
    public token: Token,
    public name: IdentifierExpression,
    public value: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const value = this.value ? this.value.toString() : "";
    return `${this.tokenLiteral()} ${this.name.toString()} = ${value};`;
  }
}

export class ReturnStatement implements Node {
  public readonly type = "RETURN_STATEMENT";

  public constructor(
    public token: Token,
    public returnValue: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    const value = this.returnValue ? this.returnValue.toString() : "";
    return `${this.tokenLiteral()} ${value};`;
  }
}

export class ExpressionStatement implements Node {
  public readonly type = "EXPRESSION_STATEMENT";

  public constructor(
    public token: Token,
    public expression: Expression | null
  ) {}

  public tokenLiteral(): string {
    return this.token.literal;
  }

  public toString(): string {
    return this.expression ? this.expression.toString() : "";
  }
}

export class BlockStatement implements Node {
  public readonly type = "BLOCK_STATEMENT";

  public constructor(
    public token: Token,
    public statements: Array<Statement>
  ) {}

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
  public readonly type = "PROGRAM";

  public constructor(public statements: Array<Statement> = []) {}

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

export type NodeType = Program | Statement | Expression;
