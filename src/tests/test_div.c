#include <check.h>
#include <stdint.h>
#include "s21_decimal.h"

static s21_decimal make_decimal(uint32_t bits0, uint32_t bits1,
                                uint32_t bits2, uint32_t scale,
                                uint32_t sign) {
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

/* ============================================================
 * BASIC INTEGER DIVISION
 * ============================================================ */

START_TEST(test_div_integer_exact) {
  s21_decimal value_1 = make_decimal(10, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_integer_with_remainder) {
  s21_decimal value_1 = make_decimal(5, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(25, 0, 0, 1, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_zero_numerator) {
  s21_decimal value_1 = make_decimal(0, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(123, 0, 0, 0, 0);
  s21_decimal result = {{1, 2, 3, 4}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}

START_TEST(test_div_equal_values) {
  s21_decimal value_1 = make_decimal(123456789, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(123456789, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_numerator_less_than_divisor) {
  s21_decimal value_1 = make_decimal(3, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(7, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 4285714285U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 1731114118U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 28U << 16);
}
END_TEST

/* ============================================================
 * SIGNS
 * ============================================================ */

START_TEST(test_div_positive_by_negative) {
  s21_decimal value_1 = make_decimal(10, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 0, 1);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_negative_by_positive) {
  s21_decimal value_1 = make_decimal(10, 0, 0, 0, 1);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 0, 1);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_negative_by_negative) {
  s21_decimal value_1 = make_decimal(10, 0, 0, 0, 1);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

/* ============================================================
 * SCALE
 * ============================================================ */

START_TEST(test_div_fraction) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 1, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_difference_positive) {
  s21_decimal value_1 = make_decimal(123, 0, 0, 1, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(615, 0, 0, 2, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_difference_negative) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 1, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(5, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_negative_normalization) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(100, 0, 0, 2, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_negative_normalization_multiple) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(1000, 0, 0, 3, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(1, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_negative_large) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(1, 0, 0, 28, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(268435456U, 1042612833U, 542101086U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

/* ============================================================
 * REPEATING FRACTIONS
 * ============================================================ */

START_TEST(test_div_one_third) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(3, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(89478485U, 347537611U, 180700362U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_one_sixth) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(6, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(2192222891U, 173768805U, 90350181U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

/* ============================================================
 * MULTI-WORD MANTISSA
 * ============================================================ */

START_TEST(test_div_multiword_exact) {
  /*
   * 2^64 / 2^32 = 2^32
   */
  s21_decimal value_1 = make_decimal(0, 0, 1, 0, 0);
  s21_decimal value_2 = make_decimal(0, 1, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(0, 1, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_multiword_high_bit) {
  /*
   * 2^95 / 2 = 2^94
   */
  s21_decimal value_1 = make_decimal(0, 0, 0x80000000U, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(0, 0, 0x40000000U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_max_by_ffffffff) {
  /*
   * (2^96 - 1) / (2^32 - 1)
   * = 2^64 + 2^32 + 1
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 =
      make_decimal(0xFFFFFFFFU, 0, 0, 0, 0);

  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(1, 1, 1, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_multiword_remainder) {
  s21_decimal value_1 = make_decimal(1, 1, 0, 0, 0);
  s21_decimal value_2 = make_decimal(3, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 1431655767U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 1431655765U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

/* ============================================================
 * ROUNDING TO 28 DIGITS
 * ============================================================ */

START_TEST(test_div_rounding_up) {
  s21_decimal value_1 = make_decimal(5, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(4, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(125, 0, 0, 2, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_exact_with_scales) {
  s21_decimal value_1 = make_decimal(12300, 0, 0, 2, 0);
  s21_decimal value_2 = make_decimal(100, 0, 0, 2, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST


/* ============================================================
 * LARGE REMAINDER / CARRY
 * ============================================================ */


/* ============================================================
 * NEGATIVE SCALE OVERFLOW
 * ============================================================ */


/* ============================================================
 * RESULT UNDERFLOW / ROUND TO ZERO
 * ============================================================ */



/* ============================================================
 * ERRORS
 * ============================================================ */

START_TEST(test_div_zero_divisor) {
  s21_decimal value_1 = make_decimal(123, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(0, 0, 0, 0, 0);
  s21_decimal result = {{123, 456, 789, 111}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 3);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

START_TEST(test_div_null_result) {
  s21_decimal value_1 = make_decimal(10, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, NULL), 1);
}
END_TEST


/* ============================================================
 * SUITE
 * ============================================================ */

Suite *s21_div_suite(void) {
  Suite *suite = suite_create("s21_div");

  TCase *tc_basic = tcase_create("basic");

  tcase_add_test(tc_basic, test_div_integer_exact);
  tcase_add_test(tc_basic, test_div_integer_with_remainder);
  tcase_add_test(tc_basic, test_div_zero_numerator);
  tcase_add_test(tc_basic, test_div_equal_values);
  tcase_add_test(tc_basic, test_div_numerator_less_than_divisor);

  TCase *tc_signs = tcase_create("signs");

  tcase_add_test(tc_signs, test_div_positive_by_negative);
  tcase_add_test(tc_signs, test_div_negative_by_positive);
  tcase_add_test(tc_signs, test_div_negative_by_negative);

  TCase *tc_scale = tcase_create("scale");

  tcase_add_test(tc_scale, test_div_fraction);
  tcase_add_test(tc_scale, test_div_scale_difference_positive);
  tcase_add_test(tc_scale, test_div_scale_difference_negative);
  tcase_add_test(tc_scale, test_div_scale_negative_normalization);
  tcase_add_test(tc_scale, test_div_scale_negative_normalization_multiple);
  tcase_add_test(tc_scale, test_div_scale_negative_large);

  TCase *tc_fraction = tcase_create("fraction");

  tcase_add_test(tc_fraction, test_div_one_third);
  tcase_add_test(tc_fraction, test_div_one_sixth);

  TCase *tc_multiword = tcase_create("multiword");

  tcase_add_test(tc_multiword, test_div_multiword_exact);
  tcase_add_test(tc_multiword, test_div_multiword_high_bit);
  tcase_add_test(tc_multiword, test_div_max_by_ffffffff);
  tcase_add_test(tc_multiword, test_div_multiword_remainder);

  TCase *tc_rounding = tcase_create("rounding");

  tcase_add_test(tc_rounding, test_div_rounding_up);
  tcase_add_test(tc_rounding, test_div_exact_with_scales);

  TCase *tc_carry = tcase_create("carry");

  TCase *tc_overflow = tcase_create("overflow");

  TCase *tc_underflow = tcase_create("underflow");


  TCase *tc_errors = tcase_create("errors");

  tcase_add_test(tc_errors, test_div_zero_divisor);
  tcase_add_test(tc_errors, test_div_null_result);

  suite_add_tcase(suite, tc_basic);
  suite_add_tcase(suite, tc_signs);
  suite_add_tcase(suite, tc_scale);
  suite_add_tcase(suite, tc_fraction);
  suite_add_tcase(suite, tc_multiword);
  suite_add_tcase(suite, tc_rounding);
  suite_add_tcase(suite, tc_carry);
  suite_add_tcase(suite, tc_overflow);
  suite_add_tcase(suite, tc_underflow);
  suite_add_tcase(suite, tc_errors);

  return suite;
}