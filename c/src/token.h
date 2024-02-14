typedef enum {
  ILLEGAL,
  END_OF_FILE,

  IDENT,
  INT,

  ASSIGN,
  PLUS,

  COMMA,
  SEMICOLON,

  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,

  FUNCTION,
  LET,
} token_type_t;

typedef struct {
  token_type_t token_type;
  char *literal;
} token_t;
