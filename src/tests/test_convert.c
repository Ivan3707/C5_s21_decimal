#include <check.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "s21_decimal.h"

/* ---------- helper: восстановить double из decimal ---------- */

static double s21_dec_to_double(s21_decimal d) {
    double val = 0.0;
    for (int i = 2; i >= 0; i--) {
        val = val * 4294967296.0 + (uint32_t)d.bits[i];
    }
    val /= pow(10.0, (double)s21_get_scale(d));
    if (s21_get_sign(d)) {
        val = -val;
    }
    return val;
}

/* ---------- int -> decimal ---------- */

START_TEST(test_int_to_decimal_zero) {
    s21_decimal d = {{-1, -1, -1, -1}};
    int res = s21_from_int_to_decimal(0, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(d.bits[0], 0);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
    ck_assert_int_eq(d.bits[3], 0);
}
END_TEST

START_TEST(test_int_to_decimal_one) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(1, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 1u);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
}
END_TEST

START_TEST(test_int_to_decimal_minus_one) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(-1, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 1u);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_int_to_decimal_positive) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(42, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 42u);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
}
END_TEST

START_TEST(test_int_to_decimal_negative) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(-42, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 42u);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_int_to_decimal_int_max) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(INT_MAX, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], (uint32_t)INT_MAX);
    ck_assert_int_eq(s21_get_sign(d), 0);
}
END_TEST

START_TEST(test_int_to_decimal_int_min) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(INT_MIN, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], (uint32_t)INT_MAX + 1u);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_int_to_decimal_2147483646) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(2147483646, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 2147483646u);
    ck_assert_int_eq(s21_get_sign(d), 0);
}
END_TEST

START_TEST(test_int_to_decimal_minus_2147483647) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_int_to_decimal(-2147483647, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 2147483647u);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_int_to_decimal_resets_bits) {
    s21_decimal d = {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};
    int res = s21_from_int_to_decimal(123, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 123u);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
}
END_TEST

START_TEST(test_int_to_decimal_null) {
    int res = s21_from_int_to_decimal(42, NULL);
    ck_assert_int_eq(res, 1);
}
END_TEST

/* ---------- decimal -> int ---------- */

START_TEST(test_decimal_to_int_zero) {
    s21_decimal d = {{0, 0, 0, 0}};
    int out = 42;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 0);
}
END_TEST

START_TEST(test_decimal_to_int_negative_zero) {
    s21_decimal d = {{0, 0, 0, (int)(1u << 31)}};
    int out = 42;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 0);
}
END_TEST

START_TEST(test_decimal_to_int_basic) {
    s21_decimal d = {{42, 0, 0, 0}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 42);
}
END_TEST

START_TEST(test_decimal_to_int_negative) {
    s21_decimal d = {{42, 0, 0, (int)(1u << 31)}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, -42);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_positive) {
    s21_decimal d = {{19, 0, 0, (int)(1u << 16)}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 1);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_negative) {
    s21_decimal d = {{19, 0, 0, (int)((1u << 31) | (1u << 16))}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, -1);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_0_9) {
    s21_decimal d = {{9, 0, 0, (int)(1u << 16)}};
    int out = 42;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 0);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_minus_0_9) {
    s21_decimal d = {{9, 0, 0, (int)((1u << 31) | (1u << 16))}};
    int out = 42;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 0);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_123_456) {
    s21_decimal d = {{123456, 0, 0, (int)(3u << 16)}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, 123);
}
END_TEST

START_TEST(test_decimal_to_int_truncate_minus_123_456) {
    s21_decimal d = {{123456, 0, 0, (int)((1u << 31) | (3u << 16))}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, -123);
}
END_TEST

START_TEST(test_decimal_to_int_int_max) {
    s21_decimal d = {{INT_MAX, 0, 0, 0}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, INT_MAX);
}
END_TEST

START_TEST(test_decimal_to_int_int_min) {
    s21_decimal d = {{(int)((uint32_t)INT_MAX + 1u), 0, 0, (int)(1u << 31)}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(out, INT_MIN);
}
END_TEST

START_TEST(test_decimal_to_int_overflow) {
    s21_decimal d = {{0, 1, 0, 0}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_decimal_to_int_bits2_overflow) {
    s21_decimal d = {{0, 0, 1, 0}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_decimal_to_int_too_big) {
    s21_decimal d = {{(int)((uint32_t)INT_MAX + 1u), 0, 0, 0}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_decimal_to_int_negative_overflow) {
    s21_decimal d = {{(int)((uint32_t)INT_MAX + 2u), 0, 0, (int)(1u << 31)}};
    int out = 0;
    int res = s21_from_decimal_to_int(d, &out);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_decimal_to_int_null) {
    s21_decimal d = {{42, 0, 0, 0}};
    int res = s21_from_decimal_to_int(d, NULL);
    ck_assert_int_eq(res, 1);
}
END_TEST

/* ---------- decimal -> float ---------- */

START_TEST(test_decimal_to_float_basic) {
    s21_decimal d = {{314, 0, 0, (int)(2u << 16)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 3.14f, 1e-5f);
}
END_TEST

START_TEST(test_decimal_to_float_negative) {
    s21_decimal d = {{314, 0, 0, (int)((1u << 31) | (2u << 16))}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, -3.14f, 1e-5f);
}
END_TEST

START_TEST(test_decimal_to_float_zero) {
    s21_decimal d = {{0, 0, 0, 0}};
    float out = 1.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq(out, 0.0f);
}
END_TEST

START_TEST(test_decimal_to_float_one) {
    s21_decimal d = {{1, 0, 0, 0}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq(out, 1.0f);
}
END_TEST

START_TEST(test_decimal_to_float_minus_one) {
    s21_decimal d = {{1, 0, 0, (int)(1u << 31)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq(out, -1.0f);
}
END_TEST

START_TEST(test_decimal_to_float_fraction) {
    s21_decimal d = {{5, 0, 0, (int)(1u << 16)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 0.5f, 1e-6f);
}
END_TEST

START_TEST(test_decimal_to_float_negative_fraction) {
    s21_decimal d = {{5, 0, 0, (int)((1u << 31) | (1u << 16))}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, -0.5f, 1e-6f);
}
END_TEST

START_TEST(test_decimal_to_float_0_001) {
    s21_decimal d = {{1, 0, 0, (int)(3u << 16)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 0.001f, 1e-7f);
}
END_TEST

START_TEST(test_decimal_to_float_integer) {
    s21_decimal d = {{12345, 0, 0, 0}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 12345.0f, 1e-3f);
}
END_TEST

START_TEST(test_decimal_to_float_123_456) {
    s21_decimal d = {{123456, 0, 0, (int)(3u << 16)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 123.456f, 1e-3f);
}
END_TEST

START_TEST(test_decimal_to_float_max_scale) {
    s21_decimal d = {{1, 0, 0, (int)(28u << 16)}};
    float out = 0.0f;
    int res = s21_from_decimal_to_float(d, &out);
    ck_assert_int_eq(res, 0);
    ck_assert_float_eq_tol(out, 1e-28f, 1e-35f);
}
END_TEST

START_TEST(test_decimal_to_float_null) {
    s21_decimal d = {{1, 0, 0, 0}};
    int res = s21_from_decimal_to_float(d, NULL);
    ck_assert_int_eq(res, 1);
}
END_TEST

/* ---------- float -> decimal: базовые ---------- */

START_TEST(test_float_to_decimal_zero) {
    s21_decimal d = {{1, 1, 1, 1}};
    int res = s21_from_float_to_decimal(0.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(d.bits[0], 0);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
}
END_TEST

START_TEST(test_float_to_decimal_negative_zero) {
    s21_decimal d = {{1, 1, 1, 1}};
    int res = s21_from_float_to_decimal(-0.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(d.bits[0], 0);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
}
END_TEST

START_TEST(test_float_to_decimal_one) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 1.0) < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_minus_one) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-1.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 1);
    ck_assert(fabs(s21_dec_to_double(d) + 1.0) < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_positive) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(3.14f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq((uint32_t)d.bits[0], 3140000u);
    ck_assert_int_eq(s21_get_scale(d), 6);
    ck_assert_int_eq(s21_get_sign(d), 0);
}
END_TEST

START_TEST(test_float_to_decimal_negative) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-3.14f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq((uint32_t)d.bits[0], 3140000u);
    ck_assert_int_eq(s21_get_scale(d), 6);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_float_to_decimal_integer) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(42.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert(s21_get_scale(d) >= 0);
}
END_TEST

START_TEST(test_float_to_decimal_zero_point_five) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(0.5f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 0.5) < 1e-6);
}
END_TEST

START_TEST(test_float_to_decimal_zero_point_one) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(0.1f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 0.1) < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_hundred) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(100.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 100.0) < 1e-3);
}
END_TEST

START_TEST(test_float_to_decimal_thousand) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1000.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 1000.0) < 1e-2);
}
END_TEST

START_TEST(test_float_to_decimal_half) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(0.5f, &d);
    ck_assert_int_eq(res, 0);
}
END_TEST

START_TEST(test_float_to_decimal_1_point_5) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1.5f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 1.5) < 1e-6);
}
END_TEST

START_TEST(test_float_to_decimal_2_point_5) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(2.5f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 2.5) < 1e-6);
}
END_TEST

START_TEST(test_float_to_decimal_pi) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(3.14159265f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 3.141593) < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_negative_pi) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-3.14159265f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_float_to_decimal_1234567) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1234567.0f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 1234567.0) < 1.0);
}
END_TEST

/* ---------- float -> decimal: большие допустимые ---------- */

START_TEST(test_float_to_decimal_1e10) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1e10f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
    /* 10^10 = 0x2_540BE400 */
    ck_assert_uint_eq((uint32_t)d.bits[0], 0x540BE400u);
    ck_assert_uint_eq((uint32_t)d.bits[1], 0x00000002u);
    ck_assert_uint_eq((uint32_t)d.bits[2], 0x00000000u);
    ck_assert(fabs(s21_dec_to_double(d) - 1e10) / 1e10 < 1e-6);
}
END_TEST

START_TEST(test_float_to_decimal_1e20) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1e20f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
    /* 10^20 = 0x5_6BC75E2D_63100000 */
    ck_assert_uint_eq((uint32_t)d.bits[0], 0x63100000u);
    ck_assert_uint_eq((uint32_t)d.bits[1], 0x6BC75E2Du);
    ck_assert_uint_eq((uint32_t)d.bits[2], 0x00000005u);
    ck_assert(fabs(s21_dec_to_double(d) - 1e20) / 1e20 < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_1e28) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1e28f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    ck_assert_int_eq(s21_get_scale(d), 0);
    /* 1e28f в float < 10^28 (≈9.999999e27), поэтому проверяем
       через восстановление double с относительной погрешностью. */
    double got = s21_dec_to_double(d);
    ck_assert(got > 9.9e27);
    ck_assert(got < 1.01e28);
    ck_assert(fabs(got - 1e28) / 1e28 < 1e-5);
}
END_TEST

START_TEST(test_float_to_decimal_negative_1e20) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-1e20f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 1);
    ck_assert_int_eq(s21_get_scale(d), 0);
    ck_assert_uint_eq((uint32_t)d.bits[0], 0x63100000u);
    ck_assert_uint_eq((uint32_t)d.bits[1], 0x6BC75E2Du);
    ck_assert_uint_eq((uint32_t)d.bits[2], 0x00000005u);
}
END_TEST

START_TEST(test_float_to_decimal_max_almost) {
    /* 7.9e28 — значение, близкое к максимуму, который влезает в decimal */
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(7.9e28f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 0);
    double got = s21_dec_to_double(d);
    ck_assert(got > 7.0e28);
    ck_assert(fabs(got - 7.9e28) / 7.9e28 < 1e-5);
}
END_TEST

/* ---------- float -> decimal: границы масштаба ---------- */

START_TEST(test_float_to_decimal_too_small) {
    s21_decimal d = {{1, 1, 1, 1}};
    int res = s21_from_float_to_decimal(1e-29f, &d);
    ck_assert_int_eq(res, 1);
    ck_assert_int_eq(d.bits[0], 0);
    ck_assert_int_eq(d.bits[1], 0);
    ck_assert_int_eq(d.bits[2], 0);
}
END_TEST

START_TEST(test_float_to_decimal_tiny_positive) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1e-20f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(fabs(s21_dec_to_double(d) - 1e-20) / 1e-20 < 1e-4);
}
END_TEST

START_TEST(test_float_to_decimal_tiny_negative) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-1e-20f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert_int_eq(s21_get_sign(d), 1);
}
END_TEST

START_TEST(test_float_to_decimal_exact_1e_minus_28) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(1e-28f, &d);
    ck_assert_int_eq(res, 0);
    ck_assert(s21_dec_to_double(d) <= 1e-27);
    ck_assert(s21_dec_to_double(d) > 0.0);
}
END_TEST

START_TEST(test_float_to_decimal_too_big) {
    s21_decimal d = {{1, 1, 1, 1}};
    int res = s21_from_float_to_decimal(1e30f, &d);
    ck_assert_int_eq(res, 1);
}
END_TEST

/* ---------- float -> decimal: спец. значения ---------- */

START_TEST(test_float_to_decimal_nan) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(NAN, &d);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_float_to_decimal_negative_nan) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-NAN, &d);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_float_to_decimal_inf) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(INFINITY, &d);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_float_to_decimal_negative_inf) {
    s21_decimal d = {{0, 0, 0, 0}};
    int res = s21_from_float_to_decimal(-INFINITY, &d);
    ck_assert_int_eq(res, 1);
}
END_TEST

START_TEST(test_float_to_decimal_null) {
    int res = s21_from_float_to_decimal(1.0f, NULL);
    ck_assert_int_eq(res, 1);
}
END_TEST

/* ---------- suite ---------- */

Suite *s21_convert_suite(void) {
    Suite *s = suite_create("s21_convert");
    TCase *tc;

    tc = tcase_create("int_to_decimal");
    tcase_add_test(tc, test_int_to_decimal_zero);
    tcase_add_test(tc, test_int_to_decimal_one);
    tcase_add_test(tc, test_int_to_decimal_minus_one);
    tcase_add_test(tc, test_int_to_decimal_positive);
    tcase_add_test(tc, test_int_to_decimal_negative);
    tcase_add_test(tc, test_int_to_decimal_int_max);
    tcase_add_test(tc, test_int_to_decimal_int_min);
    tcase_add_test(tc, test_int_to_decimal_2147483646);
    tcase_add_test(tc, test_int_to_decimal_minus_2147483647);
    tcase_add_test(tc, test_int_to_decimal_resets_bits);
    tcase_add_test(tc, test_int_to_decimal_null);
    suite_add_tcase(s, tc);

    tc = tcase_create("decimal_to_int");
    tcase_add_test(tc, test_decimal_to_int_zero);
    tcase_add_test(tc, test_decimal_to_int_negative_zero);
    tcase_add_test(tc, test_decimal_to_int_basic);
    tcase_add_test(tc, test_decimal_to_int_negative);
    tcase_add_test(tc, test_decimal_to_int_truncate_positive);
    tcase_add_test(tc, test_decimal_to_int_truncate_negative);
    tcase_add_test(tc, test_decimal_to_int_truncate_0_9);
    tcase_add_test(tc, test_decimal_to_int_truncate_minus_0_9);
    tcase_add_test(tc, test_decimal_to_int_truncate_123_456);
    tcase_add_test(tc, test_decimal_to_int_truncate_minus_123_456);
    tcase_add_test(tc, test_decimal_to_int_int_max);
    tcase_add_test(tc, test_decimal_to_int_int_min);
    tcase_add_test(tc, test_decimal_to_int_overflow);
    tcase_add_test(tc, test_decimal_to_int_bits2_overflow);
    tcase_add_test(tc, test_decimal_to_int_too_big);
    tcase_add_test(tc, test_decimal_to_int_negative_overflow);
    tcase_add_test(tc, test_decimal_to_int_null);
    suite_add_tcase(s, tc);

    tc = tcase_create("decimal_to_float");
    tcase_add_test(tc, test_decimal_to_float_basic);
    tcase_add_test(tc, test_decimal_to_float_negative);
    tcase_add_test(tc, test_decimal_to_float_zero);
    tcase_add_test(tc, test_decimal_to_float_one);
    tcase_add_test(tc, test_decimal_to_float_minus_one);
    tcase_add_test(tc, test_decimal_to_float_fraction);
    tcase_add_test(tc, test_decimal_to_float_negative_fraction);
    tcase_add_test(tc, test_decimal_to_float_0_001);
    tcase_add_test(tc, test_decimal_to_float_integer);
    tcase_add_test(tc, test_decimal_to_float_123_456);
    tcase_add_test(tc, test_decimal_to_float_max_scale);
    tcase_add_test(tc, test_decimal_to_float_null);
    suite_add_tcase(s, tc);

    tc = tcase_create("float_to_decimal");
    tcase_add_test(tc, test_float_to_decimal_zero);
    tcase_add_test(tc, test_float_to_decimal_negative_zero);
    tcase_add_test(tc, test_float_to_decimal_one);
    tcase_add_test(tc, test_float_to_decimal_minus_one);
    tcase_add_test(tc, test_float_to_decimal_positive);
    tcase_add_test(tc, test_float_to_decimal_negative);
    tcase_add_test(tc, test_float_to_decimal_integer);
    tcase_add_test(tc, test_float_to_decimal_zero_point_five);
    tcase_add_test(tc, test_float_to_decimal_zero_point_one);
    tcase_add_test(tc, test_float_to_decimal_hundred);
    tcase_add_test(tc, test_float_to_decimal_thousand);
    tcase_add_test(tc, test_float_to_decimal_half);
    tcase_add_test(tc, test_float_to_decimal_1_point_5);
    tcase_add_test(tc, test_float_to_decimal_2_point_5);
    tcase_add_test(tc, test_float_to_decimal_pi);
    tcase_add_test(tc, test_float_to_decimal_negative_pi);
    tcase_add_test(tc, test_float_to_decimal_1234567);
    /* большие допустимые */
    tcase_add_test(tc, test_float_to_decimal_1e10);
    tcase_add_test(tc, test_float_to_decimal_1e20);
    tcase_add_test(tc, test_float_to_decimal_1e28);
    tcase_add_test(tc, test_float_to_decimal_negative_1e20);
    tcase_add_test(tc, test_float_to_decimal_max_almost);
    /* границы масштаба */
    tcase_add_test(tc, test_float_to_decimal_too_small);
    tcase_add_test(tc, test_float_to_decimal_tiny_positive);
    tcase_add_test(tc, test_float_to_decimal_tiny_negative);
    tcase_add_test(tc, test_float_to_decimal_exact_1e_minus_28);
    tcase_add_test(tc, test_float_to_decimal_too_big);
    /* спец. значения */
    tcase_add_test(tc, test_float_to_decimal_nan);
    tcase_add_test(tc, test_float_to_decimal_negative_nan);
    tcase_add_test(tc, test_float_to_decimal_inf);
    tcase_add_test(tc, test_float_to_decimal_negative_inf);
    tcase_add_test(tc, test_float_to_decimal_null);
    suite_add_tcase(s, tc);

    return s;
}