#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Arena {
  unsigned char *buffer;
  size_t buffer_size;
  size_t prev_offset;
  size_t offset;
} Arena;

void arena_init(Arena *arena, void *buffer, size_t buffer_size) {
  arena->buffer = (unsigned char *)buffer;
  arena->buffer_size = buffer_size;
  arena->prev_offset = 0;
  arena->offset = 0;
}

void arena_reset(Arena *arena) { arena->offset = 0; }

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
    fprintf(stderr,
            "ERROR: arena attempted to allocate more memory than is available: "
            "want=%lu, got=%zu\n",
            offset + size, arena->buffer_size);
    return NULL;
  }

  void *ptr = &arena->buffer[offset];
  arena->prev_offset = arena->offset;
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

void *arena_resize_align(Arena *arena, void *old_memory, size_t old_size,
                         size_t new_size, size_t align) {
  unsigned char *old_mem = (unsigned char *)old_memory;

  assert(is_power_of_two(align));

  if (old_mem == NULL || old_size == 0) {
    return arena_alloc_align(arena, new_size, align);
  } else if (arena->buffer <= old_mem &&
             old_mem < arena->buffer + arena->buffer_size) {
    if (arena->buffer + arena->prev_offset == old_mem) {
      arena->offset = arena->prev_offset + new_size;
      if (new_size > old_size) {
        memset(&arena->buffer[arena->offset], 0, new_size - old_size);
      }
      return old_memory;
    } else {
      void *new_memory = arena_alloc_align(arena, new_size, align);
      size_t copy_size = old_size < new_size ? old_size : new_size;
      memmove(new_memory, old_memory, copy_size);
      return new_memory;
    }

  } else {
    assert(0 && "Memory is out of bounds of the buffer in this arena");
    return NULL;
  }
}

void *arena_resize(Arena *a, void *old_memory, size_t old_size,
                   size_t new_size) {
  return arena_resize_align(a, old_memory, old_size, new_size,
                            DEFAULT_ALIGNMENT);
}
