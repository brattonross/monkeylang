#include "lexer.h"
#include <stdio.h>

int main() {
  lexer_t *l = lexer_new("let five = 5;"
                         "let ten = 10;"
                         ""
                         "let add = fn(x, y) {"
                         "  x + y;"
                         "};"
                         ""
                         "let result = add(five, ten);"
                         "!-/*5;"
                         "5 < 10 > 5;"
                         ""
                         "if (5 < 10) {"
                         "  return true;"
                         "} else {"
                         "  return false;"
                         "}"
                         ""
                         "10 == 10;"
                         "10 != 9;");
  while (l->ch != 0) {
    token_t token = lexer_next_token(l);
    printf("%s\n", token.literal);
  }
  return 0;
}
