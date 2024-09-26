#include "ast.h"
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
    }
  }

  *out = '\0';
  return res;
}

static const char *prefix_expression_fmt = "(%s%s)";
static const char *infix_expression_fmt = "(%s %s %s)";
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
      return NULL;
    }
    snprintf(buf, buf_size + 1, prefix_expression_fmt, e->value.prefix->op,
             right_str);
    free(right_str);
    return buf;
  }
  case EXPRESSION_INFIX: {
    char *left_str = expression_to_string(e->value.infix->left);
    char *right_str = expression_to_string(e->value.infix->right);
    size_t buf_size = snprintf(NULL, 0, infix_expression_fmt, left_str,
                               e->value.infix->op, right_str);
    char *buf = malloc(buf_size + 1);
    if (buf == NULL) {
      free(left_str);
      free(right_str);
      return NULL;
    }
    snprintf(buf, buf_size + 1, infix_expression_fmt, left_str,
             e->value.infix->op, right_str);
    free(left_str);
    free(right_str);
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
      free(consequence_str);
      return NULL;
    }
    snprintf(buf, buf_size + 1, "if %s %s", condition_str, consequence_str);
    free(condition_str);
    free(consequence_str);

    if (e->value.if_->alternative != NULL) {
      char *alternative_str =
          block_statement_to_string(e->value.if_->alternative);
      size_t buf_size = snprintf(NULL, 0, "else %s", alternative_str);
      buf = realloc(buf, strlen(buf) + buf_size);
      if (buf == NULL) {
        free(alternative_str);
        return NULL;
      }
      snprintf(buf, buf_size + 1, "else %s", alternative_str);
      free(alternative_str);
    }

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
  free(p->statements);
  free(p);
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
  }
}

static const char *let_statement_fmt = "let %s = %s;";
char *let_statement_to_string(let_statement_t *s) {
  char *value = expression_token_literal(s->value);
  size_t buf_size = snprintf(NULL, 0, let_statement_fmt, s->name->value, value);
  char *buf = malloc(buf_size + 1);
  if (buf == NULL) {
    free(value);
    return NULL;
  }
  snprintf(buf, buf_size + 1, let_statement_fmt, s->name->value, value);
  free(value);
  return buf;
}

static const char *return_statement_fmt = "return %s;";
char *return_statement_to_string(return_statement_t *s) {
  char *value = expression_token_literal(s->value);
  size_t buf_size = snprintf(NULL, 0, return_statement_fmt, value);
  char *buf = malloc(buf_size + 1);
  if (buf == NULL) {
    free(value);
    return NULL;
  }
  snprintf(buf, buf_size + 1, return_statement_fmt, value);
  free(value);
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
      return NULL;
    }

    size_t len = strlen(str) + strlen(statement_str) + 1;
    char *tmp = realloc(str, len);
    if (tmp == NULL) {
      free(str);
      free(statement_str);
      return NULL;
    }
    str = tmp;

    strcat(str, statement_str);
    free(statement_str);
  }

  return str;
}
