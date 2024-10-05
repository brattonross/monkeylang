#include "string_builder.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t initial_buffer_size = 16;

string_builder_t *new_string_builder(void) {
  string_builder_t *sb = malloc(sizeof(string_builder_t));
  if (!sb) {
    return NULL;
  }

  sb->buf = malloc(initial_buffer_size);
  if (!sb->buf) {
    free(sb);
    return NULL;
  }

  sb->buf[0] = '\0';
  sb->size = 0;
  sb->cap = initial_buffer_size;

  return sb;
}

int sb_ensure_cap(string_builder_t *sb, size_t additional) {
  size_t required = sb->size + additional + 1;
  if (required <= sb->cap) {
    return 0;
  }

  size_t cap = sb->cap;
  while (cap < required) {
    cap *= 2;
  }

  char *buf = realloc(sb->buf, cap);
  if (!buf) {
    return -1;
  }

  sb->buf = buf;
  sb->cap = cap;

  return 0;
}

int sb_append(string_builder_t *sb, const char *str) {
  if (!sb || !str) {
    return -1;
  }

  size_t len = strlen(str);
  if (len == 0) {
    return 0;
  }

  if (sb_ensure_cap(sb, len) != 0) {
    return -1;
  }

  memcpy(sb->buf + sb->size, str, len);
  sb->size += len;
  sb->buf[sb->size] = '\0';

  return 0;
}

int sb_appendf(string_builder_t *sb, const char *fmt, ...) {
  if (!sb || !fmt) {
    return -1;
  }

  va_list va1, va2;
  va_start(va1, fmt);
  va_copy(va2, va1);

  size_t len = vsnprintf(NULL, 0, fmt, va1);
  va_end(va1);

  if (len < 0) {
    va_end(va2);
    return -1;
  }

  if (sb_ensure_cap(sb, len) != 0) {
    va_end(va2);
    return -1;
  }

  int result = vsnprintf(sb->buf + sb->size, len + 1, fmt, va2);
  va_end(va2);

  if (result < 0) {
    return -1;
  }

  sb->size += len;
  return 0;
}

void sb_free(string_builder_t *sb) {
  if (!sb) {
    return;
  }
  free(sb->buf);
  free(sb);
}
