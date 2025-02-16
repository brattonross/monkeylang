#include "../src/strconv.c"
#include "../src/string.c"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void test_string_to_int64(void);

int main(void) { test_string_to_int64(); }

void test_string_to_int64(void) {
  struct {
    String input;
    int64_t expected;
  } test_cases[] = {
      {
          String("5"),
          5,
      },
      {
          String("15"),
          15,
      },
      {String("-15"), -15},
  };

  for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i) {
    size_t end_ptr;
    int64_t actual = string_to_int64(test_cases[i].input, &end_ptr);
    assert(actual == test_cases[i].expected);
    assert(end_ptr == test_cases[i].input.length);
  }
}
