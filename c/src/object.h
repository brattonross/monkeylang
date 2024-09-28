#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  OBJECT_INTEGER,
  OBJECT_BOOLEAN,
  OBJECT_NULL,
  OBJECT_RETURN,
  OBJECT_ERROR,
} object_type_t;

typedef struct object_t object_t;

typedef struct {
  int64_t value;
} integer_object_t;

void integer_object_free(integer_object_t *obj);

typedef struct {
  bool value;
} boolean_object_t;

void boolean_object_free(boolean_object_t *obj);

typedef struct {
  object_t *value;
} return_value_t;

void return_value_free(return_value_t *obj);

typedef struct {
  char *message;
} error_object_t;

void error_object_free(error_object_t *obj);

struct object_t {
  object_type_t type;
  union {
    integer_object_t *integer;
    boolean_object_t *boolean;
    return_value_t *return_value;
    error_object_t *err;
  } value;
};

char *object_inspect(object_t *o);
void object_free(object_t *o);
char *object_type_to_string(object_type_t t);

#endif
