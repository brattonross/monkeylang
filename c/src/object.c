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
  case OBJECT_RETURN:
    return object_inspect(o->value.return_value->value);
  case OBJECT_ERROR:
    size_t len = strlen(o->value.err->message) + 7;
    char *message = malloc(len + 1);
    snprintf(message, len, "ERROR: %s", o->value.err->message);
    message[len] = '\0';
    return message;
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
  case OBJECT_NULL:
    break;
  case OBJECT_RETURN:
    object_free(o->value.return_value->value);
    break;
  case OBJECT_ERROR:
    free(o->value.err->message);
    break;
  }
  free(o);
}

char *object_type_to_string(object_type_t t) {
  switch (t) {
  case OBJECT_INTEGER:
    return strdup("INTEGER");
  case OBJECT_BOOLEAN:
    return strdup("BOOLEAN");
  case OBJECT_NULL:
    return strdup("NULL");
  case OBJECT_RETURN:
    return strdup("RETURN");
  case OBJECT_ERROR:
    return strdup("ERROR");
  }
}
