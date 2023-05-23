import type { BlockStatement, IdentifierExpression } from "./ast";

export type Object = {
  readonly type: string;
  inspect(): string;
};

export class IntegerObject implements Object {
  public readonly type = "INTEGER";

  public constructor(public value: number) { }

  public inspect(): string {
    return this.value.toString();
  }
}

export class StringObject implements Object {
  public readonly type = "STRING";

  public constructor(public value: string) { }

  public inspect(): string {
    return this.value;
  }
}

export class BooleanObject implements Object {
  public readonly type = "BOOLEAN";

  public constructor(public value: boolean) { }

  public inspect(): string {
    return this.value.toString();
  }
}

export class NullObject implements Object {
  public readonly type = "NULL";

  public inspect(): string {
    return "null";
  }
}

export class ReturnValueObject implements Object {
  public readonly type = "RETURN_VALUE";

  public constructor(public value: ObjectType) { }

  public inspect(): string {
    return this.value.inspect();
  }
}

export class ErrorObject implements Object {
  public readonly type = "ERROR";

  public constructor(public message: string) { }

  public inspect(): string {
    return `ERROR: ${this.message}`;
  }
}

export class FunctionObject implements Object {
  public readonly type = "FUNCTION";

  public constructor(
    public parameters: Array<IdentifierExpression>,
    public body: BlockStatement | null,
    public env: Environment
  ) { }

  public inspect(): string {
    let out = "fn(";
    for (let i = 0; i < this.parameters.length; i++) {
      out += this.parameters[i]!.toString();
      if (i !== this.parameters.length - 1) {
        out += ", ";
      }
    }
    out += ") {\n";
    if (this.body) {
      out += this.body.toString();
    }
    out += "\n}";
    return out;
  }
}

export type ObjectType =
  | IntegerObject
  | StringObject
  | BooleanObject
  | NullObject
  | ReturnValueObject
  | ErrorObject
  | FunctionObject;

export class Environment {
  #store = new Map<string, ObjectType>();
  #outer: Environment | null = null;

  public static enclosed(outer: Environment): Environment {
    const env = new Environment();
    env.#outer = outer;
    return env;
  }

  public get(name: string): ObjectType | null {
    const obj = this.#store.get(name) ?? null;
    if (obj === null && this.#outer !== null) {
      return this.#outer.get(name);
    }
    return obj;
  }

  public set(name: string, value: ObjectType): ObjectType {
    this.#store.set(name, value);
    return value;
  }
}
