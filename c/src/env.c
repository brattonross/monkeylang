#pragma once

#include "mem.c"
#include "object.c"
#include "string.c"
#include <string.h>

typedef struct EnvironmentItem {
  String key;
  Object *value;
} EnvironmentItem;

typedef struct Environment {
  EnvironmentItem *items;
  size_t capacity;
  size_t count;
} Environment;

void environment_init(Environment *env, Arena *arena) {
  env->items = arena_alloc(arena, 16 * sizeof(EnvironmentItem));
  env->capacity = 16;
  env->count = 0;
}

Object *environment_get(Environment *env, String key) {
  Object *result = NULL;
  for (size_t i = 0; i < env->count; ++i) {
    if (string_cmp(env->items[i].key, key)) {
      result = env->items[i].value;
      break;
    }
  }
  return result;
}

void environment_set(Environment *env, Arena *arena, String key,
                     Object *value) {
  for (size_t i = 0; i < env->count; ++i) {
    if (string_cmp(env->items[i].key, key)) {
      env->items[i].value = value;
      return;
    }
  }

  if (env->count == env->capacity) {
    arena_resize(arena, env->items, env->capacity * sizeof(EnvironmentItem),
                 env->capacity * 2 * sizeof(EnvironmentItem));
    env->capacity *= 2;
  }

  env->items[env->count].key = arena_strdup(arena, key);
  env->items[env->count].value = arena_alloc(arena, sizeof(Object));
  memcpy(env->items[env->count].value, value, sizeof(Object));
  ++env->count;
}
