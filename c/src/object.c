#pragma once

#include "strconv.c"
#include "string.c"
#include <stdint.h>

typedef enum ObjectType {
  OBJECT_INTEGER,
  OBJECT_BOOLEAN,
  OBJECT_NULL,
  OBJECT_RETURN,
  OBJECT_ERROR,
} ObjectType;

const String object_type_strings[] = {
    String("INTEGER"),
    String("BOOLEAN"),
    String("NULL"),
};

typedef struct Object Object;

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

typedef struct ReturnObject {
  Object *value;
} ReturnObject;

typedef struct ErrorObject {
  String message;
} ErrorObject;

typedef union ObjectData {
  IntegerObject integer_object;
  BooleanObject boolean_object;
  ReturnObject return_object;
  ErrorObject error_object;
} ObjectData;

struct Object {
  ObjectType type;
  ObjectData data;
};

String object_to_string(const Object *object, Arena *arena) {
  switch (object->type) {
  case OBJECT_INTEGER:
    return integer_object_to_string(object->data.integer_object, arena);
  case OBJECT_BOOLEAN:
    return boolean_object_to_string(object->data.boolean_object);
  case OBJECT_NULL:
    return String("null");
  case OBJECT_RETURN:
    return object_to_string(object->data.return_object.value, arena);
  case OBJECT_ERROR:
    return string_fmt(arena, "ERROR: %.*s",
                      object->data.error_object.message.length,
                      object->data.error_object.message.buffer);
  }
}

void null_object(Object *object) {
  object->type = OBJECT_NULL;
  object->data = (ObjectData){0};
}
