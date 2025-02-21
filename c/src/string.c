#pragma once

#include "mem.c"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct String {
  char *buffer;
  size_t length;
} String;

#define String(literal)                                                        \
  (String) { .buffer = literal, .length = strlen(literal) }

String string_fmt(Arena *arena, const char *fmt, ...) {
  va_list args1;
  va_list args2;
  va_start(args1, fmt);
  va_copy(args2, args1);

  int len = vsnprintf(NULL, 0, fmt, args1);
  va_end(args1);

  char *buffer = arena_alloc(arena, len + 1);
  vsnprintf(buffer, len + 1, fmt, args2);
  buffer[len] = 0;
  va_end(args2);

  return (String){.buffer = buffer, .length = len};
}

String string_slice(String s, size_t start, size_t end) {
  return (String){.buffer = s.buffer + start, .length = end - start};
}

bool string_cmp(String s1, String s2) {
  return s1.length == s2.length &&
         strncmp(s1.buffer, s2.buffer, s1.length) == 0;
}

typedef struct StringBuilder {
  String *chunks;
  size_t capacity;
  size_t length;
  Arena *arena;
} StringBuilder;

StringBuilder string_builder_create(Arena *arena) {
  return (StringBuilder){
      .chunks = arena_alloc(arena, 8 * sizeof(String)),
      .capacity = 8,
      .length = 0,
      .arena = arena,
  };
}

void string_builder_append(StringBuilder *sb, String s) {
  if (sb->length == sb->capacity) {
    size_t new_capacity = sb->capacity * 2;
    String *new_chunks = arena_alloc(sb->arena, new_capacity * sizeof(String));
    memcpy(new_chunks, sb->chunks, sb->length * sizeof(String));
    sb->chunks = new_chunks;
    sb->capacity = new_capacity;
  }

  char *buf = arena_alloc(sb->arena, s.length);
  memcpy(buf, s.buffer, s.length);

  sb->chunks[sb->length++] = (String){
      .buffer = buf,
      .length = s.length,
  };
}

String string_builder_build(StringBuilder *sb) {
  size_t total_length = 0;
  for (size_t i = 0; i < sb->length; ++i) {
    total_length += sb->chunks[i].length;
  }

  char *buffer = arena_alloc(sb->arena, total_length);
  size_t offset = 0;

  for (size_t i = 0; i < sb->length; ++i) {
    memcpy(buffer + offset, sb->chunks[i].buffer, sb->chunks[i].length);
    offset += sb->chunks[i].length;
  }

  return (String){
      .buffer = buffer,
      .length = total_length,
  };
}

bool is_letter(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_digit(char c) { return '0' <= c && c <= '9'; }

String arena_strdup(Arena *arena, String s) {
  char *buffer = arena_alloc(arena, s.length + 1);
  memcpy(buffer, s.buffer, s.length);
  buffer[s.length] = 0;
  return (String){.buffer = buffer, .length = s.length};
}
