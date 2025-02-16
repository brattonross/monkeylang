#pragma once

#include "mem.c"
#include "token.c"
#include <stddef.h>
#include <stdint.h>

typedef struct Identifier {
  Token token;
  String value;
} Identifier;

typedef struct Expression Expression;

typedef enum ExpressionType {
  EXPRESSION_IDENTIFIER,
  EXPRESSION_INTEGER,
  EXPRESSION_PREFIX,
  EXPRESSION_INFIX,
} ExpressionType;

typedef struct IntegerLiteral {
  Token token;
  int64_t value;
} IntegerLiteral;

typedef struct PrefixExpression {
  Token token;
  String op;
  Expression *right;
} PrefixExpression;

typedef struct InfixExpression {
  Token token;
  Expression *left;
  String op;
  Expression *right;
} InfixExpression;

typedef union ExpressionData {
  Identifier identifier;
  IntegerLiteral integer;
  PrefixExpression prefix;
  InfixExpression infix;
} ExpressionData;

struct Expression {
  ExpressionType type;
  ExpressionData data;
};

String expression_to_string(const Expression *expression, Arena *arena) {
  switch (expression->type) {
  case EXPRESSION_IDENTIFIER:
    return expression->data.identifier.value;
  case EXPRESSION_INTEGER:
    return string_fmt(arena, "%lld", expression->data.integer.value);
  case EXPRESSION_PREFIX: {
    String right_str =
        expression_to_string(expression->data.prefix.right, arena);
    return string_fmt(arena, "(%.*s%.*s)", expression->data.prefix.op.length,
                      expression->data.prefix.op.buffer, right_str.length,
                      right_str.buffer);
  }
  case EXPRESSION_INFIX: {
    String left_str = expression_to_string(expression->data.infix.right, arena);
    String right_str =
        expression_to_string(expression->data.infix.right, arena);
    return string_fmt(arena, "(%.*s %.*s %.*s", left_str.length,
                      left_str.buffer, expression->data.prefix.op.length,
                      expression->data.prefix.op.buffer, right_str.length,
                      right_str.buffer);
  }
  }
}

typedef struct LetStatement {
  Token token;
  Identifier *name;
  Expression *value;
} LetStatement;

String let_statement_to_string(LetStatement let, Arena *arena) {
  StringBuilder sb = string_builder_create(arena);
  string_builder_append(
      &sb, string_fmt(arena, "%.*s %.*s = ", let.token.literal.length,
                      let.token.literal.buffer, let.name->value.length,
                      let.name->value.buffer));
  if (let.value) {
    string_builder_append(&sb, expression_to_string(let.value, arena));
  }
  string_builder_append(&sb, String(";"));
  return string_builder_build(&sb);
}

typedef struct ReturnStatement {
  Token token;
  Expression *return_value;
} ReturnStatement;

String return_statement_to_string(ReturnStatement ret, Arena *arena) {
  StringBuilder sb = string_builder_create(arena);
  string_builder_append(&sb,
                        string_fmt(arena, "%.*s ", ret.token.literal.length,
                                   ret.token.literal.buffer));
  if (ret.return_value) {
    string_builder_append(&sb, expression_to_string(ret.return_value, arena));
  }
  string_builder_append(&sb, String(";"));
  return string_builder_build(&sb);
}

typedef struct ExpressionStatement {
  Token token;
  Expression *expression;
} ExpressionStatement;

String expression_statement_to_string(ExpressionStatement exp, Arena *arena) {
  if (exp.expression) {
    return expression_to_string(exp.expression, arena);
  }
  return String("");
}

typedef enum StatementType {
  STATEMENT_LET,
  STATEMENT_RETURN,
  STATEMENT_EXPRESSION,
} StatementType;

typedef union StatementData {
  LetStatement let_statement;
  ReturnStatement return_statement;
  ExpressionStatement expression_statement;
} StatementData;

typedef struct Statement {
  StatementType type;
  StatementData data;
} Statement;

String statement_token_literal(const Statement s) {
  switch (s.type) {
  case STATEMENT_LET:
    return s.data.let_statement.token.literal;
  case STATEMENT_RETURN:
    return s.data.return_statement.token.literal;
  case STATEMENT_EXPRESSION:
    return s.data.expression_statement.token.literal;
  }
}

String statement_to_string(const Statement *s, Arena *arena) {
  switch (s->type) {
  case STATEMENT_LET:
    return let_statement_to_string(s->data.let_statement, arena);
  case STATEMENT_RETURN:
    return return_statement_to_string(s->data.return_statement, arena);
  case STATEMENT_EXPRESSION:
    return expression_statement_to_string(s->data.expression_statement, arena);
  }
}

#define STATEMENT_CHUNK_SIZE 64

typedef struct StatementChunk StatementChunk;
struct StatementChunk {
  Statement statements[STATEMENT_CHUNK_SIZE];
  StatementChunk *next;
  size_t used;
};

typedef struct Program {
  StatementChunk *first_chunk;
  StatementChunk *current_chunk;
  size_t statements_len;
} Program;

/**
 * Allocates a new instance of `Program` in the given arena.
 * We do this since the program and all associated statements probably want to
 * be using the same lifetime.
 */
Program *program_create(Arena *arena) {
  Program *program = arena_alloc(arena, sizeof(Program));
  StatementChunk *chunk = arena_alloc(arena, sizeof(StatementChunk));

  chunk->next = NULL;
  chunk->used = 0;

  program->first_chunk = chunk;
  program->current_chunk = chunk;
  program->statements_len = 0;

  return program;
}

Statement *program_statement_at(const Program *program, size_t index) {
  StatementChunk *chunk = program->first_chunk;
  size_t start = 0, end = 0;
  while (chunk) {
    start = end;
    end += chunk->used;
    if (end >= index + 1) {
      break;
    }
    chunk = chunk->next;
  }
  if (!chunk) {
    return NULL;
  }
  return &chunk->statements[index - start];
}

void program_append_statement(Program *program, Arena *arena,
                              Statement statement) {
  if (program->current_chunk->used == STATEMENT_CHUNK_SIZE) {
    StatementChunk *new_chunk = arena_alloc(arena, sizeof(StatementChunk));
    new_chunk->next = NULL;
    new_chunk->used = 0;

    program->current_chunk->next = new_chunk;
    program->current_chunk = new_chunk;
  }

  program->current_chunk->statements[program->current_chunk->used++] =
      statement;
  ++program->statements_len;
}

typedef struct StatementIterator {
  StatementChunk *chunk;
  size_t index;
} StatementIterator;

void statement_iterator_init(StatementIterator *iter, StatementChunk *chunk) {
  iter->chunk = chunk;
  iter->index = 0;
}

Statement *statement_iterator_next(StatementIterator *iter) {
  if (iter->index >= iter->chunk->used) {
    iter->chunk = iter->chunk->next;
    iter->index = 0;
  }

  if (!iter->chunk) {
    return NULL;
  }
  return &iter->chunk->statements[iter->index++];
}

String program_to_string(const Program *program, Arena *arena) {
  StringBuilder sb = string_builder_create(arena);
  StatementIterator iter = {0};
  statement_iterator_init(&iter, program->first_chunk);
  Statement *s;
  while ((s = statement_iterator_next(&iter))) {
    string_builder_append(&sb, statement_to_string(s, arena));
  }
  return string_builder_build(&sb);
}
