#include <check.h>
#include <stdint.h>

#include "s21_decimal.h"

#define MAX_U32 0xFFFFFFFFU

static s21_decimal make_decimal(uint32_t bits0, uint32_t bits1, uint32_t bits2,
                                uint32_t scale, uint32_t sign) {
  s21_decimal value = {{0, 0, 0, 0}};

  value.bits[0] = (int)bits0;
  value.bits[1] = (int)bits1;
  value.bits[2] = (int)bits2;
  value.bits[3] = (int)((scale & 0xFFU) << 16);

  if (sign != 0) {
    value.bits[3] |= (int)(1U << 31);
  }

  return value;
}

static int decimal_equal(s21_decimal a, s21_decimal b) {
  return (uint32_t)a.bits[0] == (uint32_t)b.bits[0] &&
         (uint32_t)a.bits[1] == (uint32_t)b.bits[1] &&
         (uint32_t)a.bits[2] == (uint32_t)b.bits[2] &&
         (uint32_t)a.bits[3] == (uint32_t)b.bits[3];
}


START_TEST(test_mul_simple) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(6, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(2, 0, 0, 0, 0),
                           make_decimal(3, 0, 0, 0, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_zero) {
  s21_decimal result = {{1, 2, 3, 4}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(0, 0, 0, 0, 0),
                           make_decimal(5, 0, 0, 0, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_by_one) {
  s21_decimal result = {{0}};
  s21_decimal value = make_decimal(123456789, 987654321, 5, 0, 0);

  ck_assert_int_eq(s21_mul(value, make_decimal(1, 0, 0, 0, 0), &result), 0);
  ck_assert_int_eq(decimal_equal(result, value), 1);
}
END_TEST


START_TEST(test_mul_negative_by_positive) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(6, 0, 0, 0, 1);

  ck_assert_int_eq(s21_mul(make_decimal(2, 0, 0, 0, 1),
                           make_decimal(3, 0, 0, 0, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_positive_by_negative) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(6, 0, 0, 0, 1);

  ck_assert_int_eq(s21_mul(make_decimal(2, 0, 0, 0, 0),
                           make_decimal(3, 0, 0, 0, 1), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_negative_by_negative) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(6, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(2, 0, 0, 0, 1),
                           make_decimal(3, 0, 0, 0, 1), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST


START_TEST(test_mul_scales_add) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0, 0, 2, 0);

  ck_assert_int_eq(s21_mul(make_decimal(1, 0, 0, 1, 0),
                           make_decimal(1, 0, 0, 1, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_scales_keep_trailing_zeros) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(300, 0, 0, 2, 0);

  ck_assert_int_eq(s21_mul(make_decimal(15, 0, 0, 1, 0),
                           make_decimal(20, 0, 0, 1, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_scale_28_exact) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(1, 0, 0, 14, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_zero_keeps_scale_sum) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(0, 0, 0, 10, 0);

  ck_assert_int_eq(s21_mul(make_decimal(0, 0, 0, 5, 0),
                           make_decimal(0, 0, 0, 5, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST


START_TEST(test_mul_multiword_product) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0xFFFFFFFEU, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(MAX_U32, 0, 0, 0, 0),
                           make_decimal(MAX_U32, 0, 0, 0, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_carry_into_high_word) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(0, 0, 0x80000000U, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(0, 0, 1, 0, 0),
                           make_decimal(0x80000000U, 0, 0, 0, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_max_by_one) {
  s21_decimal result = {{0}};
  s21_decimal max = make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0);

  ck_assert_int_eq(s21_mul(max, make_decimal(1, 0, 0, 0, 0), &result), 0);
  ck_assert_int_eq(decimal_equal(result, max), 1);
}
END_TEST

START_TEST(test_mul_max_by_one_with_trailing_zero) {
  s21_decimal result = {{0}};
  s21_decimal max = make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0);

  ck_assert_int_eq(s21_mul(max, make_decimal(10, 0, 0, 1, 0), &result), 0);
  ck_assert_int_eq(decimal_equal(result, max), 1);
}
END_TEST



START_TEST(test_mul_rounding_max_by_half) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(0, 0, 0x80000000U, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0),
                           make_decimal(5, 0, 0, 1, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_half_to_even_up) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(15, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_half_to_even_down) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(25, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_above_half) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(16, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_below_half) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(24, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_sticky_digit_up) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(3, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(251, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 15, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_exact_half_two_digits) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 0);

  ck_assert_int_eq(s21_mul(make_decimal(250, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 15, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_negative) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(2, 0, 0, 28, 1);

  ck_assert_int_eq(s21_mul(make_decimal(15, 0, 0, 15, 1),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_rounding_reduces_large_scale) {
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(15, 0, 0, 28, 0),
                           make_decimal(15, 0, 0, 28, 0), &result),
                   2);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST



START_TEST(test_mul_overflow_positive) {
  s21_decimal result = {{123, 456, 789, 111}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0),
                           make_decimal(2, 0, 0, 0, 0), &result),
                   1);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_overflow_negative) {
  s21_decimal result = {{123, 456, 789, 111}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 1),
                           make_decimal(2, 0, 0, 0, 0), &result),
                   2);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_overflow_negative_by_negative) {
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_mul(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 1),
                           make_decimal(2, 0, 0, 0, 1), &result),
                   1);
}
END_TEST

START_TEST(test_mul_overflow_max_by_max) {
  s21_decimal result = {{0}};
  s21_decimal max = make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0);

  ck_assert_int_eq(s21_mul(max, max, &result), 1);
}
END_TEST

START_TEST(test_mul_underflow_positive) {
  s21_decimal result = {{123, 456, 789, 111}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_mul(make_decimal(1, 0, 0, 20, 0),
                           make_decimal(1, 0, 0, 20, 0), &result),
                   2);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_mul_underflow_half_to_even_zero) {
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_mul(make_decimal(5, 0, 0, 15, 0),
                           make_decimal(1, 0, 0, 14, 0), &result),
                   2);
}
END_TEST



START_TEST(test_mul_null_result) {
  ck_assert_int_eq(s21_mul(make_decimal(1, 0, 0, 0, 0),
                           make_decimal(1, 0, 0, 0, 0), NULL),
                   1);
}
END_TEST


Suite* s21_mul_suite(void) {
  Suite* suite = suite_create("s21_mul");

  TCase* tc_basic = tcase_create("basic");

  tcase_add_test(tc_basic, test_mul_simple);
  tcase_add_test(tc_basic, test_mul_zero);
  tcase_add_test(tc_basic, test_mul_by_one);

  TCase* tc_signs = tcase_create("signs");

  tcase_add_test(tc_signs, test_mul_negative_by_positive);
  tcase_add_test(tc_signs, test_mul_positive_by_negative);
  tcase_add_test(tc_signs, test_mul_negative_by_negative);

  TCase* tc_scale = tcase_create("scale");

  tcase_add_test(tc_scale, test_mul_scales_add);
  tcase_add_test(tc_scale, test_mul_scales_keep_trailing_zeros);
  tcase_add_test(tc_scale, test_mul_scale_28_exact);
  tcase_add_test(tc_scale, test_mul_zero_keeps_scale_sum);

  TCase* tc_multiword = tcase_create("multiword");

  tcase_add_test(tc_multiword, test_mul_multiword_product);
  tcase_add_test(tc_multiword, test_mul_carry_into_high_word);
  tcase_add_test(tc_multiword, test_mul_max_by_one);
  tcase_add_test(tc_multiword, test_mul_max_by_one_with_trailing_zero);

  TCase* tc_rounding = tcase_create("rounding");

  tcase_add_test(tc_rounding, test_mul_rounding_max_by_half);
  tcase_add_test(tc_rounding, test_mul_rounding_half_to_even_up);
  tcase_add_test(tc_rounding, test_mul_rounding_half_to_even_down);
  tcase_add_test(tc_rounding, test_mul_rounding_above_half);
  tcase_add_test(tc_rounding, test_mul_rounding_below_half);
  tcase_add_test(tc_rounding, test_mul_rounding_sticky_digit_up);
  tcase_add_test(tc_rounding, test_mul_rounding_exact_half_two_digits);
  tcase_add_test(tc_rounding, test_mul_rounding_negative);
  tcase_add_test(tc_rounding, test_mul_rounding_reduces_large_scale);

  TCase* tc_overflow = tcase_create("overflow");

  tcase_add_test(tc_overflow, test_mul_overflow_positive);
  tcase_add_test(tc_overflow, test_mul_overflow_negative);
  tcase_add_test(tc_overflow, test_mul_overflow_negative_by_negative);
  tcase_add_test(tc_overflow, test_mul_overflow_max_by_max);

  TCase* tc_underflow = tcase_create("underflow");

  tcase_add_test(tc_underflow, test_mul_underflow_positive);
  tcase_add_test(tc_underflow, test_mul_underflow_half_to_even_zero);

  TCase* tc_errors = tcase_create("errors");

  tcase_add_test(tc_errors, test_mul_null_result);

  suite_add_tcase(suite, tc_basic);
  suite_add_tcase(suite, tc_signs);
  suite_add_tcase(suite, tc_scale);
  suite_add_tcase(suite, tc_multiword);
  suite_add_tcase(suite, tc_rounding);
  suite_add_tcase(suite, tc_overflow);
  suite_add_tcase(suite, tc_underflow);
  suite_add_tcase(suite, tc_errors);

  return suite;
}