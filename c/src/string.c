#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct String {
  char *buffer;
  size_t len;
} String;

#define String(literal)                                                        \
  (String) { .buffer = literal, .len = strlen(literal) }

String string_slice(String s, size_t start, size_t end) {
  return (String){.buffer = s.buffer + start, .len = end - start};
}

bool string_cmp(String s1, String s2) {
  return s1.len == s2.len && strncmp(s1.buffer, s2.buffer, s1.len) == 0;
}

// typedef struct StringBuilder {
//
// } StringBuilder;
