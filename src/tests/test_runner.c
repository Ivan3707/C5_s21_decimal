#include <check.h>
#include <stdlib.h>

Suite* s21_div_suite(void);
Suite* s21_other_suite(void);
Suite* s21_helpers_suite(void);
Suite* s21_compare_suite(void);
Suite* s21_mul_suite(void);
Suite* s21_convert_suite(void);

int main(void) {
  Suite* dev_suite = s21_div_suite();
  Suite* other_suite = s21_other_suite();

  SRunner* runner = srunner_create(dev_suite);
  srunner_add_suite(runner, other_suite);
  srunner_add_suite(runner, s21_helpers_suite());
  srunner_add_suite(runner, s21_compare_suite());
  srunner_add_suite(runner, s21_mul_suite());
  srunner_add_suite(runner, s21_convert_suite());

  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? 0 : 1;
}