#pragma once

#include "../src/code.h"
#include "../src/object.h"
#include <stdlib.h>

void test_integer_object(object_t *o, int64_t expected);
instructions_t *instructions_flatten(size_t n, instructions_t **instructions);
