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

export type ObjectType =
  | IntegerObject
  | BooleanObject
  | NullObject
  | ReturnValueObject
  | ErrorObject;
