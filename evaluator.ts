import type { NodeType, Statement } from "./ast.ts";
import {
  BooleanObject,
  IntegerObject,
  NullObject,
  type ObjectType,
} from "./object.ts";

const NULL = new NullObject();
const TRUE = new BooleanObject(true);
const FALSE = new BooleanObject(false);

export function evaluate(node: NodeType | null): ObjectType | null {
  if (node === null) {
    return null;
  }

  switch (node.type) {
    case "PROGRAM":
      return evalStatements(node.statements);

    case "EXPRESSION_STATEMENT":
      return evaluate(node.expression);

    case "INTEGER_EXPRESSION":
      return new IntegerObject(node.value);

    case "BOOLEAN_EXPRESSION":
      return node.value ? TRUE : FALSE;

    case "PREFIX_EXPRESSION": {
      const right = evaluate(node.right);
      return evalPrefixExpression(node.operator, right);
    }

    case "INFIX_EXPRESSION": {
      const left = evaluate(node.left);
      const right = evaluate(node.right);
      return evalInfixExpression(node.operator, left, right);
    }

    default:
      return null;
  }
}

function evalPrefixExpression(
  operator: string,
  right: ObjectType | null
): ObjectType {
  switch (operator) {
    case "!":
      return evalBangOperatorExpression(right);

    case "-":
      return evalMinusPrefixOperatorExpression(right);

    default:
      return NULL;
  }
}

function evalInfixExpression(
  operator: string,
  left: ObjectType | null,
  right: ObjectType | null
): ObjectType {
  if (left?.type === "INTEGER" && right?.type === "INTEGER") {
    return evalIntegerInfixExpression(operator, left, right);
  } else if (operator === "==") {
    return left === right ? TRUE : FALSE;
  } else if (operator === "!=") {
    return left !== right ? TRUE : FALSE;
  }

  return NULL;
}

function evalIntegerInfixExpression(
  operator: string,
  left: IntegerObject,
  right: IntegerObject
): ObjectType {
  const leftValue = left.value;
  const rightValue = right.value;

  switch (operator) {
    case "+":
      return new IntegerObject(leftValue + rightValue);

    case "-":
      return new IntegerObject(leftValue - rightValue);

    case "*":
      return new IntegerObject(leftValue * rightValue);

    case "/":
      return new IntegerObject(Math.floor(leftValue / rightValue));

    case "<":
      return leftValue < rightValue ? TRUE : FALSE;

    case ">":
      return leftValue > rightValue ? TRUE : FALSE;

    case "==":
      return leftValue === rightValue ? TRUE : FALSE;

    case "!=":
      return leftValue !== rightValue ? TRUE : FALSE;

    default:
      return NULL;
  }
}

function evalBangOperatorExpression(right: ObjectType | null): ObjectType {
  switch (right) {
    case TRUE:
      return FALSE;

    case FALSE:
      return TRUE;

    case NULL:
      return TRUE;

    default:
      return FALSE;
  }
}

function evalMinusPrefixOperatorExpression(
  right: ObjectType | null
): ObjectType {
  if (right?.type !== "INTEGER") {
    return NULL;
  }

  return new IntegerObject(-right.value);
}

function evalStatements(statements: Array<Statement>): ObjectType | null {
  if (!statements[0]) {
    return null;
  }

  let result = evaluate(statements[0]);
  for (let i = 1; i < statements.length; i++) {
    result = evaluate(statements[i]!);
  }
  return result;
}
