import * as console from "node:console";
import { stdin, stdout } from "node:process";
import { createInterface } from "node:readline";
import { evaluate } from "./evaluator.ts";
import { Lexer } from "./lexer.ts";
import { Parser } from "./parser.ts";

const repl = createInterface({
  input: stdin,
  output: stdout,
  prompt: ">> ",
});

repl.on("line", (line) => {
  const lexer = new Lexer(line);
  const parser = new Parser(lexer);

  const program = parser.parseProgram();
  if (parser.errors.length !== 0) {
    for (let i = 0; i < parser.errors.length; i++) {
      console.error(parser.errors[i]);
    }
  } else {
    const evaluated = evaluate(program);
    if (evaluated !== null) {
      console.log(evaluated.inspect());
    }
  }

  repl.prompt();
});

repl.on("close", () => {
  repl.close();
});

repl.prompt();
