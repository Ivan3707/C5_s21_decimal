#include <check.h>
#include <stdlib.h>

#include "../s21_decimal.h"

START_TEST(test_add_same_scale) {
  int sign = _i % 2;
  int scales[] = {0, 2, 28};
  int scale = scales[_i / 2];
  s21_decimal a = {{125, 0, 0, 0}};
  s21_decimal b = {{230, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = b.bits[3] =
      (int)(((uint32_t)sign << 31) | ((uint32_t)scale << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_int_eq(result.bits[0], 355);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_uint_eq((uint32_t)result.bits[3], (uint32_t)a.bits[3]);
}
END_TEST

START_TEST(test_add_carry_through_words) {
  s21_decimal a = {{-1, -1, 0, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = b.bits[3] = (int)((uint32_t)_i << 31);

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 1);
  ck_assert_uint_eq((uint32_t)result.bits[3], (uint32_t)a.bits[3]);
}
END_TEST

START_TEST(test_add_overflow_without_scale) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {{0, 0, 0, 0}};
  a.bits[3] = b.bits[3] = (int)((uint32_t)_i << 31);

  ck_assert_int_eq(s21_add(a, b, &result), _i ? 2 : 1);
}
END_TEST

START_TEST(test_add_overflow_bankers_rounding) {
  const struct {
    uint32_t extra;
    uint32_t expected[3];
  } cases[] = {
      {1u, {0x9999999Au, 0x99999999u, 0x19999999u}},
      {4u, {0x9999999Au, 0x99999999u, 0x19999999u}},
      {9u, {0x9999999Au, 0x99999999u, 0x19999999u}},
      {10u, {0x9999999Au, 0x99999999u, 0x19999999u}},
      {20u, {0x9999999Cu, 0x99999999u, 0x19999999u}},
  };
  int index = _i / 4;
  int sign = _i % 2;
  int scale = (_i / 2) % 2 ? 28 : 1;
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{(int)cases[index].extra, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = b.bits[3] =
      (int)(((uint32_t)sign << 31) | ((uint32_t)scale << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  for (int i = 0; i < 3; i++) {
    ck_assert_uint_eq((uint32_t)result.bits[i], cases[index].expected[i]);
  }
  uint32_t expected_flags =
      ((uint32_t)sign << 31) | ((uint32_t)(scale - 1) << 16);
  ck_assert_uint_eq((uint32_t)result.bits[3], expected_flags);
}
END_TEST

START_TEST(test_add_result_aliases_input) {
  s21_decimal a = {{125, 0, 0, 0}};
  s21_decimal b = {{230, 0, 0, 0}};
  a.bits[3] = b.bits[3] = (int)((2u << 16) | ((uint32_t)_i << 31));
  uint32_t expected_flags = (uint32_t)a.bits[3];

  ck_assert_int_eq(s21_add(a, b, &a), 0);
  ck_assert_int_eq(a.bits[0], 355);
  ck_assert_int_eq(a.bits[1], 0);
  ck_assert_int_eq(a.bits[2], 0);
  ck_assert_uint_eq((uint32_t)a.bits[3], expected_flags);
}
END_TEST

START_TEST(test_add_different_signs_same_scale) {
  const struct {
    int coefficient_1;
    int sign_1;
    int coefficient_2;
    int sign_2;
    int expected_coefficient;
    int expected_sign;
  } cases[] = {
      {9, 0, 4, 1, 5, 0}, {9, 1, 4, 0, 5, 1}, {4, 0, 9, 1, 5, 1},
      {4, 1, 9, 0, 5, 0}, {9, 0, 9, 1, 0, 0}, {9, 1, 9, 0, 0, 0},
  };
  int index = _i;
  int scale = 2;
  s21_decimal a = {{cases[index].coefficient_1, 0, 0, 0}};
  s21_decimal b = {{cases[index].coefficient_2, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] =
      (int)(((uint32_t)cases[index].sign_1 << 31) | ((uint32_t)scale << 16));
  b.bits[3] =
      (int)(((uint32_t)cases[index].sign_2 << 31) | ((uint32_t)scale << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_int_eq(result.bits[0], cases[index].expected_coefficient);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(s21_get_sign(result), cases[index].expected_sign);
  ck_assert_int_eq(s21_get_scale(result),
                   cases[index].expected_coefficient == 0 ? 0 : scale);
}
END_TEST

START_TEST(test_add_different_signs_compares_high_words) {
  s21_decimal a = {{0, 0, 1, 0}};
  s21_decimal b = {{-1, -1, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = (int)(2u << 16);
  b.bits[3] = (int)((1u << 31) | (2u << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], 1u);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0u);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(result), 0);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

START_TEST(test_add_different_scales) {
  const struct {
    int coefficient_1;
    int scale_1;
    int sign_1;
    int coefficient_2;
    int scale_2;
    int sign_2;
    int expected_coefficient;
    int expected_sign;
  } cases[] = {
      {12, 1, 0, 3, 2, 0, 123, 0},
      {12, 1, 0, 3, 2, 1, 117, 0},
      {12, 1, 1, 3, 2, 0, 117, 1},
      {3, 2, 0, 12, 1, 1, 117, 1},
  };
  const int index = _i;
  s21_decimal a = {{cases[index].coefficient_1, 0, 0, 0}};
  s21_decimal b = {{cases[index].coefficient_2, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = (int)(((uint32_t)cases[index].sign_1 << 31) |
                    ((uint32_t)cases[index].scale_1 << 16));
  b.bits[3] = (int)(((uint32_t)cases[index].sign_2 << 31) |
                    ((uint32_t)cases[index].scale_2 << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_int_eq(result.bits[0], cases[index].expected_coefficient);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(s21_get_sign(result), cases[index].expected_sign);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

START_TEST(test_add_different_scales_reduces_big_exactly) {
  const int sign = _i;
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {{7, 8, 9, -1}};
  a.bits[3] = (int)((uint32_t)sign << 31);
  b.bits[3] = (int)(((uint32_t)sign << 31) | ((uint32_t)28u << 16));

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], UINT32_MAX);
  ck_assert_uint_eq((uint32_t)result.bits[1], UINT32_MAX);
  ck_assert_uint_eq((uint32_t)result.bits[2], UINT32_MAX);
  ck_assert_int_eq(s21_get_sign(result), sign);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_add_different_scales_bankers_rounding) {
  const uint32_t fractions[] = {4u, 5u, 6u};
  const uint32_t expected_low[] = {UINT32_MAX, UINT32_MAX - 1u,
                                   UINT32_MAX - 1u};
  int index = _i / 2;
  int sign = _i % 2;
  s21_decimal a = {{-1, -1, -1, (int)((uint32_t)sign << 31)}};
  s21_decimal b = {{(int)fractions[index], 0, 0,
                    (int)(((uint32_t)(!sign) << 31) | (1u << 16))}};
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_uint_eq((uint32_t)result.bits[0], expected_low[index]);
  ck_assert_uint_eq((uint32_t)result.bits[1], UINT32_MAX);
  ck_assert_uint_eq((uint32_t)result.bits[2], UINT32_MAX);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), sign);
}
END_TEST

START_TEST(test_add_opposites_with_different_scales_are_zero) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, (int)((1u << 31) | (1u << 16))}};
  s21_decimal result = {{7, 8, 9, -1}};

  ck_assert_int_eq(s21_add(a, b, &result), 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(result.bits[3], 0);
}
END_TEST

START_TEST(test_add_null_result) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};

  ck_assert_int_eq(s21_add(a, b, NULL), 1);
}
END_TEST

Suite* s21_add_suite(void) {
  Suite* suite = suite_create("s21_add");
  TCase* cases = tcase_create("implemented cases");
  tcase_add_loop_test(cases, test_add_same_scale, 0, 6);
  tcase_add_loop_test(cases, test_add_carry_through_words, 0, 2);
  tcase_add_loop_test(cases, test_add_overflow_without_scale, 0, 2);
  tcase_add_loop_test(cases, test_add_overflow_bankers_rounding, 0, 20);
  tcase_add_loop_test(cases, test_add_result_aliases_input, 0, 2);
  tcase_add_loop_test(cases, test_add_different_signs_same_scale, 0, 6);
  tcase_add_test(cases, test_add_different_signs_compares_high_words);
  tcase_add_loop_test(cases, test_add_different_scales, 0, 4);
  tcase_add_loop_test(cases, test_add_different_scales_reduces_big_exactly, 0,
                      2);
  tcase_add_loop_test(cases, test_add_different_scales_bankers_rounding, 0, 6);
  tcase_add_test(cases, test_add_opposites_with_different_scales_are_zero);
  tcase_add_test(cases, test_add_null_result);
  suite_add_tcase(suite, cases);
  return suite;
}

#ifdef S21_ADD_TEST_MAIN
int main(void) {
  SRunner* runner = srunner_create(s21_add_suite());
  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return failed == 0 ? 0 : 1;
}
#endif
