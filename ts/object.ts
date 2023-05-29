import type { BlockStatement, IdentifierExpression } from "./ast";

export type Object = {
  readonly type: string;
  inspect(): string;
};

export type Hashable = {
  hashCode(): number;
};

export class IntegerObject implements Object, Hashable {
  public readonly type = "INTEGER";

  public constructor(public value: number) {}

  public inspect(): string {
    return this.value.toString();
  }

  public hashCode(): number {
    return this.value;
  }
}

export class StringObject implements Object, Hashable {
  public readonly type = "STRING";

  public constructor(public value: string) {}

  public inspect(): string {
    return this.value;
  }

  public hashCode(): number {
    let hash = 0;

    for (let i = 0; i < this.value.length; i++) {
      hash = (hash << 5) - hash + this.value.charCodeAt(i);
      hash = hash & hash;
    }

    return hash;
  }
}

export class BooleanObject implements Object, Hashable {
  public readonly type = "BOOLEAN";

  public constructor(public value: boolean) {}

  public inspect(): string {
    return this.value.toString();
  }

  public hashCode(): number {
    return this.value ? 1 : 0;
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

  public constructor(public value: ObjectType) {}

  public inspect(): string {
    return this.value.inspect();
  }
}

export class ErrorObject implements Object {
  public readonly type = "ERROR";

  public constructor(public message: string) {}

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
  ) {}

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

export class BuiltinFunctionObject implements Object {
  public readonly type = "BUILTIN_FUNCTION";

  public constructor(public fn: (...args: Array<ObjectType>) => ObjectType) {}

  public inspect(): string {
    return "builtin function";
  }
}

export class ArrayObject implements Object {
  public readonly type = "ARRAY";

  public constructor(public elements: Array<ObjectType>) {}

  public inspect(): string {
    let out = "[";
    for (let i = 0; i < this.elements.length; i++) {
      out += this.elements[i]!.inspect();
      if (i !== this.elements.length - 1) {
        out += ", ";
      }
    }
    out += "]";
    return out;
  }
}

export type HashPair = {
  key: ObjectType;
  value: ObjectType;
};

export class HashObject implements Object {
  public readonly type = "HASH";

  public constructor(public pairs: Map<number, HashPair>) {}

  public inspect(): string {
    let out = "{";
    const keys = Array.from(this.pairs.keys());
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i]!;
      const pair = this.pairs.get(key)!;
      out += pair.key.inspect();
      out += ": ";
      out += pair.value.inspect();
      if (i !== keys.length - 1) {
        out += ", ";
      }
    }
    out += "}";
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
  | FunctionObject
  | BuiltinFunctionObject
  | ArrayObject
  | HashObject;

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
