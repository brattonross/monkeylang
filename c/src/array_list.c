#include "array_list.h"
#include <stdlib.h>
#include <string.h>

array_list_t *array_list_create(const size_t cap) {
  array_list_t *list = malloc(sizeof(array_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->arr = calloc(cap, sizeof(char *));
  if (list->arr == NULL) {
    array_list_free(list);
    return NULL;
  }

  list->len = 0;
  list->cap = cap;
  return list;
}

void array_list_free(array_list_t *list) {
  if (list == NULL) {
    return;
  }
  for (size_t i = 0; i < list->len; ++i) {
    free(list->arr[i]);
    list->arr[i] = NULL;
  }
  free(list->arr);
  list->arr = NULL;
  free(list);
  list = NULL;
}

array_list_error_t array_list_resize(array_list_t *list, const size_t cap) {
  char **temp = realloc(list->arr, sizeof(char *) * cap);
  if (temp == NULL) {
    return ARRAY_LIST_ALLOC_ERROR;
  }
  list->arr = temp;
  list->cap = cap;
  return ARRAY_LIST_SUCCESS;
}

array_list_error_t array_list_push(array_list_t *list, const char *el) {
  if (list->len == list->cap) {
    array_list_error_t err = array_list_resize(list, list->cap * 2);
    if (err != ARRAY_LIST_SUCCESS) {
      return err;
    }
  }
  list->arr[list->len] = strdup(el);
  if (list->arr[list->len] == NULL) {
    return ARRAY_LIST_ALLOC_ERROR;
  }
  list->len++;
  return ARRAY_LIST_SUCCESS;
}

array_list_error_t array_list_get(array_list_t *list, size_t i, char *el) {
  if (i < 0 || i >= list->len) {
    return ARRAY_LIST_INDEX_OUT_OF_BOUNDS_ERROR;
  }
  strncpy(el, list->arr[i], strlen(list->arr[i]));
  return ARRAY_LIST_SUCCESS;
}

array_list_error_t array_list_set(array_list_t *list, const size_t i,
                                  const char *el) {
  if (i < 0 || i >= list->len) {
    return ARRAY_LIST_INDEX_OUT_OF_BOUNDS_ERROR;
  }
  strncpy(list->arr[i], el, strlen(list->arr[i]));
  return ARRAY_LIST_SUCCESS;
}

array_list_error_t array_list_remove(array_list_t *list, const size_t i) {
  if (i < 0 || i >= list->len) {
    return ARRAY_LIST_INDEX_OUT_OF_BOUNDS_ERROR;
  }

  free(list->arr[i]);
  list->arr[i] = NULL;

  for (size_t j = i; j < list->len - 1; ++j) {
    list->arr[j] = list->arr[j + 1];
  }
  list->len--;

  if (list->len > 0 && list->len < list->cap / 4) {
    // TODO: we ignore the return here
    array_list_resize(list, list->cap / 2);
  }
  return ARRAY_LIST_SUCCESS;
}
