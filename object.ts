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

export type ObjectType = IntegerObject | BooleanObject | NullObject;
