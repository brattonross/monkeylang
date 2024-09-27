#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  OBJECT_INTEGER,
  OBJECT_BOOLEAN,
  OBJECT_NULL,
} object_type_t;

typedef struct {
  int64_t value;
} integer_object_t;

typedef struct {
  bool value;
} boolean_object_t;

typedef struct {
  object_type_t type;
  union {
    integer_object_t *integer;
    boolean_object_t *boolean;
  } value;
} object_t;

char *object_inspect(object_t *o);
void object_free(object_t *o);

#endif
