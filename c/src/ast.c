#pragma once

#include "mem.c"
#include "token.c"
#include <stddef.h>

typedef enum ExpressionType {
  EXPRESSION_,
} ExpressionType;

typedef struct Expression {
  ExpressionType type;
  // union {
  // } value;
} Expression;

typedef struct Identifier {
  Token token;
  String value;
} Identifier;

typedef struct LetStatement {
  Token token;
  Identifier name;
  Expression value;
} LetStatement;

typedef enum StatementType {
  STATEMENT_LET,
} StatementType;

typedef struct Statement {
  StatementType type;
  union {
    LetStatement let;
  } value;
} Statement;

String statement_token_literal(const Statement s) {
  switch (s.type) {
  case STATEMENT_LET:
    return s.value.let.token.literal;
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
  Arena *arena;
  StatementChunk *first_chunk;
  StatementChunk *current_chunk;
  size_t statements_len;
} Program;

/**
 * Allocates a new instance of `Program` in the given arena.
 * We do this since the program and all associated statements probably want to
 * be using the same lifetime.
 */
Program *create_program(Arena *arena) {
  Program *program = arena_alloc(arena, sizeof(Program));
  StatementChunk *chunk = arena_alloc(arena, sizeof(StatementChunk));

  chunk->next = NULL;
  chunk->used = 0;

  program->arena = arena;
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

void program_append_statement(Program *program, Statement statement) {
  if (program->current_chunk->used == STATEMENT_CHUNK_SIZE) {
    StatementChunk *new_chunk =
        arena_alloc(program->arena, sizeof(StatementChunk));
    new_chunk->next = NULL;
    new_chunk->used = 0;

    program->current_chunk->next = new_chunk;
    program->current_chunk = new_chunk;
  }

  program->current_chunk->statements[program->current_chunk->used++] =
      statement;
  ++program->statements_len;
}
