#pragma once

#include "mem.c"
#include "string.c"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int64_t string_to_int64(String str, size_t *end_ptr) {
  int64_t result = 0;
  size_t i = 0;

  bool negate = str.buffer[0] == '-';
  if (negate) {
    ++i;
  }

  for (; i < str.length; ++i) {
    if (!is_digit(str.buffer[i])) {
      break;
    }
    result = (result * 10) + (str.buffer[i] - '0');
  }
  *end_ptr = i;

  if (negate) {
    result *= -1;
  }
  return result;
}

String string_from_int64(Arena *arena, int64_t value) {
  return string_fmt(arena, "%" PRId64, value);
}
