import type { BlockStatement, IfExpression, NodeType, Program } from "./ast.ts";
import {
  BooleanObject,
  IntegerObject,
  NullObject,
  ReturnValueObject,
  type ObjectType,
  ErrorObject,
} from "./object.ts";

export const NULL = new NullObject();
export const TRUE = new BooleanObject(true);
export const FALSE = new BooleanObject(false);

export function evaluate(node: NodeType | null): ObjectType | null {
  if (node === null) {
    return null;
  }

  switch (node.type) {
    case "PROGRAM":
      return evalProgram(node);

    case "EXPRESSION_STATEMENT":
      return evaluate(node.expression);

    case "INTEGER_EXPRESSION":
      return new IntegerObject(node.value);

    case "BOOLEAN_EXPRESSION":
      return node.value ? TRUE : FALSE;

    case "PREFIX_EXPRESSION": {
      const right = evaluate(node.right);
      if (isError(right)) {
        return right;
      }
      return evalPrefixExpression(node.operator, right);
    }

    case "INFIX_EXPRESSION": {
      const left = evaluate(node.left);
      if (isError(left)) {
        return left;
      }
      const right = evaluate(node.right);
      if (isError(right)) {
        return right;
      }
      return evalInfixExpression(node.operator, left, right);
    }

    case "BLOCK_STATEMENT":
      return evalBlockStatement(node);

    case "IF_EXPRESSION":
      return evalIfExpression(node);

    case "RETURN_STATEMENT": {
      const value = evaluate(node.returnValue);
      if (isError(value)) {
        return value;
      }
      return new ReturnValueObject(value!);
    }

    default:
      return null;
  }
}

function isError(obj: ObjectType | null): obj is ErrorObject {
  return obj?.type === "ERROR";
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
      return new ErrorObject(`unknown operator: ${operator}${right?.type}`);
  }
}

function evalMinusPrefixOperatorExpression(
  right: ObjectType | null
): ObjectType {
  if (right?.type !== "INTEGER") {
    return new ErrorObject(`unknown operator: -${right?.type}`);
  }

  return new IntegerObject(-right.value);
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
  } else if (left?.type !== right?.type) {
    return new ErrorObject(
      `type mismatch: ${left?.type} ${operator} ${right?.type}`
    );
  }

  return new ErrorObject(
    `unknown operator: ${left?.type} ${operator} ${right?.type}`
  );
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
      return new ErrorObject(
        `unknown operator: ${left.type} ${operator} ${right.type}`
      );
  }
}

function evalIfExpression(node: IfExpression): ObjectType | null {
  const condition = evaluate(node.condition);
  if (isError(condition)) {
    return condition;
  } else if (isTruthy(condition)) {
    return evaluate(node.consequence);
  } else if (node.alternative) {
    return evaluate(node.alternative);
  } else {
    return NULL;
  }
}

function isTruthy(obj: ObjectType | null): boolean {
  switch (obj) {
    case NULL:
      return false;

    case TRUE:
      return true;

    case FALSE:
      return false;

    default:
      return true;
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

function evalProgram(program: Program): ObjectType | null {
  let result: ObjectType | null = null;
  for (let i = 0; i < program.statements.length; i++) {
    result = evaluate(program.statements[i]!);

    if (result?.type === "RETURN_VALUE") {
      result = result.value;
      break;
    } else if (result?.type === "ERROR") {
      break;
    }
  }
  return result;
}

function evalBlockStatement(block: BlockStatement): ObjectType | null {
  let result: ObjectType | null = null;
  for (let i = 0; i < block.statements.length; i++) {
    result = evaluate(block.statements[i]!);

    if (result === null) {
      continue;
    }

    if (result.type === "RETURN_VALUE" || result.type === "ERROR") {
      break;
    }
  }
  return result;
}
