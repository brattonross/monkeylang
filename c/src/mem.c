#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Arena {
  unsigned char *buffer;
  size_t buffer_size;
  size_t offset;
} Arena;

void arena_init(Arena *arena, void *buffer, size_t buffer_size) {
  arena->buffer = (unsigned char *)buffer;
  arena->buffer_size = buffer_size;
  arena->offset = 0;
}

bool is_power_of_two(uintptr_t x) { return (x & (x - 1)) == 0; }

uintptr_t align_forward(uintptr_t ptr, size_t align) {
  assert(is_power_of_two(align));
  uintptr_t p = ptr;
  uintptr_t a = (uintptr_t)align;
  uintptr_t modulo = p & (a - 1);
  if (modulo != 0) {
    p += a - modulo;
  }
  return p;
}

void *arena_alloc_align(Arena *arena, size_t size, size_t align) {
  uintptr_t current_ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;
  uintptr_t offset = align_forward(current_ptr, align);
  offset -= (uintptr_t)arena->buffer;

  if (offset + size > arena->buffer_size) {
    return NULL;
  }

  void *ptr = &arena->buffer[offset];
  arena->offset = offset + size;
  memset(ptr, 0, size);
  return ptr;
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#endif

void *arena_alloc(Arena *arena, size_t size) {
  return arena_alloc_align(arena, size, DEFAULT_ALIGNMENT);
}
