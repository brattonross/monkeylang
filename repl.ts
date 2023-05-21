import { createInterface } from "node:readline";
import { Lexer } from "./lexer.ts";
import { Parser } from "./parser.ts";

const repl = createInterface({
  input: process.stdin,
  output: process.stdout,
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
    console.log(program.toString());
  }

  repl.prompt();
});

repl.prompt();
