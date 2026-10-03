#include <check.h>
#include <stdlib.h>

Suite* s21_div_suite(void);

int main(void) {
  Suite* string_suite = s21_div_suite();

  SRunner* runner = srunner_create(string_suite);

  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? 0 : 1;
}