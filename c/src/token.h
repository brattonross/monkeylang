typedef enum {
  END_OF_FILE = 0,
  ILLEGAL,

  IDENT,
  INT,

  ASSIGN,
  PLUS,
  MINUS,
  BANG,
  ASTERISK,
  SLASH,

  LT,
  GT,

  EQ,
  NOT_EQ,

  COMMA,
  SEMICOLON,

  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,

  FUNCTION,
  LET,
  TRUE,
  FALSE,
  IF,
  ELSE,
  RETURN,
} token_type_t;

typedef struct {
  token_type_t token_type;
  char *literal;
} token_t;
