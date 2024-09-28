#include "object.h"
#include "ast.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    size_t len = strlen(obj->value.err->message) + 7;
    char *message = malloc(len + 1);
    snprintf(message, len, "ERROR: %s", obj->value.err->message);
    message[len] = '\0';
    return message;
  case OBJECT_FUNCTION:
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
  case OBJECT_FUNCTION:
    function_object_free(obj->value.fn);
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
