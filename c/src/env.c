#include "env.h"
#include <stdlib.h>
#include <string.h>

environment_t *new_environment() {
  environment_t *env = malloc(sizeof(environment_t));
  if (env == NULL) {
    return NULL;
  }
  env->items_len = 0;
  env->items = NULL;
  return env;
}

object_t *environment_get(const environment_t *env, const char *key) {
  // TODO: this is a linear search
  for (size_t i = 0; i < env->items_len; ++i) {
    if (strncmp(key, env->items[i]->key, strlen(env->items[i]->key)) == 0) {
      return env->items[i]->value;
    }
  }
  return NULL;
}

void item_free(item_t *item) {
  object_free(item->value);
  item->value = NULL;
  free(item->key);
  item->key = NULL;
  free(item);
  item = NULL;
}

item_t *new_item(const char *key, object_t *value) {
  item_t *item = malloc(sizeof(item_t));
  if (item == NULL) {
    return NULL;
  }
  item->key = strdup(key);
  item->value = value;
  return item;
}

object_t *environment_set(environment_t *env, const char *key,
                          object_t *value) {
  // TODO: this is a linear search
  for (size_t i = 0; i < env->items_len; ++i) {
    if (strncmp(key, env->items[i]->key, strlen(env->items[i]->key)) == 0) {
      item_free(env->items[i]);
      env->items[i] = new_item(key, value);
      return value;
    }
  }

  // we didn't find an existing item with the key, create a new one.
  env->items_len++;
  env->items = realloc(env->items, env->items_len * sizeof(item_t));
  if (env->items == NULL) {
    env->items = NULL;
    env->items_len = 0;
    return NULL;
  }
  env->items[env->items_len - 1] = new_item(key, value);
  return value;
}
