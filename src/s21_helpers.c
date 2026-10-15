#include "s21_decimal.h"

#define S21_DECIMAL_WORDS 4

uint32_t divide_by_10(s21_decimal* result) {
  uint64_t current = (uint64_t)(uint32_t)result->bits[2];
  result->bits[2] = current / 10;
  uint32_t remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint64_t)(uint32_t)result->bits[1];
  result->bits[1] = current / 10;
  remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint64_t)(uint32_t)result->bits[0];
  result->bits[0] = current / 10;
  remainder = current % 10;

  return remainder;
}

uint32_t s21_divide_by_10_with_high(s21_decimal* result, uint32_t high) {
  uint64_t current;
  uint32_t remainder;

  current = ((uint64_t)high << 32) | (uint32_t)result->bits[2];
  result->bits[2] = current / 10;
  remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[1];
  result->bits[1] = current / 10;
  remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[0];
  result->bits[0] = current / 10;
  remainder = current % 10;

  return remainder;
}

int add_one(s21_decimal* result) {
  uint64_t current;

  current = (uint64_t)(uint32_t)result->bits[0] + 1ULL;
  result->bits[0] = (uint32_t)current;
  uint32_t carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)result->bits[1] + carry;
  result->bits[1] = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)result->bits[2] + carry;
  result->bits[2] = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  return carry;
}

int s21_compare_mantissa(s21_decimal value_1, s21_decimal value_2) {
  int res = 0;

  for (int i = 2; i >= 0 && res == 0; i--) {
    if ((uint32_t)value_1.bits[i] > (uint32_t)value_2.bits[i]) {
      res = 1;
    } else if ((uint32_t)value_1.bits[i] < (uint32_t)value_2.bits[i]) {
      res = -1;
    }
  }

  return res;
}

int s21_add_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result) {
  uint32_t carry = 0;

  for (int i = 0; i < 3; i++) {
    uint64_t sum = (uint64_t)(uint32_t)value_1.bits[i] +
                   (uint32_t)(value_2.bits[i]) + carry;

    carry = (uint32_t)(sum >> 32);

    result->bits[i] = (int)(uint32_t)sum;
  }
  return carry;
}

int s21_sub_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result) {
  uint32_t borrow = 0;

  for (int i = 0; i < 3; i++) {
    uint32_t a = (uint32_t)value_1.bits[i];
    uint32_t b = (uint32_t)value_2.bits[i];

    uint64_t needed = (uint64_t)b + borrow;

    uint32_t difference = (uint32_t)((uint64_t)a - needed);

    borrow = (uint64_t)a < needed;
    result->bits[i] = (int)difference;
  }
  return borrow;
}

int s21_get_sign(s21_decimal value) {
  unsigned int sign = (uint32_t)value.bits[3] >> 31;
  return sign;
}

void s21_set_sign(s21_decimal* value, int sign) {
  if (sign == 0) {
    value->bits[3] = (int)((uint32_t)value->bits[3] & ~(1u << 31));
  } else {
    value->bits[3] = (int)((uint32_t)value->bits[3] | (1u << 31));
  }
}

int s21_get_scale(s21_decimal value) {
  return ((uint32_t)(value.bits[3]) >> 16) & 255u;
}

void s21_set_scale(s21_decimal* value, int scale) {
  value->bits[3] = (int)((uint32_t)value->bits[3] & ~(255u << 16));

  value->bits[3] = (int)((uint32_t)value->bits[3] | ((uint32_t)scale << 16));
}

void s21_zero(s21_decimal* value) {
  for (int i = 0; i < S21_DECIMAL_WORDS; i++) {
    value->bits[i] = 0;
  }
}
