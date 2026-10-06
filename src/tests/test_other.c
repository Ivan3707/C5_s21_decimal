#include <check.h>
#include "s21_decimal.h"

static s21_decimal make_decimal(uint32_t bits0, uint32_t bits1,
                                uint32_t bits2, uint32_t scale,
                                uint32_t sign) {
  s21_decimal value = {{0, 0, 0, 0}};

  value.bits[0] = bits0;
  value.bits[1] = bits1;
  value.bits[2] = bits2;
  value.bits[3] = (int)((scale & 0xFF) << 16);

  if (sign != 0) {
    value.bits[3] |= (int)(1U << 31);
  }

  return value;
}

static int decimal_equal(s21_decimal a, s21_decimal b) {
  return a.bits[0] == b.bits[0] &&
         a.bits[1] == b.bits[1] &&
         a.bits[2] == b.bits[2] &&
         a.bits[3] == b.bits[3];
}

/* ==================== truncate ==================== */

START_TEST(test_truncate_positive) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 0);

  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_truncate_negative) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 1);

  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_truncate_without_fraction) {
  s21_decimal value = make_decimal(12345, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12345);
  ck_assert_int_eq(result.bits[3], 0);
}
END_TEST

START_TEST(test_truncate_zero) {
  s21_decimal value = make_decimal(0, 0, 0, 5, 0);
  s21_decimal result = {{1, 2, 3, 4}};
  s21_decimal expected = make_decimal(0, 0, 0, 0, 0);

  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_truncate_null) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);

  ck_assert_int_eq(s21_truncate(value, NULL), 1);
}
END_TEST

START_TEST(test_truncate_large_mantissa) {
  s21_decimal value = make_decimal(30, 20, 10, 1, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(3, 2, 1, 0, 0);

  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_truncate_multiword) {
  s21_decimal value = make_decimal(0, 1, 0, 1, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_truncate(value, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0x19999999);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0);
}
END_TEST

/* ==================== floor ==================== */

START_TEST(test_floor_positive_fraction) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 0);

  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_floor_negative_fraction) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(124, 0, 0, 0, 1);

  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_floor_negative_integer) {
  s21_decimal value = make_decimal(12300, 0, 0, 2, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 1);

  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_floor_positive_integer) {
  s21_decimal value = make_decimal(12300, 0, 0, 2, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 123);
  ck_assert_int_eq(result.bits[3], 0);
}
END_TEST

START_TEST(test_floor_null) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);

  ck_assert_int_eq(s21_floor(value, NULL), 1);
}
END_TEST

START_TEST(test_floor_negative_multiword_fraction) {
  s21_decimal value = make_decimal(1, 1, 0, 1, 1);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_floor(value, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0x1999999A);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0x80000000);
}
END_TEST

/* ==================== negate ==================== */

START_TEST(test_negate_positive) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(12345, 0, 0, 2, 1);

  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_negate_negative) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(12345, 0, 0, 2, 0);

  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_negate_zero) {
  s21_decimal value = make_decimal(0, 0, 0, 0, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq((uint32_t)result.bits[3] >> 31, 1);
}
END_TEST

START_TEST(test_negate_preserves_scale_and_mantissa) {
  s21_decimal value = make_decimal(123456, 789, 123, 7, 0);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(result.bits[0], value.bits[0]);
  ck_assert_int_eq(result.bits[1], value.bits[1]);
  ck_assert_int_eq(result.bits[2], value.bits[2]);
  ck_assert_int_eq(((uint32_t)result.bits[3] >> 16) & 0xFF, 7);
  ck_assert_int_eq((uint32_t)result.bits[3] >> 31, 1);
}
END_TEST

START_TEST(test_negate_null) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);

  ck_assert_int_eq(s21_negate(value, NULL), 1);
}
END_TEST

/* ==================== round ==================== */

START_TEST(test_round_down) {
  s21_decimal value = make_decimal(1234, 0, 0, 1, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 0);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_up) {
  s21_decimal value = make_decimal(1236, 0, 0, 1, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(124, 0, 0, 0, 0);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_half_up) {
  s21_decimal value = make_decimal(1235, 0, 0, 1, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(124, 0, 0, 0, 0);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_negative_down) {
  s21_decimal value = make_decimal(1234, 0, 0, 1, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 1);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_negative_half_up) {
  s21_decimal value = make_decimal(1235, 0, 0, 1, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(124, 0, 0, 0, 1);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_negative_up) {
  s21_decimal value = make_decimal(1236, 0, 0, 1, 1);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(124, 0, 0, 0, 1);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_without_fraction) {
  s21_decimal value = make_decimal(123, 0, 0, 0, 0);
  s21_decimal result = {{0}};
  s21_decimal expected = make_decimal(123, 0, 0, 0, 0);

  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(decimal_equal(result, expected), 1);
}
END_TEST

START_TEST(test_round_null) {
  s21_decimal value = make_decimal(12345, 0, 0, 2, 0);

  ck_assert_int_eq(s21_round(value, NULL), 1);
}
END_TEST

START_TEST(test_round_large_mantissa) {
  s21_decimal value = make_decimal(
      0xFFFFFFF7,
      0xFFFFFFFF,
      9,
      1,
      0
  );
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_round(value, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0xFFFFFFFF);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0xFFFFFFFF);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0);
}
END_TEST

START_TEST(test_round_negative_half_multiword) {
  s21_decimal value = make_decimal(0xFFFFFFF6, 0xFFFFFFFF, 9, 1, 1);
  s21_decimal result = {{0}};

  ck_assert_int_eq(s21_round(value, &result), 0);

  ck_assert_uint_eq((uint32_t)result.bits[0], 0xFFFFFFFF);
  ck_assert_uint_eq((uint32_t)result.bits[1], 0xFFFFFFFF);
  ck_assert_uint_eq((uint32_t)result.bits[2], 0);
  ck_assert_uint_eq((uint32_t)result.bits[3], 0x80000000);
}
END_TEST

/* ==================== suite ==================== */

Suite *s21_other_suite(void) {
  Suite *suite = suite_create("s21_other");

  TCase *tc_truncate = tcase_create("truncate");
  tcase_add_test(tc_truncate, test_truncate_positive);
  tcase_add_test(tc_truncate, test_truncate_negative);
  tcase_add_test(tc_truncate, test_truncate_without_fraction);
  tcase_add_test(tc_truncate, test_truncate_zero);
  tcase_add_test(tc_truncate, test_truncate_null);
  tcase_add_test(tc_truncate, test_truncate_large_mantissa);
  tcase_add_test(tc_truncate, test_truncate_multiword);
  

  TCase *tc_floor = tcase_create("floor");
  tcase_add_test(tc_floor, test_floor_positive_fraction);
  tcase_add_test(tc_floor, test_floor_negative_fraction);
  tcase_add_test(tc_floor, test_floor_negative_integer);
  tcase_add_test(tc_floor, test_floor_positive_integer);
  tcase_add_test(tc_floor, test_floor_null);
  tcase_add_test(tc_floor, test_floor_negative_multiword_fraction);
  

  TCase *tc_negate = tcase_create("negate");
  tcase_add_test(tc_negate, test_negate_positive);
  tcase_add_test(tc_negate, test_negate_negative);
  tcase_add_test(tc_negate, test_negate_zero);
  tcase_add_test(tc_negate, test_negate_preserves_scale_and_mantissa);
  tcase_add_test(tc_negate, test_negate_null);


  TCase *tc_round = tcase_create("round");
  tcase_add_test(tc_round, test_round_down);
  tcase_add_test(tc_round, test_round_up);
  tcase_add_test(tc_round, test_round_half_up);
  tcase_add_test(tc_round, test_round_negative_down);
  tcase_add_test(tc_round, test_round_negative_half_up);
  tcase_add_test(tc_round, test_round_negative_up);
  tcase_add_test(tc_round, test_round_without_fraction);
  tcase_add_test(tc_round, test_round_null);
  tcase_add_test(tc_round, test_round_large_mantissa);
  tcase_add_test(tc_round, test_round_negative_half_multiword);

  suite_add_tcase(suite, tc_truncate);
  suite_add_tcase(suite, tc_floor);
  suite_add_tcase(suite, tc_negate);
  suite_add_tcase(suite, tc_round);

  return suite;
}