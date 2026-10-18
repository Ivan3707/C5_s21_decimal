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

static void check_compare(s21_decimal a, s21_decimal b, int expected) {
  ck_assert_int_eq(s21_is_less(a, b), expected < 0);
  ck_assert_int_eq(s21_is_less_or_equal(a, b), expected <= 0);
  ck_assert_int_eq(s21_is_greater(a, b), expected > 0);
  ck_assert_int_eq(s21_is_greater_or_equal(a, b), expected >= 0);
  ck_assert_int_eq(s21_is_equal(a, b), expected == 0);
  ck_assert_int_eq(s21_is_not_equal(a, b), expected != 0);
}

START_TEST(test_compare_zero_signs_equal) {
  check_compare(make_decimal(0, 0, 0, 0, 0), make_decimal(0, 0, 0, 0, 1), 0);
}
END_TEST

START_TEST(test_compare_zero_scales_equal) {
  check_compare(make_decimal(0, 0, 0, 5, 0), make_decimal(0, 0, 0, 0, 1), 0);
}
END_TEST

START_TEST(test_compare_zero_vs_positive) {
  check_compare(make_decimal(0, 0, 0, 0, 0), make_decimal(1, 0, 0, 0, 0), -1);
  check_compare(make_decimal(1, 0, 0, 0, 0), make_decimal(0, 0, 0, 0, 0), 1);
}
END_TEST

START_TEST(test_compare_zero_vs_negative) {
  check_compare(make_decimal(0, 0, 0, 0, 0), make_decimal(1, 0, 0, 0, 1), 1);
  check_compare(make_decimal(1, 0, 0, 0, 1), make_decimal(0, 0, 0, 0, 0), -1);
}
END_TEST

START_TEST(test_compare_different_signs) {
  check_compare(make_decimal(1, 0, 0, 0, 1), make_decimal(1, 0, 0, 0, 0), -1);
  check_compare(make_decimal(1, 0, 0, 0, 0), make_decimal(1, 0, 0, 0, 1), 1);
}
END_TEST

START_TEST(test_compare_positive_order) {
  check_compare(make_decimal(1, 0, 0, 0, 0), make_decimal(2, 0, 0, 0, 0), -1);
  check_compare(make_decimal(2, 0, 0, 0, 0), make_decimal(1, 0, 0, 0, 0), 1);
  check_compare(make_decimal(7, 0, 0, 0, 0), make_decimal(7, 0, 0, 0, 0), 0);
}
END_TEST

START_TEST(test_compare_negative_order) {
  check_compare(make_decimal(2, 0, 0, 0, 1), make_decimal(1, 0, 0, 0, 1), -1);
  check_compare(make_decimal(1, 0, 0, 0, 1), make_decimal(2, 0, 0, 0, 1), 1);
  check_compare(make_decimal(7, 0, 0, 0, 1), make_decimal(7, 0, 0, 0, 1), 0);
}
END_TEST

START_TEST(test_compare_scale_equal_values) {
  check_compare(make_decimal(10, 0, 0, 1, 0), make_decimal(100, 0, 0, 2, 0), 0);
  check_compare(make_decimal(10, 0, 0, 1, 1), make_decimal(100, 0, 0, 2, 1), 0);
}
END_TEST

START_TEST(test_compare_scale_different_values) {
  check_compare(make_decimal(15, 0, 0, 1, 0), make_decimal(149, 0, 0, 2, 0), 1);
  check_compare(make_decimal(149, 0, 0, 2, 0), make_decimal(15, 0, 0, 1, 0),
                -1);
  check_compare(make_decimal(15, 0, 0, 1, 1), make_decimal(149, 0, 0, 2, 1),
                -1);
}
END_TEST

START_TEST(test_compare_scale_28) {
  check_compare(make_decimal(1, 0, 0, 28, 0), make_decimal(1, 0, 0, 1, 0), -1);
  check_compare(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0),
                make_decimal(MAX_U32, MAX_U32, MAX_U32, 28, 0), 1);
  check_compare(make_decimal(MAX_U32, MAX_U32, MAX_U32, 28, 1),
                make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 1), 1);
}
END_TEST

START_TEST(test_compare_multiword_low_to_mid) {
  check_compare(make_decimal(0, 1, 0, 0, 0), make_decimal(MAX_U32, 0, 0, 0, 0),
                1);
}
END_TEST

START_TEST(test_compare_multiword_high_words) {
  check_compare(make_decimal(0, 0, 1, 0, 0), make_decimal(0, 0, 2, 0, 0), -1);
  check_compare(make_decimal(0, 5, 1, 0, 0), make_decimal(0, 4, 1, 0, 0), 1);
}
END_TEST

START_TEST(test_compare_max_values) {
  check_compare(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0),
                make_decimal(0xFFFFFFFEU, MAX_U32, MAX_U32, 0, 0), 1);
  check_compare(make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 1),
                make_decimal(MAX_U32, MAX_U32, MAX_U32, 0, 0), -1);
}
END_TEST

Suite* s21_compare_suite(void) {
  Suite* suite = suite_create("s21_compare");

  TCase* tc_zero = tcase_create("zero");

  tcase_add_test(tc_zero, test_compare_zero_signs_equal);
  tcase_add_test(tc_zero, test_compare_zero_scales_equal);
  tcase_add_test(tc_zero, test_compare_zero_vs_positive);
  tcase_add_test(tc_zero, test_compare_zero_vs_negative);

  TCase* tc_signs = tcase_create("signs");

  tcase_add_test(tc_signs, test_compare_different_signs);
  tcase_add_test(tc_signs, test_compare_positive_order);
  tcase_add_test(tc_signs, test_compare_negative_order);

  TCase* tc_scale = tcase_create("scale");

  tcase_add_test(tc_scale, test_compare_scale_equal_values);
  tcase_add_test(tc_scale, test_compare_scale_different_values);
  tcase_add_test(tc_scale, test_compare_scale_28);

  TCase* tc_multiword = tcase_create("multiword");

  tcase_add_test(tc_multiword, test_compare_multiword_low_to_mid);
  tcase_add_test(tc_multiword, test_compare_multiword_high_words);
  tcase_add_test(tc_multiword, test_compare_max_values);

  suite_add_tcase(suite, tc_zero);
  suite_add_tcase(suite, tc_signs);
  suite_add_tcase(suite, tc_scale);
  suite_add_tcase(suite, tc_multiword);

  return suite;
}