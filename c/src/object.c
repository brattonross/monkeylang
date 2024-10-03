#include "object.h"
#include "ast.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

object_t *new_null_object(void) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_NULL;
  return obj;
}

object_t *new_error_object(char *fmt, ...) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_ERROR;
  obj->value.err = malloc(sizeof(error_object_t));
  if (obj->value.err == NULL) {
    object_free(obj);
    return NULL;
  }

  va_list args;
  va_start(args, fmt);
  size_t len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  va_start(args, fmt);
  char *buf = malloc(len + 1);
  vsnprintf(buf, len + 1, fmt, args);
  va_end(args);
  obj->value.err->message = buf;
  return obj;
}

object_t *new_boolean_object(bool value) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_BOOLEAN;
  obj->value.boolean = malloc(sizeof(boolean_object_t));
  if (obj->value.boolean == NULL) {
    object_free(obj);
    return NULL;
  }
  obj->value.boolean->value = value;
  return obj;
}

object_t *new_integer_object(int64_t value) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->type = OBJECT_INTEGER;
  obj->value.integer = malloc(sizeof(integer_object_t));
  if (obj->value.integer == NULL) {
    object_free(obj);
    return NULL;
  }
  obj->value.integer->value = value;
  return obj;
}

object_t *new_function_object(size_t parameters_len, identifier_t **parameters,
                              environment_t *env, block_statement_t *body) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }

  obj->type = OBJECT_FUNCTION;
  obj->value.fn = malloc(sizeof(function_object_t));
  if (obj->value.fn == NULL) {
    object_free(obj);
    return NULL;
  }

  obj->value.fn->parameters_len = parameters_len;
  obj->value.fn->parameters = parameters;
  obj->value.fn->env = env;
  obj->value.fn->body = body;
  return obj;
}

object_t *new_string_object(const char *value) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }

  obj->type = OBJECT_STRING;
  obj->value.string = malloc(sizeof(string_object_t));
  if (obj->value.string == NULL) {
    object_free(obj);
    return NULL;
  }

  obj->value.string->value = strdup(value);
  return obj;
}

object_t *new_array_object(size_t len, object_t **elements) {
  object_t *obj = malloc(sizeof(object_t));
  if (obj == NULL) {
    return NULL;
  }

  obj->type = OBJECT_ARRAY;
  obj->value.array = malloc(sizeof(array_object_t));
  if (obj->value.array == NULL) {
    object_free(obj);
    return NULL;
  }

  obj->value.array->len = len;
  obj->value.array->elements = calloc(len, sizeof(object_t));
  if (obj->value.array->elements == NULL) {
    object_free(obj);
    return NULL;
  }
  memcpy(obj->value.array->elements, elements, len * sizeof(object_t));

  return obj;
}

object_t *builtin_len(size_t argc, object_t **argv) {
  if (argc != 1) {
    return new_error_object("wrong number of arguments. got=%d, want=1", argc);
  }

  if (argv[0]->type == OBJECT_STRING) {
    return new_integer_object(strlen(argv[0]->value.string->value));
  }
  if (argv[0]->type == OBJECT_ARRAY) {
    return new_integer_object(argv[0]->value.array->len);
  }

  return new_error_object("argument to `len` not supported, got %s",
                          object_type_to_string(argv[0]->type));
}

object_t *builtin_first(size_t argc, object_t **argv) {
  if (argc != 1) {
    return new_error_object("wrong number of arguments. got=%d, want=1", argc);
  }
  if (argv[0]->type != OBJECT_ARRAY) {
    return new_error_object("argument to `first` must be ARRAY, got %s",
                            object_type_to_string(argv[0]->type));
  }
  if (argv[0]->value.array->len > 0) {
    return argv[0]->value.array->elements[0];
  }
  return new_null_object();
}

object_t *builtin_last(size_t argc, object_t **argv) {
  if (argc != 1) {
    return new_error_object("wrong number of arguments. got=%d, want=1", argc);
  }
  if (argv[0]->type != OBJECT_ARRAY) {
    return new_error_object("argument to `last` must be ARRAY, got %s",
                            object_type_to_string(argv[0]->type));
  }
  if (argv[0]->value.array->len > 0) {
    return argv[0]->value.array->elements[argv[0]->value.array->len - 1];
  }
  return new_null_object();
}

object_t *builtin_rest(size_t argc, object_t **argv) {
  if (argc != 1) {
    return new_error_object("wrong number of arguments. got=%d, want=1", argc);
  }
  if (argv[0]->type != OBJECT_ARRAY) {
    return new_error_object("argument to `rest` must be ARRAY, got %s",
                            object_type_to_string(argv[0]->type));
  }
  if (argv[0]->value.array->len > 1) {
    return new_array_object(argv[0]->value.array->len - 1,
                            &argv[0]->value.array->elements[1]);
  }
  return new_null_object();
}

object_t *builtin_push(size_t argc, object_t **argv) {
  if (argc != 2) {
    return new_error_object("wrong number of arguments. got=%d, want=2", argc);
  }
  if (argv[0]->type != OBJECT_ARRAY) {
    return new_error_object("argument to `push` must be ARRAY, got %s",
                            object_type_to_string(argv[0]->type));
  }
  size_t len = argv[0]->value.array->len + 1;
  object_t **arr = calloc(len, sizeof(object_t));
  if (arr == NULL) {
    return new_null_object();
  }
  memcpy(arr, argv[0]->value.array->elements, (len - 1) * sizeof(object_t));
  arr[len - 1] = malloc(sizeof(object_t));
  if (arr[len - 1] == NULL) {
    free(arr);
    arr = NULL;
    return new_null_object();
  }
  memcpy(arr[len - 1], argv[1], sizeof(object_t));
  object_t *out = new_array_object(len, arr);
  free(arr);
  arr = NULL;
  return out;
}

typedef struct {
  char *name;
  object_t *builtin;
} builtin_definition_t;
static const builtin_definition_t builtins[] = {
    {"len", &(object_t){OBJECT_BUILTIN,
                        .value.builtin = &(builtin_object_t){builtin_len}}},
    {"first", &(object_t){OBJECT_BUILTIN,
                          .value.builtin = &(builtin_object_t){builtin_first}}},
    {"last", &(object_t){OBJECT_BUILTIN,
                         .value.builtin = &(builtin_object_t){builtin_last}}},
    {"rest", &(object_t){OBJECT_BUILTIN,
                         .value.builtin = &(builtin_object_t){builtin_rest}}},
    {"push", &(object_t){OBJECT_BUILTIN,
                         .value.builtin = &(builtin_object_t){builtin_push}}},
};
static const size_t builtins_len = sizeof(builtins) / sizeof(*builtins);

object_t *lookup_builtin(const char *name) {
  // TODO: This is linear
  for (size_t i = 0; i < builtins_len; ++i) {
    if (strncmp(builtins[i].name, name, strlen(name)) == 0) {
      return builtins[i].builtin;
    }
  }
  return NULL;
}

char *object_inspect(object_t *obj) {
  switch (obj->type) {
  case OBJECT_INTEGER: {
    char *buf = malloc(21);
    snprintf(buf, sizeof(buf), "%" PRId64, obj->value.integer->value);
    return buf;
  }
  case OBJECT_BOOLEAN:
    return strdup(obj->value.boolean->value ? "true" : "false");
  case OBJECT_NULL:
    return strdup("null");
  case OBJECT_RETURN:
    return object_inspect(obj->value.return_value->value);
  case OBJECT_ERROR:
    size_t len = strlen(obj->value.err->message) + 8;
    char *message = malloc(len + 1);
    snprintf(message, len, "ERROR: %s", obj->value.err->message);
    return message;
  case OBJECT_FUNCTION: {
    char *param_strs[obj->value.fn->parameters_len];
    size_t total_len = 10; // min space for `fn() {\n\n}` + null terminator.
    for (size_t i = 0; i < obj->value.fn->parameters_len; ++i) {
      char *param_str = identifier_to_string(obj->value.fn->parameters[i]);
      // TODO: what if param_str is NULL?
      if (total_len > 10) {
        total_len += 2; // ", "
      }
      total_len += strlen(param_str);
      param_strs[i] = param_str;
    }

    char *body = block_statement_to_string(obj->value.fn->body);
    size_t body_len = strlen(body);
    total_len += body_len;

    char *res = malloc(total_len);
    char *out = res;
    *out = '\0';

    memcpy(out, "fn(", 3);
    out += 3;

    for (size_t i = 0; i < obj->value.fn->parameters_len; ++i) {
      if (i > 0) {
        memcpy(out, ", ", 2);
        out += 2;
      }

      size_t len = strlen(param_strs[i]);
      memcpy(out, param_strs[i], len);
      out += len;
      free(param_strs[i]);
      param_strs[i] = NULL;
    }

    memcpy(out, ") {\n", 4);
    out += 4;

    memcpy(out, body, body_len);
    out += body_len;

    memcpy(out, "\n}", 2);
    out += 2;

    *out = '\0';
    return res;
  }
  case OBJECT_STRING:
    return strdup(obj->value.string->value);
  case OBJECT_BUILTIN:
    return strdup("builtin function");
  case OBJECT_ARRAY: {
    array_object_t *arr = obj->value.array;
    size_t total_len = 3; // []
    char *elements[arr->len];
    for (size_t i = 0; i < arr->len; ++i) {
      if (i > 0) {
        total_len += 2; // ", "
      }
      char *s = object_inspect(arr->elements[i]);
      elements[i] = s;
      total_len += strlen(s);
    }

    char *buf = malloc(total_len);
    char *out = buf;
    *out = '\0';

    memcpy(out, "[", 1);
    out++;

    for (size_t i = 0; i < arr->len; ++i) {
      if (i > 0) {
        memcpy(out, ", ", 2);
        out += 2;
      }
      memcpy(out, elements[i], strlen(elements[i]));
      out += strlen(elements[i]);
    }

    memcpy(out, "]", 1);
    out++;

    *out = '\0';
    return buf;
  }
  case OBJECT_HASH: {
    hash_object_t *hash = obj->value.hash;
    size_t total_len = 3; // {}
    char *keys[hash->len];
    char *values[hash->len];
    for (size_t i = 0; i < hash->len; ++i) {
      if (i > 0) {
        total_len += 2; // ", "
      }
      keys[i] = object_inspect(hash->pairs[i]->key);
      total_len += strlen(keys[i]);
      total_len += 2; // ": "
      values[i] = object_inspect(hash->pairs[i]->value);
      total_len += strlen(keys[i]);
    }

    char *buf = malloc(total_len);
    char *out = buf;
    *out = '\0';

    memcpy(out, "{", 1);
    out++;

    for (size_t i = 0; i < hash->len; ++i) {
      if (i > 0) {
        memcpy(out, ", ", 2);
        out += 2;
      }

      memcpy(out, keys[i], strlen(keys[i]));
      out += strlen(keys[i]);

      memcpy(out, ": ", 2);
      out += 2;

      memcpy(out, values[i], strlen(values[i]));
      out += strlen(values[i]);
    }

    memcpy(out, "}", 1);
    out++;

    *out = '\0';
    return buf;
  }
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

void string_object_free(string_object_t *obj) {
  free(obj->value);
  obj->value = NULL;
  free(obj);
  obj = NULL;
}

void array_object_free(array_object_t *obj) {
  for (size_t i = 0; i < obj->len; ++i) {
    object_free(obj->elements[i]);
  }
  free(obj);
  obj = NULL;
}

void hash_object_free(hash_object_t *obj) {
  for (size_t i = 0; i < obj->len; ++i) {
    object_free(obj->pairs[i]->key);
    object_free(obj->pairs[i]->value);
    free(obj->pairs[i]);
    obj->pairs[i] = NULL;
  }
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
  case OBJECT_FUNCTION:
    function_object_free(obj->value.fn);
    break;
  case OBJECT_STRING:
    string_object_free(obj->value.string);
    break;
  case OBJECT_BUILTIN:
    // TODO: ?
    break;
  case OBJECT_ARRAY:
    array_object_free(obj->value.array);
    break;
  case OBJECT_HASH:
    hash_object_free(obj->value.hash);
    break;
  case OBJECT_NULL:
    // nothing to do
    break;
  }

  free(obj);
  obj = NULL;
}

void function_object_free(function_object_t *obj) {
  // TODO: should we free env here? I don't think so?
  block_statement_free(obj->body);
  for (size_t i = 0; i < obj->parameters_len; ++i) {
    identifier_free(obj->parameters[i]);
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
  case OBJECT_FUNCTION:
    return strdup("FUNCTION");
  case OBJECT_STRING:
    return strdup("STRING");
  case OBJECT_BUILTIN:
    return strdup("BUILTIN");
  case OBJECT_ARRAY:
    return strdup("ARRAY");
  case OBJECT_HASH:
    return strdup("HASH");
  }
}

environment_t *new_environment() {
  environment_t *env = malloc(sizeof(environment_t));
  if (env == NULL) {
    return NULL;
  }
  env->items_len = 0;
  env->items = NULL;
  env->outer = NULL;
  return env;
}

environment_t *new_enclosed_environment(environment_t *outer) {
  environment_t *env = new_environment();
  env->outer = outer;
  return env;
}

object_t *environment_get(const environment_t *env, const char *key) {
  // TODO: this is a linear search
  for (size_t i = 0; i < env->items_len; ++i) {
    if (strncmp(key, env->items[i]->key, strlen(env->items[i]->key)) == 0) {
      return env->items[i]->value;
    }
  }
  if (env->outer != NULL) {
    return environment_get(env->outer, key);
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

hash_key_t *new_hash_key(object_type_t type, uint64_t value) {
  hash_key_t *key = malloc(sizeof(hash_key_t));
  if (key == NULL) {
    return NULL;
  }
  key->type = type;
  key->value = value;
  return key;
}

object_t *new_hash_object(size_t len, hash_object_item_t **pairs) {
  object_t *o = malloc(sizeof(object_t));
  if (o == NULL) {
    return NULL;
  }

  o->type = OBJECT_HASH;
  o->value.hash = malloc(sizeof(hash_object_t));
  if (o->value.hash == NULL) {
    object_free(o);
    return NULL;
  }

  o->value.hash->len = len;
  o->value.hash->pairs = pairs;

  return o;
}

uint64_t hash_string(char *str) {
  uint64_t hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

hash_key_t *object_hash_key(object_t *obj) {
  switch (obj->type) {
  case OBJECT_BOOLEAN:
    return new_hash_key(OBJECT_BOOLEAN, obj->value.boolean->value);
  case OBJECT_INTEGER:
    return new_hash_key(OBJECT_INTEGER, obj->value.integer->value);
  case OBJECT_STRING:
    return new_hash_key(OBJECT_STRING, hash_string(obj->value.string->value));
  default:
    return NULL;
  }
}

void hash_key_free(hash_key_t *h) {
  free(h);
  h = NULL;
}

void hash_object_item_free(hash_object_item_t *h) {
  object_free(h->key);
  object_free(h->value);
  free(h);
  h = NULL;
}

void hash_object_items_free(size_t n, hash_object_item_t **h) {
  for (size_t i = 0; i < n; ++i) {
    hash_object_item_free(h[i]);
  }
  free(h);
  h = NULL;
}
