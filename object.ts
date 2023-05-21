export type ObjectType = string;

export type Object = {
  type: ObjectType;
  inspect(): string;
};

export class IntegerObject implements Object {
  public type: ObjectType = "INTEGER";

  public constructor(public value: number) { }

  public inspect(): string {
    return this.value.toString();
  }
}

export class BooleanObject implements Object {
  public type: ObjectType = "BOOLEAN";

  public constructor(public value: boolean) { }

  public inspect(): string {
    return this.value.toString();
  }
}

export class NullObject implements Object {
  public type: ObjectType = "NULL";

  public inspect(): string {
    return "null";
  }
}
