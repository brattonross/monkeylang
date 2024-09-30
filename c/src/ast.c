#include "ast.h"
#include "token.h"
#include <stdio.h>
#include <string.h>

char *statement_to_string(statement_t *s);
char *block_statement_to_string(block_statement_t *s) {
  if (s == NULL || s->statements_len == 0 || s->statements == NULL) {
    return strdup("");
  }

  size_t total_len = 1;
  for (size_t i = 0; i < s->statements_len; ++i) {
    char *tmp = statement_to_string(s->statements[i]);
    if (tmp != NULL) {
      total_len += strlen(tmp);
      free(tmp);
      tmp = NULL;
    }
  }

  char *res = malloc(total_len);
  char *out = res;
  *out = '\0';

  for (size_t i = 0; i < s->statements_len; ++i) {
    char *tmp = statement_to_string(s->statements[i]);
    if (tmp != NULL) {
      size_t len = strlen(tmp);
      memcpy(out, tmp, len);
      out += len;
      free(tmp);
      tmp = NULL;
    }
  }

  *out = '\0';
  return res;
}

char *identifier_to_string(identifier_t *ident) { return strdup(ident->value); }

static const char *prefix_expression_fmt = "(%s%s)";
char *expression_to_string(expression_t *e) {
  switch (e->type) {
  case EXPRESSION_IDENTIFIER:
    return strdup(e->value.ident->value);
  case EXPRESSION_INTEGER_LITERAL:
    return strdup(e->value.integer->token->literal);
  case EXPRESSION_PREFIX: {
    char *right_str = expression_to_string(e->value.prefix->right);
    size_t buf_size = snprintf(NULL, 0, prefix_expression_fmt,
                               e->value.prefix->op, right_str);
    char *buf = malloc(buf_size + 1);
    if (buf == NULL) {
      free(right_str);
      right_str = NULL;
      return NULL;
    }
    snprintf(buf, buf_size + 1, prefix_expression_fmt, e->value.prefix->op,
             right_str);
    free(right_str);
    right_str = NULL;
    return buf;
  }
  case EXPRESSION_INFIX: {
    size_t base_len = 5; // (  )
    char *left_str = expression_to_string(e->value.infix->left);
    size_t left_str_len = strlen(left_str);
    char *op = e->value.infix->op;
    size_t op_len = strlen(op);
    char *right_str = expression_to_string(e->value.infix->right);
    size_t right_str_len = strlen(right_str);

    size_t total_len = base_len + left_str_len + op_len + right_str_len;
    char *buf = malloc(total_len);
    char *out = buf;
    *out = '\0';

    memcpy(out, "(", 1);
    out++;

    memcpy(out, left_str, left_str_len);
    out += left_str_len;

    memcpy(out, " ", 1);
    out++;

    memcpy(out, op, op_len);
    out += op_len;

    memcpy(out, " ", 1);
    out++;

    memcpy(out, right_str, right_str_len);
    out += right_str_len;

    memcpy(out, ")", 1);
    out++;

    *out = '\0';

    return buf;
  }
  case EXPRESSION_BOOLEAN_LITERAL: {
    return strdup(e->value.boolean->token->literal);
  }
  case EXPRESSION_IF: {
    char *condition_str = expression_to_string(e->value.if_->condition);
    char *consequence_str =
        block_statement_to_string(e->value.if_->consequence);
    size_t buf_size =
        snprintf(NULL, 0, "if %s %s", condition_str, consequence_str);
    char *buf = malloc(buf_size + 1);
    if (buf == NULL) {
      free(condition_str);
      condition_str = NULL;
      free(consequence_str);
      consequence_str = NULL;
      return NULL;
    }
    snprintf(buf, buf_size + 1, "if %s %s", condition_str, consequence_str);
    free(condition_str);
    condition_str = NULL;
    free(consequence_str);
    consequence_str = NULL;

    if (e->value.if_->alternative != NULL) {
      char *alternative_str =
          block_statement_to_string(e->value.if_->alternative);
      size_t buf_size = snprintf(NULL, 0, "else %s", alternative_str);
      buf = realloc(buf, strlen(buf) + buf_size);
      if (buf == NULL) {
        free(alternative_str);
        alternative_str = NULL;
        return NULL;
      }
      snprintf(buf, buf_size + 1, "else %s", alternative_str);
      free(alternative_str);
      alternative_str = NULL;
    }

    return buf;
  }
  case EXPRESSION_FUNCTION_LITERAL: {
    function_literal_t *fn = e->value.fn;
    size_t total_len = 1;

    total_len += 3; // "fn("

    for (size_t i = 0; i < fn->parameters_len; ++i) {
      char *tmp = identifier_to_string(fn->parameters[i]);
      if (tmp != NULL) {
        if (total_len > 1) {
          total_len += 2; // ", "
        }
        total_len += strlen(tmp);
        free(tmp);
        tmp = NULL;
      }
    }
    total_len += 2; // ") "
    char *body = block_statement_to_string(fn->body);
    size_t body_len = strlen(body);
    total_len += body_len;

    char *res = malloc(total_len);
    char *out = res;
    *out = '\0';

    memcpy(out, "fn(", 3);
    out += 3;

    size_t params_count = 0;
    for (size_t i = 0; i < fn->parameters_len; ++i) {
      char *tmp = identifier_to_string(fn->parameters[i]);
      if (tmp != NULL) {
        if (params_count > 0) {
          memcpy(out, ", ", 2);
          out += 2;
        }
        size_t len = strlen(tmp);
        memcpy(out, tmp, len);
        out += len;
        free(tmp);
        tmp = NULL;
        params_count++;
      }
    }

    memcpy(out, ") ", 2);
    out += 2;

    memcpy(out, body, body_len);
    out += body_len;

    *out = '\0';
    return res;
  }
  case EXPRESSION_CALL: {
    call_expression_t *call = e->value.call;
    size_t total_len = 1;

    size_t fn_ident_len = strlen(e->value.call->fn->value.ident->value);
    total_len += fn_ident_len;

    total_len += 1; // "("
    for (size_t i = 0; i < call->argc; ++i) {
      char *tmp = expression_to_string(call->argv[i]);
      if (tmp != NULL) {
        if (total_len > 1) {
          total_len += 2; // ", "
        }
        total_len += strlen(tmp);
        free(tmp);
        tmp = NULL;
      }
    }
    total_len += 1; // ")"

    char *res = malloc(total_len);
    char *out = res;
    *out = '\0';

    memcpy(out, e->value.call->fn->value.ident->value, fn_ident_len);
    out += fn_ident_len;

    memcpy(out, "(", 1);
    out += 1;
    size_t params_count = 0;
    for (size_t i = 0; i < call->argc; ++i) {
      char *tmp = expression_to_string(call->argv[i]);
      if (tmp != NULL) {
        if (params_count > 0) {
          memcpy(out, ", ", 2);
          out += 2;
        }
        size_t len = strlen(tmp);
        memcpy(out, tmp, len);
        out += len;
        free(tmp);
        tmp = NULL;
        params_count++;
      }
    }
    memcpy(out, ")", 1);
    out += 1;
    *out = '\0';
    return res;
  }
  case EXPRESSION_STRING:
    return strdup(e->value.string->token->literal);
  case EXPRESSION_ARRAY_LITERAL: {
    array_literal_t *arr = e->value.arr;
    size_t total_len = 3; // []
    char *elements[arr->len];
    for (size_t i = 0; i < arr->len; ++i) {
      if (total_len > 3) {
        total_len += 2; // ", "
      }
      elements[i] = expression_to_string(arr->elements[i]);
      total_len = strlen(elements[i]);
    }

    char *res = malloc(total_len);
    char *out = res;
    *out = '\0';

    memcpy(out, "[", 1);
    out++;

    for (size_t i = 0; i < arr->len; ++i) {
      if (i > 0) {
        memcpy(out, ", ", 2);
        out += 2;
      }
      size_t len = strlen(elements[i]);
      memcpy(out, elements[i], len);
      out += len;
    }

    memcpy(out, "]", 1);
    out++;

    *out = '\0';
    return res;
  }
  case EXPRESSION_INDEX: {
    index_expression_t *index = e->value.index;
    size_t base_len = 4; // ([])
    char *left_str = expression_to_string(index->left);
    size_t left_str_len = strlen(left_str);
    char *index_str = expression_to_string(index->index);
    size_t index_str_len = strlen(index_str);

    size_t total_len = base_len + left_str_len + index_str_len + 1;
    char *buf = malloc(total_len + 1);
    if (buf == NULL) {
      free(left_str);
      free(index_str);
      return NULL;
    }
    snprintf(buf, total_len, "(%s[%s])", left_str, index_str);
    return buf;
  }
  case EXPRESSION_HASH: {
    hash_literal_t *hash = e->value.hash;
    size_t total_len = 3; // {}
    char *keys[hash->len];
    char *values[hash->len];
    for (size_t i = 0; i < hash->len; ++i) {
      if (i > 0) {
        total_len += 2; // ", "
      }
      keys[i] = expression_to_string(hash->pairs[i]->key);
      total_len += strlen(keys[i]);
      total_len += 1; // ":"
      values[i] = expression_to_string(hash->pairs[i]->value);
      total_len += strlen(keys[i]);
    }

    char *buf = malloc(total_len);
    char *out = buf;
    *out = '\0';

    memcpy(out, "{", 1);
    out++;

    for (size_t i = 0; i < hash->len; ++i) {
      if (i > 0) {
        memcpy(out, ", ", 2);
        out += 2;
      }

      memcpy(out, keys[i], strlen(keys[i]));
      out += strlen(keys[i]);

      memcpy(out, ":", 1);
      out++;

      memcpy(out, values[i], strlen(values[i]));
      out += strlen(values[i]);
    }

    memcpy(out, "}", 1);
    out++;

    *out = '\0';
    return buf;
  }
  }
}

const char *statement_token_literal(const statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    return strdup(s->value.let->token->literal);
  case STATEMENT_RETURN:
    return strdup(s->value.ret->token->literal);
  case STATEMENT_EXPRESSION:
    return strdup(s->value.exp->token->literal);
  case STATEMENT_BLOCK:
    return strdup(s->value.block->token->literal);
  }
}

const char *identifier_token_literal(identifier_t *i) {
  return strdup(i->token->literal);
}

void program_free(program_t *p) {
  for (size_t i = 0; i < p->statements_len; ++i) {
    statement_free(p->statements[i]);
    p->statements[i] = NULL;
  }
  free(p->statements);
  p->statements = NULL;
  free(p);
  p = NULL;
}

char *program_token_literal(const program_t *p) {
  if (p->statements_len > 0) {
    return strdup(statement_token_literal(p->statements[0]));
  }
  return NULL;
}

char *expression_token_literal(const expression_t *e) {
  if (e == NULL) {
    return NULL;
  }
  switch (e->type) {
  case EXPRESSION_IDENTIFIER:
    return strdup(e->value.ident->token->literal);
  case EXPRESSION_INTEGER_LITERAL:
    return strdup(e->value.integer->token->literal);
  case EXPRESSION_PREFIX:
    return strdup(e->value.prefix->token->literal);
  case EXPRESSION_INFIX:
    return strdup(e->value.infix->token->literal);
  case EXPRESSION_BOOLEAN_LITERAL:
    return strdup(e->value.boolean->token->literal);
  case EXPRESSION_IF:
    return strdup(e->value.if_->token->literal);
  case EXPRESSION_FUNCTION_LITERAL:
    return strdup(e->value.fn->token->literal);
  case EXPRESSION_CALL:
    return strdup(e->value.call->token->literal);
  case EXPRESSION_STRING:
    return strdup(e->value.string->token->literal);
  case EXPRESSION_ARRAY_LITERAL:
    return strdup(e->value.arr->token->literal);
  case EXPRESSION_INDEX:
    return strdup(e->value.index->token->literal);
  case EXPRESSION_HASH:
    return strdup(e->value.hash->token->literal);
  }
}

static const char *let_statement_fmt = "let %s = %s;";
char *let_statement_to_string(let_statement_t *s) {
  char *value = expression_to_string(s->value);
  size_t buf_size =
      snprintf(NULL, 0, let_statement_fmt, s->ident->value, value);
  char *buf = malloc(buf_size + 1);
  if (buf == NULL) {
    free(value);
    value = NULL;
    return NULL;
  }
  snprintf(buf, buf_size + 1, let_statement_fmt, s->ident->value, value);
  free(value);
  value = NULL;
  return buf;
}

static const char *return_statement_fmt = "return %s;";
char *return_statement_to_string(return_statement_t *s) {
  char *value = expression_to_string(s->value);
  size_t buf_size = snprintf(NULL, 0, return_statement_fmt, value);
  char *buf = malloc(buf_size + 1);
  if (buf == NULL) {
    free(value);
    value = NULL;
    return NULL;
  }
  snprintf(buf, buf_size + 1, return_statement_fmt, value);
  free(value);
  value = NULL;
  return buf;
}

char *expression_statement_to_string(expression_statement_t *s) {
  return expression_to_string(s->expression);
}

char *statement_to_string(statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    return let_statement_to_string(s->value.let);
  case STATEMENT_RETURN:
    return return_statement_to_string(s->value.ret);
  case STATEMENT_EXPRESSION:
    return expression_statement_to_string(s->value.exp);
  case STATEMENT_BLOCK:
    return block_statement_to_string(s->value.block);
  }
}

char *program_to_string(const program_t *p) {
  char *str = malloc(1);
  if (str == NULL) {
    return NULL;
  }
  str[0] = '\0';

  for (size_t i = 0; i < p->statements_len; ++i) {
    statement_t *s = p->statements[i];
    char *statement_str = statement_to_string(s);
    if (statement_str == NULL) {
      free(str);
      str = NULL;
      return NULL;
    }

    size_t len = strlen(str) + strlen(statement_str) + 1;
    char *tmp = realloc(str, len);
    if (tmp == NULL) {
      free(str);
      str = NULL;
      free(statement_str);
      statement_str = NULL;
      return NULL;
    }
    str = tmp;

    strcat(str, statement_str);
    free(statement_str);
    statement_str = NULL;
  }

  return str;
}

void identifier_free(identifier_t *ident) {
  token_free(ident->token);
  free(ident->value);
  ident->value = NULL;
}

void statement_free(statement_t *s) {
  switch (s->type) {
  case STATEMENT_LET:
    let_statement_free(s->value.let);
    break;
  case STATEMENT_BLOCK:
    block_statement_free(s->value.block);
    break;
  case STATEMENT_RETURN:
    return_statement_free(s->value.ret);
    break;
  case STATEMENT_EXPRESSION:
    expression_statement_free(s->value.exp);
    break;
  }

  free(s);
  s = NULL;
}

void expression_statement_free(expression_statement_t *exp) {
  token_free(exp->token);
  expression_free(exp->expression);
  free(exp);
  exp = NULL;
}

void return_statement_free(return_statement_t *ret) {
  token_free(ret->token);
  expression_free(ret->value);
  free(ret);
  ret = NULL;
}

void let_statement_free(let_statement_t *let) {
  token_free(let->token);
  expression_free(let->value);
  identifier_free(let->ident);
  free(let);
  let = NULL;
}

void block_statement_free(block_statement_t *block) {
  token_free(block->token);
  for (size_t i = 0; i < block->statements_len; ++i) {
    statement_free(block->statements[i]);
  }
  free(block->statements);
  block->statements = NULL;
}

void if_expression_free(if_expression_t *exp) {
  token_free(exp->token);
  expression_free(exp->condition);
  block_statement_free(exp->alternative);
  free(exp);
  exp = NULL;
}

void call_expression_free(call_expression_t *exp) {
  token_free(exp->token);
  expression_free(exp->fn);
  for (size_t i = 0; i < exp->argc; ++i) {
    expression_free(exp->argv[i]);
  }
  free(exp->argv);
  exp->argv = NULL;
  free(exp);
  exp = NULL;
}

void infix_expression_free(infix_expression_t *exp) {
  token_free(exp->token);
  free(exp->op);
  exp->op = NULL;
  expression_free(exp->left);
  expression_free(exp->right);
  free(exp);
  exp = NULL;
}

void prefix_expression_free(prefix_expression_t *exp) {
  token_free(exp->token);
  expression_free(exp->right);
  free(exp->op);
  exp->op = NULL;
  free(exp);
  exp = NULL;
}

void boolean_literal_free(boolean_literal_t *exp) {
  token_free(exp->token);
  free(exp);
  exp = NULL;
}

void integer_literal_free(integer_literal_t *exp) {
  token_free(exp->token);
  free(exp);
  exp = NULL;
}

void function_literal_free(function_literal_t *exp) {
  token_free(exp->token);
  block_statement_free(exp->body);
  for (size_t i = 0; i < exp->parameters_len; ++i) {
    identifier_free(exp->parameters[i]);
  }
  free(exp->parameters);
  exp->parameters = NULL;
  free(exp);
  exp = NULL;
}

void string_literal_free(string_literal_t *exp) {
  token_free(exp->token);
  free(exp->value);
  exp->value = NULL;
  free(exp);
  exp = NULL;
}

void array_literal_free(array_literal_t *exp) {
  token_free(exp->token);
  for (size_t i = 0; i < exp->len; ++i) {
    expression_free(exp->elements[i]);
  }
  free(exp);
  exp = NULL;
}

void index_expression_free(index_expression_t *exp) {
  token_free(exp->token);
  expression_free(exp->left);
  expression_free(exp->index);
  free(exp);
  exp = NULL;
}

void hash_literal_free(hash_literal_t *exp) {
  token_free(exp->token);
  for (size_t i = 0; i < exp->len; ++i) {
    expression_free(exp->pairs[i]->key);
    expression_free(exp->pairs[i]->value);
    free(exp->pairs[i]);
    exp->pairs[i] = NULL;
  }
  free(exp);
  exp = NULL;
}

void expression_free(expression_t *exp) {
  switch (exp->type) {
  case EXPRESSION_IDENTIFIER:
    identifier_free(exp->value.ident);
    break;
  case EXPRESSION_IF:
    if_expression_free(exp->value.if_);
    break;
  case EXPRESSION_CALL:
    call_expression_free(exp->value.call);
    break;
  case EXPRESSION_INFIX:
    infix_expression_free(exp->value.infix);
    break;
  case EXPRESSION_PREFIX:
    prefix_expression_free(exp->value.prefix);
    break;
  case EXPRESSION_BOOLEAN_LITERAL:
    boolean_literal_free(exp->value.boolean);
    break;
  case EXPRESSION_INTEGER_LITERAL:
    integer_literal_free(exp->value.integer);
    break;
  case EXPRESSION_FUNCTION_LITERAL:
    function_literal_free(exp->value.fn);
    break;
  case EXPRESSION_STRING:
    string_literal_free(exp->value.string);
    break;
  case EXPRESSION_ARRAY_LITERAL:
    array_literal_free(exp->value.arr);
    break;
  case EXPRESSION_INDEX:
    index_expression_free(exp->value.index);
    break;
  case EXPRESSION_HASH:
    hash_literal_free(exp->value.hash);
    break;
  }

  free(exp);
  exp = NULL;
}
