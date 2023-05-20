import { createInterface } from "node:readline";
import { Lexer } from "./lexer.ts";

const repl = createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: ">> ",
});

repl.on("line", (line) => {
  const lexer = new Lexer(line);

  for (
    let token = lexer.nextToken();
    token.type !== "EOF";
    token = lexer.nextToken()
  ) {
    console.log(token);
  }

  repl.prompt();
});

repl.prompt();
