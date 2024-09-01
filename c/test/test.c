#include "../src/lexer.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_next_token(void) {
  typedef struct {
    token_type_t type;
    char *literal;
  } testcase_t;
  testcase_t expected[] = {
      {.type = TOKEN_ASSIGN, .literal = "="},
      {.type = TOKEN_PLUS, .literal = "+"},
      {.type = TOKEN_LEFT_PAREN, .literal = "("},
      {.type = TOKEN_RIGHT_PAREN, .literal = ")"},
      {.type = TOKEN_LEFT_BRACE, .literal = "{"},
      {.type = TOKEN_RIGHT_BRACE, .literal = "}"},
      {.type = TOKEN_COMMA, .literal = ","},
      {.type = TOKEN_SEMICOLON, .literal = ";"},
      {.type = TOKEN_EOF, .literal = ""},
  };
  lexer_t *l = new_lexer("=+(){},;");
  int i = 0;
  for (token_t actual = next_token(l); actual.type != EOF;
       actual = next_token(l)) {
    TEST_ASSERT_EQUAL_INT(expected[i].type, actual.type);
    TEST_ASSERT_EQUAL_STRING(expected[i].literal, actual.literal);
    i++;
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_next_token);

  return UNITY_END();
}
