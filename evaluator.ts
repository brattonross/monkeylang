import type { Expression, Program, Statement } from "./ast.ts";
import { IntegerObject, type Object } from "./object.ts";

export function evaluate(
  node: Expression | Statement | Program | null
): Object | null {
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

    default:
      return null;
  }
}

function evalStatements(statements: Array<Statement>): Object | null {
  if (!statements[0]) {
    return null;
  }

  let result = evaluate(statements[0]);
  for (let i = 1; i < statements.length; i++) {
    result = evaluate(statements[i]!);
  }
  return result;
}
