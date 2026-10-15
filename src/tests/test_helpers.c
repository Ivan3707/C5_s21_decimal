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

START_TEST(test_set_scale) {
  s21_decimal value = {{567, 123, 456, 0}};
  value.bits[3] = (int)((1u << 31) | (7u << 16));

  s21_set_scale(&value, 2);

  ck_assert_int_eq(s21_get_scale(value), 2);
  ck_assert_int_eq(s21_get_sign(value), 1);
  ck_assert_int_eq(value.bits[0], 567);
  ck_assert_int_eq(value.bits[1], 123);
  ck_assert_int_eq(value.bits[2], 456);

  s21_set_scale(&value, 0);
  ck_assert_int_eq(s21_get_scale(value), 0);
  ck_assert_int_eq(s21_get_sign(value), 1);
}
END_TEST

START_TEST(test_add_mantissa_simple) {
  s21_decimal value_1 = {{1, 2, 3, 0}};
  s21_decimal value_2 = {{10, 20, 30, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int overflow = s21_add_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(overflow, 0);
  ck_assert_int_eq(result.bits[0], 11);
  ck_assert_int_eq(result.bits[1], 22);
  ck_assert_int_eq(result.bits[2], 33);
}
END_TEST

START_TEST(test_add_mantissa_carry_to_next_word) {
  s21_decimal value_1 = {{-1, 0, 0, 0}};
  s21_decimal value_2 = {{1, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int overflow = s21_add_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(overflow, 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 1);
  ck_assert_int_eq(result.bits[2], 0);
}
END_TEST

START_TEST(test_add_mantissa_overflow) {
  s21_decimal value_1 = {{-1, -1, -1, 0}};
  s21_decimal value_2 = {{1, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int overflow = s21_add_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(overflow, 1);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
}
END_TEST

START_TEST(test_sub_mantissa_simple) {
  s21_decimal value_1 = {{100, 0, 0, 0}};
  s21_decimal value_2 = {{50, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int borrow = s21_sub_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(borrow, 0);
  ck_assert_int_eq(result.bits[0], 50);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
}
END_TEST

START_TEST(test_sub_mantissa_borrow_to_next_word) {
  s21_decimal value_1 = {{0, 1, 0, 0}};
  s21_decimal value_2 = {{50, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int borrow = s21_sub_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(borrow, 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], 0xFFFFFFCEu);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
}
END_TEST

START_TEST(test_sub_mantissa_underflow) {
  s21_decimal value_1 = {{50, 0, 0, 0}};
  s21_decimal value_2 = {{100, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};

  int borrow = s21_sub_mantissa(value_1, value_2, &result);

  ck_assert_int_eq(borrow, 1);
  ck_assert_uint_eq((uint32_t)result.bits[0], 0xFFFFFFCEu);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0xFFFFFFFFu);
}
END_TEST

START_TEST(test_compare_mantissa) {
  const struct {
    s21_decimal a;
    s21_decimal b;
    int expected;
  } cases[] = {
      {{{0, 0, 0, 0}}, {{0, 0, 0, 0}}, 0},
      {{{-1, -1, -1, 0}}, {{-1, -1, -1, (int)0x801C0000u}}, 0},
      {{{1, 0, 0, 0}}, {{0, 0, 0, 0}}, 1},
      {{{-1, 0, 0, 0}}, {{0x7FFFFFFF, 0, 0, 0}}, 1},
      {{{-1, 0, 0, 0}}, {{0, 1, 0, 0}}, -1},
      {{{-1, -1, 0, 0}}, {{0, 0, 1, 0}}, -1},
      {{{0, -1, 0, 0}}, {{-1, 0x7FFFFFFF, 0, 0}}, 1},
      {{{0, 0, -1, 0}}, {{-1, -1, 0x7FFFFFFF, 0}}, 1},
  };
  int index = _i / 2;
  if (_i % 2 == 0) {
    ck_assert_int_eq(s21_compare_mantissa(cases[index].a, cases[index].b),
                     cases[index].expected);
  } else {
    ck_assert_int_eq(s21_compare_mantissa(cases[index].b, cases[index].a),
                     -cases[index].expected);
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
  tcase_add_test(cases, test_set_scale);
  tcase_add_test(cases, test_add_mantissa_simple);
  tcase_add_test(cases, test_add_mantissa_carry_to_next_word);
  tcase_add_test(cases, test_add_mantissa_overflow);
  tcase_add_test(cases, test_sub_mantissa_simple);
  tcase_add_test(cases, test_sub_mantissa_borrow_to_next_word);
  tcase_add_test(cases, test_sub_mantissa_underflow);
  tcase_add_loop_test(cases, test_compare_mantissa, 0, 16);

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
