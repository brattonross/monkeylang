#pragma once

#include "mem.c"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct String {
  char *buffer;
  size_t len;
} String;

#define String(literal)                                                        \
  (String) { .buffer = literal, .len = strlen(literal) }

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

  return (String){.buffer = buffer, .len = len};
}

String string_slice(String s, size_t start, size_t end) {
  return (String){.buffer = s.buffer + start, .len = end - start};
}

bool string_cmp(String s1, String s2) {
  return s1.len == s2.len && strncmp(s1.buffer, s2.buffer, s1.len) == 0;
}

// typedef struct StringBuilder {
//
// } StringBuilder;
