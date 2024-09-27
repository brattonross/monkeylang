#include "object.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *object_inspect(object_t *o) {
  switch (o->type) {
  case OBJECT_INTEGER: {
    char *buf = malloc(21);
    snprintf(buf, sizeof(buf), "%" PRId64, o->value.integer->value);
    return buf;
  }
  case OBJECT_BOOLEAN:
    return strdup(o->value.boolean->value ? "true" : "false");
  case OBJECT_NULL:
    return strdup("null");
  }
}

void object_free(object_t *o) {
  switch (o->type) {
  case OBJECT_INTEGER:
    free(o->value.integer);
    break;
  case OBJECT_BOOLEAN:
    free(o->value.boolean);
    break;
  default:
    // noop
  }
  free(o);
}
