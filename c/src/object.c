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

void integer_object_free(integer_object_t *obj) {
  free(obj);
  obj = NULL;
}

void boolean_object_free(boolean_object_t *obj) {
  free(obj);
  obj = NULL;
}

void return_value_free(return_value_t *obj) {
  object_free(obj->value);
  free(obj);
  obj = NULL;
}

void error_object_free(error_object_t *obj) {
  free(obj->message);
  obj->message = NULL;
  free(obj);
  obj = NULL;
}

void object_free(object_t *obj) {
  switch (obj->type) {
  case OBJECT_INTEGER:
    integer_object_free(obj->value.integer);
    break;
  case OBJECT_BOOLEAN:
    boolean_object_free(obj->value.boolean);
    break;
  case OBJECT_RETURN:
    return_value_free(obj->value.return_value);
    break;
  case OBJECT_ERROR:
    error_object_free(obj->value.err);
    break;
  case OBJECT_NULL:
    // nothing to do
    break;
  }

  free(obj);
  obj = NULL;
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
