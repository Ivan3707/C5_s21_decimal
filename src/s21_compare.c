#include "s21_decimal.h"

#define BIG_WORDS 6  // 96 бит * 10^28 помещается в 192 бита

static int is_zero(s21_decimal value) {
  return value.bits[0] == 0 && value.bits[1] == 0 && value.bits[2] == 0;
}

static void to_big(s21_decimal value, uint32_t* big) {
  for (int i = 0; i < BIG_WORDS; i++) {
    big[i] = 0;
  }
  for (int i = 0; i < 3; i++) {
    big[i] = (uint32_t)value.bits[i];
  }
}

static void big_mul10(uint32_t* big) {
  uint64_t carry = 0;
  for (int i = 0; i < BIG_WORDS; i++) {
    uint64_t current = (uint64_t)big[i] * 10 + carry;
    big[i] = (uint32_t)current;
    carry = current >> 32;
  }
}

static int big_cmp(const uint32_t* a, const uint32_t* b) {
  int result = 0;
  for (int i = BIG_WORDS - 1; i >= 0 && result == 0; i--) {
    if (a[i] > b[i]) {
      result = 1;
    } else if (a[i] < b[i]) {
      result = -1;
    }
  }
  return result;
}

static int compare_magnitude(s21_decimal a, s21_decimal b) {
  uint32_t big_a[BIG_WORDS];
  uint32_t big_b[BIG_WORDS];
  int scale_a = s21_get_scale(a);
  int scale_b = s21_get_scale(b);

  to_big(a, big_a);
  to_big(b, big_b);
  for (; scale_a < scale_b; scale_a++) {
    big_mul10(big_a);
  }
  for (; scale_b < scale_a; scale_b++) {
    big_mul10(big_b);
  }
  return big_cmp(big_a, big_b);
}

static int compare(s21_decimal a, s21_decimal b) {
  int result = 0;
  int sign_a = s21_get_sign(a);
  int sign_b = s21_get_sign(b);

  if (is_zero(a) && is_zero(b)) {
    result = 0;
  } else if (is_zero(a)) {
    result = sign_b ? 1 : -1;
  } else if (is_zero(b)) {
    result = sign_a ? -1 : 1;
  } else if (sign_a != sign_b) {
    result = sign_a ? -1 : 1;
  } else {
    result = compare_magnitude(a, b);
    if (sign_a) {
      result = -result;
    }
  }
  return result;
}

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) < 0;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) <= 0;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) > 0;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) >= 0;
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) == 0;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  return compare(value_1, value_2) != 0;
}