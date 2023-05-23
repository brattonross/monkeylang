import { Token, lookupIdent } from "./token.ts";

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
        if (this.#peekChar() === "=") {
          const ch = this.#ch;
          this.#readChar();
          const literal = ch + this.#ch;
          token = new Token("EQ", literal);
        } else {
          token = new Token("ASSIGN", this.#ch);
        }
        break;
      case "+":
        token = new Token("PLUS", this.#ch);
        break;
      case "-":
        token = new Token("MINUS", this.#ch);
        break;
      case "!":
        if (this.#peekChar() === "=") {
          const ch = this.#ch;
          this.#readChar();
          const literal = ch + this.#ch;
          token = new Token("NOT_EQ", literal);
        } else {
          token = new Token("BANG", this.#ch);
        }
        break;
      case "/":
        token = new Token("SLASH", this.#ch);
        break;
      case "*":
        token = new Token("ASTERISK", this.#ch);
        break;
      case "<":
        token = new Token("LT", this.#ch);
        break;
      case ">":
        token = new Token("GT", this.#ch);
        break;
      case ";":
        token = new Token("SEMICOLON", this.#ch);
        break;
      case "(":
        token = new Token("LPAREN", this.#ch);
        break;
      case ")":
        token = new Token("RPAREN", this.#ch);
        break;
      case ",":
        token = new Token("COMMA", this.#ch);
        break;
      case "{":
        token = new Token("LBRACE", this.#ch);
        break;
      case "}":
        token = new Token("RBRACE", this.#ch);
        break;
      case '"':
        token = new Token("STRING", this.#readString());
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

  #peekChar(): string {
    if (this.#readPosition >= this.#input.length) {
      return "\0";
    } else {
      return this.#input[this.#readPosition]!;
    }
  }

  #readChar(): void {
    if (this.#readPosition >= this.#input.length) {
      this.#ch = "\0";
    } else {
      this.#ch = this.#input[this.#readPosition]!;
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

  #readString(): string {
    const position = this.#position + 1;
    while (true) {
      this.#readChar();
      if (this.#ch === '"' || this.#ch === "\0") {
        break;
      }
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
