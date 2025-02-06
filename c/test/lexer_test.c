#include "../src/lexer.c"
#include "../src/token.c"
#include <assert.h>
#include <string.h>

void test_token_type(void);
void test_next_token(void);

int main(void) {
  test_token_type();
  test_next_token();
}

void test_token_type(void) {
  char *input = "=+(){},;";
  Lexer l = {0};
  lexer_init(&l, input);

  Token expected[] = {
      (Token){.type = TOKEN_ASSIGN, .literal = String("=")},
      (Token){.type = TOKEN_PLUS, .literal = String("+")},
      (Token){.type = TOKEN_LPAREN, .literal = String("(")},
      (Token){.type = TOKEN_RPAREN, .literal = String(")")},
      (Token){.type = TOKEN_LBRACE, .literal = String("{")},
      (Token){.type = TOKEN_RBRACE, .literal = String("}")},
      (Token){.type = TOKEN_COMMA, .literal = String(",")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_EOF, .literal = String("")},
  };
  for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
    Token t = lexer_next_token(&l);
    assert(expected[i].type == t.type);
    assert(expected[i].literal.length == t.literal.length);
    assert(strncmp(expected[i].literal.buffer, t.literal.buffer,
                   t.literal.length) == 0);
  }
}

void test_next_token(void) {
  char *input = "let five = 5;\n"
                "let ten = 10;\n"
                "\n"
                "let add = fn(x, y) {\n"
                "\tx + y;\n"
                "};\n"
                "\n"
                "let result = add(five, ten);\n"
                "!-/*5;\n"
                "5 < 10 > 5;\n"
                "\n"
                "if (5 < 10) {\n"
                "\treturn true;\n"
                "} else {\n"
                "\treturn false;\n"
                "}\n"
                "\n"
                "10 == 10;\n"
                "10 != 9;\n";
  Lexer l = {0};
  lexer_init(&l, input);

  Token expected[] = {
      (Token){.type = TOKEN_LET, .literal = String("let")},
      (Token){.type = TOKEN_IDENT, .literal = String("five")},
      (Token){.type = TOKEN_ASSIGN, .literal = String("=")},
      (Token){.type = TOKEN_INT, .literal = String("5")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_LET, .literal = String("let")},
      (Token){.type = TOKEN_IDENT, .literal = String("ten")},
      (Token){.type = TOKEN_ASSIGN, .literal = String("=")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_LET, .literal = String("let")},
      (Token){.type = TOKEN_IDENT, .literal = String("add")},
      (Token){.type = TOKEN_ASSIGN, .literal = String("=")},
      (Token){.type = TOKEN_FUNCTION, .literal = String("fn")},
      (Token){.type = TOKEN_LPAREN, .literal = String("(")},
      (Token){.type = TOKEN_IDENT, .literal = String("x")},
      (Token){.type = TOKEN_COMMA, .literal = String(",")},
      (Token){.type = TOKEN_IDENT, .literal = String("y")},
      (Token){.type = TOKEN_RPAREN, .literal = String(")")},
      (Token){.type = TOKEN_LBRACE, .literal = String("{")},
      (Token){.type = TOKEN_IDENT, .literal = String("x")},
      (Token){.type = TOKEN_PLUS, .literal = String("+")},
      (Token){.type = TOKEN_IDENT, .literal = String("y")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_RBRACE, .literal = String("}")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_LET, .literal = String("let")},
      (Token){.type = TOKEN_IDENT, .literal = String("result")},
      (Token){.type = TOKEN_ASSIGN, .literal = String("=")},
      (Token){.type = TOKEN_IDENT, .literal = String("add")},
      (Token){.type = TOKEN_LPAREN, .literal = String("(")},
      (Token){.type = TOKEN_IDENT, .literal = String("five")},
      (Token){.type = TOKEN_COMMA, .literal = String(",")},
      (Token){.type = TOKEN_IDENT, .literal = String("ten")},
      (Token){.type = TOKEN_RPAREN, .literal = String(")")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_BANG, .literal = String("!")},
      (Token){.type = TOKEN_MINUS, .literal = String("-")},
      (Token){.type = TOKEN_SLASH, .literal = String("/")},
      (Token){.type = TOKEN_ASTERISK, .literal = String("*")},
      (Token){.type = TOKEN_INT, .literal = String("5")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_INT, .literal = String("5")},
      (Token){.type = TOKEN_LT, .literal = String("<")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_GT, .literal = String(">")},
      (Token){.type = TOKEN_INT, .literal = String("5")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_IF, .literal = String("if")},
      (Token){.type = TOKEN_LPAREN, .literal = String("(")},
      (Token){.type = TOKEN_INT, .literal = String("5")},
      (Token){.type = TOKEN_LT, .literal = String("<")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_RPAREN, .literal = String(")")},
      (Token){.type = TOKEN_LBRACE, .literal = String("{")},
      (Token){.type = TOKEN_RETURN, .literal = String("return")},
      (Token){.type = TOKEN_TRUE, .literal = String("true")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_RBRACE, .literal = String("}")},
      (Token){.type = TOKEN_ELSE, .literal = String("else")},
      (Token){.type = TOKEN_LBRACE, .literal = String("{")},
      (Token){.type = TOKEN_RETURN, .literal = String("return")},
      (Token){.type = TOKEN_FALSE, .literal = String("false")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_RBRACE, .literal = String("}")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_EQ, .literal = String("==")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_INT, .literal = String("10")},
      (Token){.type = TOKEN_NOT_EQ, .literal = String("!=")},
      (Token){.type = TOKEN_INT, .literal = String("9")},
      (Token){.type = TOKEN_SEMICOLON, .literal = String(";")},
      (Token){.type = TOKEN_EOF, .literal = String("")},
  };
  for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
    Token t = lexer_next_token(&l);
    assert(expected[i].type == t.type);
    assert(expected[i].literal.length == t.literal.length);
    assert(strncmp(expected[i].literal.buffer, t.literal.buffer,
                   t.literal.length) == 0);
  }
}
