#include "../src/lexer.h"
#include "unity.h"

void test_lexer_next_token(void) {
  typedef struct {
    const token_type_t type;
    const char *literal;
  } testcase_t;
  testcase_t expected[] = {
      {.type = TOKEN_LET, .literal = "let"},
      {.type = TOKEN_IDENTIFIER, .literal = "five"},
      {.type = TOKEN_ASSIGN, .literal = "="},
      {.type = TOKEN_INT, .literal = "5"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_LET, .literal = "let"},
      {.type = TOKEN_IDENTIFIER, .literal = "ten"},
      {.type = TOKEN_ASSIGN, .literal = "="},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_LET, .literal = "let"},
      {.type = TOKEN_IDENTIFIER, .literal = "add"},
      {.type = TOKEN_ASSIGN, .literal = "="},
      {.type = TOKEN_FUNCTION, .literal = "fn"},
      {.type = TOKEN_LEFT_PAREN, .literal = "("},
      {.type = TOKEN_IDENTIFIER, .literal = "x"},
      {.type = TOKEN_COMMA, .literal = ","},
      {.type = TOKEN_IDENTIFIER, .literal = "y"},
      {.type = TOKEN_RIGHT_PAREN, .literal = ")"},
      {.type = TOKEN_LEFT_BRACE, .literal = "{"},
      {.type = TOKEN_IDENTIFIER, .literal = "x"},
      {.type = TOKEN_PLUS, .literal = "+"},
      {.type = TOKEN_IDENTIFIER, .literal = "y"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_RIGHT_BRACE, .literal = "}"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_LET, .literal = "let"},
      {.type = TOKEN_IDENTIFIER, .literal = "result"},
      {.type = TOKEN_ASSIGN, .literal = "="},
      {.type = TOKEN_IDENTIFIER, .literal = "add"},
      {.type = TOKEN_LEFT_PAREN, .literal = "("},
      {.type = TOKEN_IDENTIFIER, .literal = "five"},
      {.type = TOKEN_COMMA, .literal = ","},
      {.type = TOKEN_IDENTIFIER, .literal = "ten"},
      {.type = TOKEN_RIGHT_PAREN, .literal = ")"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_BANG, .literal = "!"},
      {.type = TOKEN_MINUS, .literal = "-"},
      {.type = TOKEN_SLASH, .literal = "/"},
      {.type = TOKEN_ASTERISK, .literal = "*"},
      {.type = TOKEN_INT, .literal = "5"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_INT, .literal = "5"},
      {.type = TOKEN_LESS_THAN, .literal = "<"},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_GREATER_THAN, .literal = ">"},
      {.type = TOKEN_INT, .literal = "5"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_IF, .literal = "if"},
      {.type = TOKEN_LEFT_PAREN, .literal = "("},
      {.type = TOKEN_INT, .literal = "5"},
      {.type = TOKEN_LESS_THAN, .literal = "<"},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_RIGHT_PAREN, .literal = ")"},
      {.type = TOKEN_LEFT_BRACE, .literal = "{"},
      {.type = TOKEN_RETURN, .literal = "return"},
      {.type = TOKEN_TRUE, .literal = "true"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_RIGHT_BRACE, .literal = "}"},
      {.type = TOKEN_ELSE, .literal = "else"},
      {.type = TOKEN_LEFT_BRACE, .literal = "{"},
      {.type = TOKEN_RETURN, .literal = "return"},
      {.type = TOKEN_FALSE, .literal = "false"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_RIGHT_BRACE, .literal = "}"},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_EQUAL, .literal = "=="},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_INT, .literal = "10"},
      {.type = TOKEN_NOT_EQUAL, .literal = "!="},
      {.type = TOKEN_INT, .literal = "9"},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_EOF, .literal = ""},
  };
  lexer_t *l = lexer_init("let five = 5;\n"
                          "let ten = 10;\n"
                          "\n"
                          "let add = fn(x, y) {\n"
                          "  x + y;\n"
                          "};\n"
                          "\n"
                          "let result = add(five, ten);\n"
                          "!-/*5;\n"
                          "5 < 10 > 5;\n"
                          "\n"
                          "if (5 < 10) {\n"
                          "  return true;\n"
                          "} else {\n"
                          "  return false;\n"
                          "}\n"
                          "\n"
                          "10 == 10;\n"
                          "10 != 9;\n"
                          "");
  TEST_ASSERT_NOT_NULL(l);

  int i = 0;
  for (token_t *actual = lexer_next_token(l); actual->type != TOKEN_EOF;
       actual = lexer_next_token(l)) {
    TEST_ASSERT_NOT_NULL(actual);
    TEST_ASSERT_EQUAL_INT(expected[i].type, actual->type);
    TEST_ASSERT_EQUAL_STRING(expected[i].literal, actual->literal);
    i++;
  }
}
