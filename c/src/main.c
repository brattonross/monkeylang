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
                         "let result = add(five, ten);");
  while (l->ch != 0) {
    token_t token = lexer_next_token(l);
    printf("%s\n", token.literal);
  }
  return 0;
}
