#include <check.h>
#include <stdlib.h>

#include "../s21_decimal.h"

START_TEST(test_get_sign) {
  int sign = _i / 29;
  int scale = _i % 29;
  s21_decimal value = {{567, 123, 456, 0}};
  value.bits[3] = (int)(((uint32_t)sign << 31) | ((uint32_t)scale << 16));
  ck_assert_int_eq(s21_get_sign(value), sign);
}
END_TEST

START_TEST(test_get_scale) {
  int sign = _i / 29;
  int scale = _i % 29;
  s21_decimal value = {{567, 123, 456, 0}};
  value.bits[3] = (int)(((uint32_t)sign << 31) | ((uint32_t)scale << 16));
  ck_assert_int_eq(s21_get_scale(value), scale);
}
END_TEST

START_TEST(test_set_sign) {
  int requested_sign = _i % 2;
  int original_sign = (_i / 2) % 2;
  int scale = _i / 4;
  s21_decimal value = {{567, 123, 456, 0}};
  value.bits[3] =
      (int)(((uint32_t)original_sign << 31) | ((uint32_t)scale << 16));

  s21_set_sign(&value, requested_sign);

  uint32_t expected =
      ((uint32_t)requested_sign << 31) | ((uint32_t)scale << 16);
  ck_assert_uint_eq((uint32_t)value.bits[3], expected);
  ck_assert_int_eq(value.bits[0], 567);
  ck_assert_int_eq(value.bits[1], 123);
  ck_assert_int_eq(value.bits[2], 456);
}
END_TEST

START_TEST(test_zero) {
  s21_decimal value = {{-1, -1, -1, 0}};
  value.bits[3] = (int)((1u << 31) | (28u << 16));
  s21_zero(&value);
  for (int i = 0; i < 4; i++) {
    ck_assert_int_eq(value.bits[i], 0);
  }
  s21_zero(&value);
  for (int i = 0; i < 4; i++) {
    ck_assert_int_eq(value.bits[i], 0);
  }
}
END_TEST

Suite* s21_helpers_suite(void) {
  Suite* suite = suite_create("decimal helpers");
  TCase* cases = tcase_create("sign, scale and zero");
  tcase_add_loop_test(cases, test_get_sign, 0, 58);
  tcase_add_loop_test(cases, test_get_scale, 0, 58);
  tcase_add_loop_test(cases, test_set_sign, 0, 116);
  tcase_add_test(cases, test_zero);
  suite_add_tcase(suite, cases);
  return suite;
}

#ifdef S21_HELPERS_TEST_MAIN
int main(void) {
  SRunner* runner = srunner_create(s21_helpers_suite());
  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
