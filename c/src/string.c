#pragma once

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

// typedef struct StringBuilder {
//
// } StringBuilder;
