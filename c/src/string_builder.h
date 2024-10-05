#pragma once

#include <stdlib.h>

typedef struct {
  char *buf;
  size_t size;
  size_t cap;
} string_builder_t;

string_builder_t *new_string_builder(void);
int sb_append(string_builder_t *sb, const char *str);
int sb_appendf(string_builder_t *sb, const char *fmt, ...);
void sb_free(string_builder_t *sb);
