#include <check.h>
#include <stdint.h>
#include <stdio.h>

#include "s21_decimal.h"

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
END_TEST

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

  s21_decimal expected =
      make_decimal(3182877550U, 4128234610U, 232329036U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_max_by_one) {
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);
  s21_decimal value_2 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, value_1), 1);
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

START_TEST(test_div_negative_fraction) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 1);
  s21_decimal value_2 = make_decimal(6, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(2192222891U, 173768805U, 90350181U, 28, 1);

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

START_TEST(test_div_scale_28_preserved) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 28, 0);
  s21_decimal value_2 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(1, 0, 0, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_scale_normalization_with_nonzero_integer_part) {
  s21_decimal value_1 = make_decimal(1234, 0, 0, 2, 0);
  s21_decimal value_2 = make_decimal(1, 0, 0, 3, 0);
  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(12340, 0, 0, 0, 0);

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

  s21_decimal expected = make_decimal(89478485U, 347537611U, 180700362U, 28, 0);

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

  s21_decimal value_2 = make_decimal(0xFFFFFFFFU, 0, 0, 0, 0);

  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(1, 1, 1, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_multiword_remainder) {
  /*
   * (2^33 + 3) / (2^32 + 1)
   *
   * Exact value:
   *   2 + 1 / (2^32 + 1)
   *
   * The result is represented with scale 28 and rounded
   * according to round-to-even.
   */
  s21_decimal value_1 = make_decimal(3, 2, 0, 0, 0);
  s21_decimal value_2 = make_decimal(1, 1, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(1037382659U, 2627326752U, 1084202172U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_multiword_fraction_with_rounding) {
  /*
   * (2^32 + 1) * 2 + 1
   * -------------------
   *       2^32 + 1
   *
   * = 2 + 1 / (2^32 + 1)
   */
  s21_decimal value_1 = make_decimal(3, 2, 0, 0, 0);
  s21_decimal value_2 = make_decimal(1, 1, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(1037382659U, 2627326752U, 1084202172U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

/* ============================================================
 * ROUNDING
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

START_TEST(test_div_rounding_at_28_digits_up) {
  /*
   * 1 / 6 = 0.16666...
   *
   * At scale 28 the remaining part is greater than half,
   * therefore the last digit must be increased.
   */
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);
  s21_decimal value_2 = make_decimal(6, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(2192222891U, 173768805U, 90350181U, 28, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_compare_mid_low_paths) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;
  /* mid > */
  value_1 = make_decimal(2U, 0U, 0U, 0, 0);
  value_2 = make_decimal(0xFFFFFFFFU, 0U, 0U, 0, 0);
  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  expected = make_decimal(3169427839U, 1084202172U, 0U, 28, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);

  /* mid < */
  value_1 = make_decimal(78576U, 0U, 0U, 0, 0);
  value_2 = make_decimal(0xA2E1D2D5U, 0x16F5E9A3U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(2550621021U, 110577U, 0U, 28, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);

  /* low > */
  value_1 = make_decimal(1U, 0U, 0U, 0, 0);
  value_2 = make_decimal(1000000007U, 0U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(1033160170U, 2328306420U, 0U, 28, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);

  /* low < */
  value_1 = make_decimal(1U, 0U, 0U, 0, 0);
  value_2 = make_decimal(0xFFFFFFFFU, 0U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(1584713919U, 542101086U, 0U, 28, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);

  /* comparison == 0 */
  value_1 = make_decimal(1U, 0U, 0U, 0, 0);
  value_2 = make_decimal(0x20000000U, 0U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(4045935368U, 41841393U, 1U, 28, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_overflow_bankers) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(0xB8B6F23CU, 0x5B6E0E1DU, 0xE59B0E31U, 0, 0);
  value_2 = make_decimal(0xD7D7D7D7U, 0xE7B4D6E2U, 0x01D3A0C5U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(1867796769U, 2459397242U, 681400353U, 26, 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_round_overflow_half_with_remainder) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(858993466U, 858993459U, 3006477107U, 0, 0);

  value_2 = make_decimal(7U, 0U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(2576980379U, 2576980377U, 429496729U, 0, 0);

  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_high_carry) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(4068907032U, 3164219291U, 3257025563U, 0, 0);

  value_2 = make_decimal(921896062U, 3582522800U, 3426189145U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(1072170700U, 927794662U, 515335558U, 28, 0);

  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_high_greater) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(685156112U, 1690653720U, 428204268U, 0, 0);

  value_2 = make_decimal(3115688420U, 3589912389U, 2880075826U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(2717732188U, 201746259U, 80598572U, 28, 0);

  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_mid_less) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(1153625709U, 1922022182U, 0U, 0, 0);

  value_2 = make_decimal(2708297204U, 2023281913U, 0U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(4187598476U, 486359200U, 514970408U, 28, 0);

  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_rounding_overflow_max_bankers) {
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  s21_decimal expected;

  value_1 = make_decimal(4294967288U, 4294967295U, 4294967295U, 0, 0);

  value_2 = make_decimal(268435455U, 1042612833U, 542101086U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  expected = make_decimal(2576980378U, 2576980377U, 429496729U, 27, 0);

  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

/* ============================================================
 * LARGE REMAINDER / CARRY
 * ============================================================ */

START_TEST(test_div_remainder_multiply_by_10_carry) {
  /*
   * value_1 = 3 * 2^94
   * value_2 = 2^95
   *
   * value_1 / value_2 = 1.5
   *
   * During the first fractional iteration:
   * remainder * 10 exceeds 96 bits, so remainder_high != 0.
   */
  s21_decimal value_1 = make_decimal(0, 0, 0xC0000000U, 0, 0);

  s21_decimal value_2 = make_decimal(0, 0, 0x80000000U, 0, 0);

  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(15, 0, 0, 1, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_remainder_high_carry_multiple_subtractions) {
  /*
   * Same carry path, but the high part of the temporary
   * remainder requires several subtractions of the divisor.
   *
   * 3 * 2^94 / 2^95 = 1.5
   */
  s21_decimal value_1 = make_decimal(0, 0, 0xC0000000U, 0, 0);

  s21_decimal value_2 = make_decimal(0, 0, 0x80000000U, 0, 0);

  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 15U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 1U << 16);
}
END_TEST

/* ============================================================
 * MANTISSA OVERFLOW DURING FRACTION GENERATION
 * ============================================================ */

START_TEST(test_div_max_precision_overflow_round_up) {
  /*
   * (2^96 - 1) / 101
   *
   * The 29th significant digit cannot be stored in 96 bits.
   * The result must be rounded while preserving scale 2.
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 = make_decimal(101, 0, 0, 0, 0);

  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(935537430U, 1403306146U, 4252442867U, 2, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_div_max_precision_overflow_without_rounding) {
  /*
   * (2^96 - 1) / 103
   *
   * The next digit is below the rounding threshold.
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 = make_decimal(103, 0, 0, 0, 0);

  s21_decimal result = {{0}};

  s21_decimal expected =
      make_decimal(1709647175U, 708878097U, 4169871161U, 2, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}

/* ============================================================
 * NEGATIVE SCALE OVERFLOW
 * ============================================================ */

START_TEST(test_div_negative_scale_overflow_positive) {
  /*
   * (2^96 - 1) / 0.1
   *
   * Result = (2^96 - 1) * 10, which does not fit into
   * a 96-bit mantissa.
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 = make_decimal(1, 0, 0, 1, 0);

  s21_decimal result = {{123, 456, 789, 111}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 1);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

START_TEST(test_div_negative_scale_overflow_negative) {
  /*
   * Same overflow for a negative result.
   *
   * According to the decimal API convention, negative overflow
   * returns 2.
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 = make_decimal(1, 0, 0, 1, 1);

  s21_decimal result = {{123, 456, 789, 111}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 2);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

/* ============================================================
 * RESULT UNDERFLOW / ROUND TO ZERO
 * ============================================================ */

START_TEST(test_div_result_underflow_to_zero) {
  /*
   * 1 / (2^96 - 1)
   *
   * The value is smaller than 0.5 * 10^-28, therefore
   * after limiting the scale to 28 digits it rounds to zero.
   */
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 0);

  s21_decimal value_2 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal result = {{123, 456, 789, 111}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 2);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

START_TEST(test_div_negative_result_underflow_to_zero) {
  s21_decimal value_1 = make_decimal(1, 0, 0, 0, 1);

  s21_decimal value_2 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal result = {{123, 456, 789, 111}};

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 2);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0U);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0U);
}
END_TEST

/* ============================================================
 * MAXIMUM REPRESENTABLE VALUES
 * ============================================================ */

START_TEST(test_div_max_by_two) {
  /*
   * (2^96 - 1) / 2
   *
   * = 39614081257132168796771975167.5
   *
   * Round-to-even gives 2^95.
   */
  s21_decimal value_1 =
      make_decimal(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0, 0);

  s21_decimal value_2 = make_decimal(2, 0, 0, 0, 0);

  s21_decimal result = {{0}};

  s21_decimal expected = make_decimal(0, 0, 0x80000000U, 0, 0);

  ck_assert_int_eq(s21_div(value_1, value_2, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

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

Suite* s21_div_suite(void) {
  Suite* suite = suite_create("s21_div");

  TCase* tc_basic = tcase_create("basic");

  tcase_add_test(tc_basic, test_div_integer_exact);
  tcase_add_test(tc_basic, test_div_integer_with_remainder);
  tcase_add_test(tc_basic, test_div_zero_numerator);
  tcase_add_test(tc_basic, test_div_equal_values);
  tcase_add_test(tc_basic, test_div_numerator_less_than_divisor);
  tcase_add_test(tc_basic, test_div_max_by_one);

  TCase* tc_signs = tcase_create("signs");

  tcase_add_test(tc_signs, test_div_positive_by_negative);
  tcase_add_test(tc_signs, test_div_negative_by_positive);
  tcase_add_test(tc_signs, test_div_negative_by_negative);
  tcase_add_test(tc_signs, test_div_negative_fraction);

  TCase* tc_scale = tcase_create("scale");

  tcase_add_test(tc_scale, test_div_fraction);
  tcase_add_test(tc_scale, test_div_scale_difference_positive);
  tcase_add_test(tc_scale, test_div_scale_difference_negative);
  tcase_add_test(tc_scale, test_div_scale_negative_normalization);
  tcase_add_test(tc_scale, test_div_scale_negative_normalization_multiple);
  tcase_add_test(tc_scale, test_div_scale_negative_large);
  tcase_add_test(tc_scale, test_div_scale_28_preserved);
  tcase_add_test(tc_scale,
                 test_div_scale_normalization_with_nonzero_integer_part);

  TCase* tc_fraction = tcase_create("fraction");

  tcase_add_test(tc_fraction, test_div_one_third);
  tcase_add_test(tc_fraction, test_div_one_sixth);

  TCase* tc_multiword = tcase_create("multiword");

  tcase_add_test(tc_multiword, test_div_multiword_exact);
  tcase_add_test(tc_multiword, test_div_multiword_high_bit);
  tcase_add_test(tc_multiword, test_div_max_by_ffffffff);
  tcase_add_test(tc_multiword, test_div_multiword_remainder);
  tcase_add_test(tc_multiword, test_div_multiword_fraction_with_rounding);

  TCase* tc_rounding = tcase_create("rounding");

  tcase_add_test(tc_rounding, test_div_rounding_up);
  tcase_add_test(tc_rounding, test_div_exact_with_scales);
  tcase_add_test(tc_rounding, test_div_rounding_at_28_digits_up);
  tcase_add_test(tc_rounding, test_div_rounding_overflow_bankers);
  tcase_add_test(tc_rounding, test_div_rounding_compare_mid_low_paths);

  tcase_add_test(tc_rounding, test_div_round_overflow_half_with_remainder);
  tcase_add_test(tc_rounding, test_div_rounding_high_carry);
  tcase_add_test(tc_rounding, test_div_rounding_high_greater);
  tcase_add_test(tc_rounding, test_div_rounding_mid_less);
  tcase_add_test(tc_rounding, test_div_rounding_overflow_max_bankers);

  TCase* tc_carry = tcase_create("carry");

  tcase_add_test(tc_carry, test_div_remainder_multiply_by_10_carry);
  tcase_add_test(tc_carry, test_div_remainder_high_carry_multiple_subtractions);

  TCase* tc_overflow = tcase_create("overflow");

  tcase_add_test(tc_overflow, test_div_max_precision_overflow_round_up);
  tcase_add_test(tc_overflow, test_div_max_precision_overflow_without_rounding);
  tcase_add_test(tc_overflow, test_div_negative_scale_overflow_positive);
  tcase_add_test(tc_overflow, test_div_negative_scale_overflow_negative);

  TCase* tc_underflow = tcase_create("underflow");

  tcase_add_test(tc_underflow, test_div_result_underflow_to_zero);
  tcase_add_test(tc_underflow, test_div_negative_result_underflow_to_zero);

  TCase* tc_maximum = tcase_create("maximum");

  tcase_add_test(tc_maximum, test_div_max_by_two);

  TCase* tc_errors = tcase_create("errors");

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
  suite_add_tcase(suite, tc_maximum);
  suite_add_tcase(suite, tc_errors);

  return suite;
}