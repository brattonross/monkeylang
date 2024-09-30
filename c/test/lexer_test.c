#include "lexer_test.h"
#include "../src/lexer.h"
#include "unity.h"

void test_lexer_next_token(void) {
  typedef struct {
    const token_type_t type;
    const char *literal;
  } testcase_t;
  testcase_t expected[] = {
      {TOKEN_LET, "let"},        {TOKEN_IDENTIFIER, "five"},
      {TOKEN_ASSIGN, "="},       {TOKEN_INT, "5"},
      {TOKEN_SEMICOLON, ";"},    {TOKEN_LET, "let"},
      {TOKEN_IDENTIFIER, "ten"}, {TOKEN_ASSIGN, "="},
      {TOKEN_INT, "10"},         {TOKEN_SEMICOLON, ";"},
      {TOKEN_LET, "let"},        {TOKEN_IDENTIFIER, "add"},
      {TOKEN_ASSIGN, "="},       {TOKEN_FUNCTION, "fn"},
      {TOKEN_LEFT_PAREN, "("},   {TOKEN_IDENTIFIER, "x"},
      {TOKEN_COMMA, ","},        {TOKEN_IDENTIFIER, "y"},
      {TOKEN_RIGHT_PAREN, ")"},  {TOKEN_LEFT_BRACE, "{"},
      {TOKEN_IDENTIFIER, "x"},   {TOKEN_PLUS, "+"},
      {TOKEN_IDENTIFIER, "y"},   {TOKEN_SEMICOLON, ";"},
      {TOKEN_RIGHT_BRACE, "}"},  {TOKEN_SEMICOLON, ";"},
      {TOKEN_LET, "let"},        {TOKEN_IDENTIFIER, "result"},
      {TOKEN_ASSIGN, "="},       {TOKEN_IDENTIFIER, "add"},
      {TOKEN_LEFT_PAREN, "("},   {TOKEN_IDENTIFIER, "five"},
      {TOKEN_COMMA, ","},        {TOKEN_IDENTIFIER, "ten"},
      {TOKEN_RIGHT_PAREN, ")"},  {TOKEN_SEMICOLON, ";"},
      {TOKEN_BANG, "!"},         {TOKEN_MINUS, "-"},
      {TOKEN_SLASH, "/"},        {TOKEN_ASTERISK, "*"},
      {TOKEN_INT, "5"},          {TOKEN_SEMICOLON, ";"},
      {TOKEN_INT, "5"},          {TOKEN_LESS_THAN, "<"},
      {TOKEN_INT, "10"},         {TOKEN_GREATER_THAN, ">"},
      {TOKEN_INT, "5"},          {TOKEN_SEMICOLON, ";"},
      {TOKEN_IF, "if"},          {TOKEN_LEFT_PAREN, "("},
      {TOKEN_INT, "5"},          {TOKEN_LESS_THAN, "<"},
      {TOKEN_INT, "10"},         {TOKEN_RIGHT_PAREN, ")"},
      {TOKEN_LEFT_BRACE, "{"},   {TOKEN_RETURN, "return"},
      {TOKEN_TRUE, "true"},      {TOKEN_SEMICOLON, ";"},
      {TOKEN_RIGHT_BRACE, "}"},  {TOKEN_ELSE, "else"},
      {TOKEN_LEFT_BRACE, "{"},   {TOKEN_RETURN, "return"},
      {TOKEN_FALSE, "false"},    {TOKEN_SEMICOLON, ";"},
      {TOKEN_RIGHT_BRACE, "}"},  {TOKEN_INT, "10"},
      {TOKEN_EQUAL, "=="},       {TOKEN_INT, "10"},
      {TOKEN_SEMICOLON, ";"},    {TOKEN_INT, "10"},
      {TOKEN_NOT_EQUAL, "!="},   {TOKEN_INT, "9"},
      {TOKEN_SEMICOLON, ";"},    {TOKEN_STRING, "foobar"},
      {TOKEN_STRING, "foo bar"}, {TOKEN_LEFT_BRACKET, "["},
      {TOKEN_INT, "1"},          {TOKEN_COMMA, ","},
      {TOKEN_INT, "2"},          {TOKEN_RIGHT_BRACKET, "]"},
      {TOKEN_SEMICOLON, ";"},    {TOKEN_LEFT_BRACE, "{"},
      {TOKEN_STRING, "foo"},     {TOKEN_COLON, ":"},
      {TOKEN_STRING, "bar"},     {TOKEN_RIGHT_BRACE, "}"},
      {TOKEN_EOF, ""},
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
                          "\"foobar\"\n"
                          "\"foo bar\"\n"
                          "[1, 2];\n"
                          "{\"foo\": \"bar\"}\n");
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
