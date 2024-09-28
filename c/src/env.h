#ifndef __ENV_H__
#define __ENV_H__

#include "object.h"
#include <stdlib.h>

typedef struct {
  char *key;
  object_t *value;
} item_t;

typedef struct {
  size_t items_len;
  item_t **items;
} environment_t;

environment_t *new_environment();
object_t *environment_get(const environment_t *env, const char *key);
object_t *environment_set(environment_t *env, const char *key, object_t *value);

#endif
