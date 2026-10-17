#include <check.h>
#include <stdlib.h>

#include "../s21_decimal.h"

static s21_decimal make_decimal(uint32_t low, uint32_t middle, uint32_t high,
                                int scale, int sign) {
  s21_decimal value = {{(int)low, (int)middle, (int)high, 0}};
  value.bits[3] = (int)(((uint32_t)sign << 31) | ((uint32_t)scale << 16));
  return value;
}

START_TEST(test_sub_sign_combinations) {
  const struct {
    uint32_t coefficient_1;
    int sign_1;
    uint32_t coefficient_2;
    int sign_2;
    uint32_t expected_coefficient;
    int expected_sign;
  } cases[] = {
      {9, 0, 4, 0, 5, 0},  {4, 0, 9, 0, 5, 1}, {9, 1, 4, 0, 13, 1},
      {9, 0, 4, 1, 13, 0}, {9, 1, 4, 1, 5, 1}, {4, 1, 9, 1, 5, 0},
  };
  int index = _i;
  s21_decimal value_1 =
      make_decimal(cases[index].coefficient_1, 0, 0, 0, cases[index].sign_1);
  s21_decimal value_2 =
      make_decimal(cases[index].coefficient_2, 0, 0, 0, cases[index].sign_2);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0],
                    cases[index].expected_coefficient);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0);
  ck_assert_int_eq(s21_get_sign(result), cases[index].expected_sign);
}
END_TEST

START_TEST(test_sub_different_scales) {
  const struct {
    int sign_1;
    int sign_2;
    uint32_t expected_coefficient;
    int expected_sign;
  } cases[] = {{0, 0, 117, 0}, {1, 1, 117, 1}, {0, 1, 123, 0}, {1, 0, 123, 1}};
  int index = _i;
  s21_decimal value_1 = make_decimal(12, 0, 0, 1, cases[index].sign_1);
  s21_decimal value_2 = make_decimal(3, 0, 0, 2, cases[index].sign_2);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0],
                    cases[index].expected_coefficient);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), cases[index].expected_sign);
}
END_TEST

START_TEST(test_sub_zero_and_equal_values) {
  s21_decimal value_1 = make_decimal(_i == 1 ? 0 : 5, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(_i == 2 ? 0 : 5, 0, 0, 0, 0);
  s21_decimal result = {{7, 8, 9, -1}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), 0);
  if (_i == 0) {
    ck_assert_uint_eq((uint32_t)result.bits[0], 0);
    ck_assert_uint_eq((uint32_t)result.bits[3], 0);
  } else {
    ck_assert_uint_eq((uint32_t)result.bits[0], 5);
    ck_assert_int_eq(s21_get_sign(result), _i == 1);
  }
}
END_TEST

START_TEST(test_sub_preserves_scale) {
  s21_decimal value_1 = make_decimal(500, 0, 0, 2, 0);
  s21_decimal value_2 = make_decimal(200, 0, 0, 2, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], 300);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_overflow) {
  int sign = _i;
  s21_decimal value_1 =
      make_decimal(UINT32_MAX, UINT32_MAX, UINT32_MAX, 0, sign);
  s21_decimal value_2 = make_decimal(1, 0, 0, 0, !sign);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), sign ? 2 : 1);
}
END_TEST

START_TEST(test_sub_bankers_rounding) {
  const uint32_t fractions[] = {4u, 5u, 6u};
  const uint32_t expected_low[] = {UINT32_MAX, UINT32_MAX - 1u,
                                   UINT32_MAX - 1u};
  int index = _i;
  s21_decimal value_1 = make_decimal(UINT32_MAX, UINT32_MAX, UINT32_MAX, 0, 0);
  s21_decimal value_2 = make_decimal(fractions[index], 0, 0, 1, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_sub(value_1, value_2, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], expected_low[index]);
  ck_assert_uint_eq((uint32_t)result.bits[1], UINT32_MAX);
  ck_assert_uint_eq((uint32_t)result.bits[2], UINT32_MAX);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_result_aliases_input) {
  s21_decimal value_1 = make_decimal(900, 0, 0, 2, 0);
  s21_decimal value_2 = make_decimal(400, 0, 0, 2, 0);
  s21_decimal* result = _i == 0 ? &value_1 : &value_2;

  ck_assert_int_eq(s21_sub(value_1, value_2, result), 0);
  ck_assert_uint_eq((uint32_t)result->bits[0], 500);
  ck_assert_int_eq(s21_get_scale(*result), 2);
  ck_assert_int_eq(s21_get_sign(*result), 0);
}
END_TEST

START_TEST(test_sub_null_result) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);

  ck_assert_int_eq(s21_sub(value_1, value_2, NULL), 1);
}
END_TEST

Suite* s21_sub_suite(void) {
  Suite* suite = suite_create("s21_sub");
  TCase* cases = tcase_create("subtraction cases");
  tcase_add_loop_test(cases, test_sub_sign_combinations, 0, 6);
  tcase_add_loop_test(cases, test_sub_different_scales, 0, 4);
  tcase_add_loop_test(cases, test_sub_zero_and_equal_values, 0, 3);
  tcase_add_test(cases, test_sub_preserves_scale);
  tcase_add_loop_test(cases, test_sub_overflow, 0, 2);
  tcase_add_loop_test(cases, test_sub_bankers_rounding, 0, 3);
  tcase_add_loop_test(cases, test_sub_result_aliases_input, 0, 2);
  tcase_add_test(cases, test_sub_null_result);
  suite_add_tcase(suite, cases);
  return suite;
}
