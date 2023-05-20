import { Token, lookupIdent } from "./token";

function isLetter(ch: string): boolean {
  return ("a" <= ch && ch <= "z") || ("A" <= ch && ch <= "Z") || ch === "_";
}

function isDigit(ch: string): boolean {
  return "0" <= ch && ch <= "9";
}

export class Lexer {
  #input: string;
  #position = 0;
  #readPosition = 0;
  #ch = "";

  public constructor(input: string) {
    this.#input = input;
    this.#readChar();
  }

  public nextToken(): Token {
    let token: Token;

    this.#skipWhitespace();

    switch (this.#ch) {
      case "=":
        token = new Token("=", this.#ch);
        break;
      case ";":
        token = new Token(";", this.#ch);
        break;
      case "(":
        token = new Token("(", this.#ch);
        break;
      case ")":
        token = new Token(")", this.#ch);
        break;
      case ",":
        token = new Token(",", this.#ch);
        break;
      case "+":
        token = new Token("+", this.#ch);
        break;
      case "{":
        token = new Token("{", this.#ch);
        break;
      case "}":
        token = new Token("}", this.#ch);
        break;
      case "\0":
        token = new Token("EOF", "");
        break;
      default:
        if (isLetter(this.#ch)) {
          const literal = this.#readIdentifier();
          const type = lookupIdent(literal);
          return new Token(type, literal);
        } else if (isDigit(this.#ch)) {
          const literal = this.#readNumber();
          return new Token("INT", literal);
        } else {
          token = new Token("ILLEGAL", this.#ch);
        }
    }

    this.#readChar();
    return token;
  }

  #readChar(): void {
    if (this.#readPosition >= this.#input.length) {
      this.#ch = "\0";
    } else {
      this.#ch = this.#input[this.#readPosition];
    }

    this.#position = this.#readPosition;
    this.#readPosition += 1;
  }

  #readIdentifier(): string {
    const position = this.#position;
    while (isLetter(this.#ch)) {
      this.#readChar();
    }
    return this.#input.slice(position, this.#position);
  }

  #readNumber(): string {
    const position = this.#position;
    while (isDigit(this.#ch)) {
      this.#readChar();
    }
    return this.#input.slice(position, this.#position);
  }

  #skipWhitespace(): void {
    while (
      this.#ch === " " ||
      this.#ch === "\t" ||
      this.#ch === "\n" ||
      this.#ch === "\r"
    ) {
      this.#readChar();
    }
  }
}
