#include "token.h"

typedef struct {
  token_type_t token_type;
  char *value;
} identifier_t;

typedef struct {
  token_type_t token_type;
  identifier_t *name;
} let_statement_t;

typedef struct {
} program_t;
