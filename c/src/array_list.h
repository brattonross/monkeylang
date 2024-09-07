#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include <stdlib.h>

typedef struct {
  char **arr;
  size_t len;
  size_t cap;
} array_list_t;

typedef enum {
  ARRAY_LIST_SUCCESS = 0,
  ARRAY_LIST_ALLOC_ERROR = -1,
  ARRAY_LIST_INDEX_OUT_OF_BOUNDS_ERROR = -2,
} array_list_error_t;

array_list_t *array_list_create(const size_t cap);
void array_list_free(array_list_t *list);
const array_list_error_t array_list_push(array_list_t *list, const char *el);
const array_list_error_t array_list_get(array_list_t *list, const int i,
                                        char *el);
const array_list_error_t array_list_set(array_list_t *list, const int i,
                                        const char *el);
const array_list_error_t array_list_remove(array_list_t *list, const int i);

#endif
