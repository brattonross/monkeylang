#include "object_test.h"
#include "../src/object.h"
#include "unity.h"

void test_string_hash_key(void) {
  object_t *hello1 = &(object_t){
      OBJECT_STRING, .value.string = &(string_object_t){"Hello World"}};
  object_t *hello2 = &(object_t){
      OBJECT_STRING, .value.string = &(string_object_t){"Hello World"}};
  object_t *diff1 = &(object_t){
      OBJECT_STRING, .value.string = &(string_object_t){"My name is johnny"}};
  object_t *diff2 = &(object_t){
      OBJECT_STRING, .value.string = &(string_object_t){"My name is johnny"}};

  hash_key_t *hash_hello1 = object_hash_key(hello1);
  hash_key_t *hash_hello2 = object_hash_key(hello2);
  hash_key_t *hash_diff1 = object_hash_key(diff1);
  hash_key_t *hash_diff2 = object_hash_key(diff2);

  TEST_ASSERT_EQUAL_INT(hash_hello1->type, hash_hello2->type);
  TEST_ASSERT_EQUAL_INT(hash_hello1->value, hash_hello2->value);

  TEST_ASSERT_EQUAL_INT(hash_diff1->type, hash_diff2->type);
  TEST_ASSERT_EQUAL_INT(hash_diff1->value, hash_diff2->value);

  TEST_ASSERT_EQUAL_INT(hash_hello1->type, hash_diff1->type);
  TEST_ASSERT_NOT_EQUAL_INT(hash_hello1->value, hash_diff1->value);
}
