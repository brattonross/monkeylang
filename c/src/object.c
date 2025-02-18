#pragma once

#include "strconv.c"
#include <stdint.h>

typedef enum ObjectType {
  OBJECT_INTEGER,
  OBJECT_BOOLEAN,
  OBJECT_NULL,
} ObjectType;

typedef struct IntegerObject {
  int64_t value;
} IntegerObject;

String integer_object_to_string(const IntegerObject object, Arena *arena) {
  return string_from_int64(arena, object.value);
}

typedef struct BooleanObject {
  bool value;
} BooleanObject;

String boolean_object_to_string(const BooleanObject object) {
  if (object.value) {
    return String("true");
  } else {
    return String("false");
  }
}

typedef union ObjectData {
  IntegerObject integer;
  BooleanObject boolean;
} ObjectData;

typedef struct Object {
  ObjectType type;
  ObjectData data;
} Object;

String object_to_string(const Object *object, Arena *arena) {
  switch (object->type) {
  case OBJECT_INTEGER:
    return integer_object_to_string(object->data.integer, arena);
  case OBJECT_BOOLEAN:
    return boolean_object_to_string(object->data.boolean);
  case OBJECT_NULL:
    return String("null");
  }
}
