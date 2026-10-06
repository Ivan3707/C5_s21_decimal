#include <stdio.h>

#include "s21_decimal.h"

static int compare_mantissa(s21_decimal value_2, s21_decimal* remainder) {
  int quotient = 0;
  if ((uint32_t)remainder->bits[2] > (uint32_t)value_2.bits[2]) {
    quotient = 1;
  } else if ((uint32_t)remainder->bits[2] == (uint32_t)value_2.bits[2]) {
    if ((uint32_t)remainder->bits[1] > (uint32_t)value_2.bits[1]) {
      quotient = 1;
    } else if ((uint32_t)remainder->bits[1] == (uint32_t)value_2.bits[1]) {
      if ((uint32_t)remainder->bits[0] >= (uint32_t)value_2.bits[0]) {
        quotient = 1;
      }
    }
  }
  return quotient;
}

static uint32_t subtract_mantissa(s21_decimal value_2, s21_decimal* remainder) {
  uint64_t sub;
  uint32_t borrow = 0;

  uint32_t a = (uint32_t)remainder->bits[0];
  uint32_t b = (uint32_t)value_2.bits[0];

  borrow = a < b;
  remainder->bits[0] = a - b;

  a = (uint32_t)remainder->bits[1];
  b = (uint32_t)value_2.bits[1];

  sub = (uint64_t)b + borrow;
  borrow = (uint64_t)a < sub;
  remainder->bits[1] = (uint32_t)((uint64_t)a - sub);

  a = (uint32_t)remainder->bits[2];
  b = (uint32_t)value_2.bits[2];

  sub = (uint64_t)b + borrow;
  borrow = (uint64_t)a < sub;
  remainder->bits[2] = (uint32_t)((uint64_t)a - sub);

  return borrow;
}

// первый цикл
static void divide_integer_part(s21_decimal value_1, s21_decimal value_2,
                                s21_decimal* result, s21_decimal* remainder) {
  int i = 95;
  while (i >= 0) {
    int index = i / 32;
    int offset = i % 32;
    int bit = ((uint32_t)value_1.bits[index] >> offset) & 1;

    uint64_t current;

    current = ((uint64_t)(uint32_t)remainder->bits[0] << 1) | bit;
    remainder->bits[0] = (uint32_t)current;
    uint32_t carry = current >> 32;

    current = ((uint64_t)(uint32_t)remainder->bits[1] << 1) | carry;
    remainder->bits[1] = (uint32_t)current;
    carry = current >> 32;

    current = ((uint64_t)(uint32_t)remainder->bits[2] << 1) | carry;
    remainder->bits[2] = (uint32_t)current;

    int x = compare_mantissa(value_2, remainder);
    if (x) {
      subtract_mantissa(value_2, remainder);
    }

    current = ((uint64_t)(uint32_t)result->bits[0] << 1) | x;
    result->bits[0] = current & 0xFFFFFFFF;

    current = ((uint64_t)(uint32_t)result->bits[1] << 1) | (current >> 32);
    result->bits[1] = current & 0xFFFFFFFF;

    current = ((uint64_t)(uint32_t)result->bits[2] << 1) | (current >> 32);
    result->bits[2] = current & 0xFFFFFFFF;

    i--;
  }
}

static uint32_t multiply_mantissa_by_10(s21_decimal* remainder,
                                        uint32_t digit) {
  uint64_t current = (uint64_t)(uint32_t)remainder->bits[0] * 10ULL + digit;
  remainder->bits[0] = (uint32_t)current;
  uint32_t carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)remainder->bits[1] * 10ULL + carry;
  remainder->bits[1] = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)remainder->bits[2] * 10ULL + carry;
  remainder->bits[2] = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  return carry;
}

static uint32_t divide_by_10_with_high(s21_decimal* result, uint32_t high) {
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

static void round_overflow_result(s21_decimal* result, s21_decimal remainder,
                                  uint32_t remainder_10, int* res) {
  if (remainder_10 > 5 || (remainder_10 == 5 &&
                           (((uint32_t)result->bits[0] & 1) ||
                            (remainder.bits[0] != 0 || remainder.bits[1] != 0 ||
                             remainder.bits[2] != 0)))) {
    s21_decimal temp = *result;
    if (add_one(&temp) == 0) {
      *result = temp;
    } else {
      *res = 1;
    }
  }
}
// второй цикл
static void calculate_fraction_part(s21_decimal value_2, s21_decimal* result,
                                    s21_decimal* remainder, int* scale,
                                    int* overflow, int* res) {
  while ((remainder->bits[0] != 0 || remainder->bits[1] != 0 ||
          remainder->bits[2] != 0) &&
         *scale < 28 && *res == 0 && *overflow == 0) {
    uint32_t remainder_high = 0;
    uint32_t carry = multiply_mantissa_by_10(remainder, 0);

    if (carry != 0) {
      remainder_high = carry;
    }

    int digit = 0;
    int greater = 1;

    while (digit < 10 && greater == 1) {
      greater = 0;
      if (remainder_high != 0) {
        greater = 1;
      } else if (compare_mantissa(value_2, remainder)) {
        greater = 1;
      }
      if (greater) {
        uint32_t borrow = subtract_mantissa(value_2, remainder);
        remainder_high -= borrow;
        digit++;
      }
    }

    carry = multiply_mantissa_by_10(result, digit);

    (*scale)++;
    if (carry != 0) {
      *overflow = 1;
      uint32_t remainder_10 = divide_by_10_with_high(result, carry);
      round_overflow_result(result, *remainder, remainder_10, res);
      (*scale)--;
    }
  }
}

static int compare_twice_remainder(s21_decimal remainder, s21_decimal divisor) {
  int res = 0;
  uint64_t current;
  uint32_t carry;

  current = (uint64_t)(uint32_t)remainder.bits[0] << 1;
  uint32_t low = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)remainder.bits[1] << 1 | carry;
  uint32_t mid = (uint32_t)current;
  carry = (uint32_t)(current >> 32);

  current = (uint64_t)(uint32_t)remainder.bits[2] << 1 | carry;
  uint32_t high = (uint32_t)current;
  uint32_t high_carry = (uint32_t)(current >> 32);

  if (high_carry > 0) {
    res = 1;
  } else if (high > (uint32_t)divisor.bits[2]) {
    res = 1;
  } else if (high < (uint32_t)divisor.bits[2]) {
    res = -1;
  }

  else if (mid > (uint32_t)divisor.bits[1]) {
    res = 1;
  } else if (mid < (uint32_t)divisor.bits[1]) {
    res = -1;
  }

  else if (low > (uint32_t)divisor.bits[0]) {
    res = 1;
  } else if (low < (uint32_t)divisor.bits[0]) {
    res = -1;
  }

  return res;
}

static uint32_t divide_by_10_with_bankers_rounding(s21_decimal* result) {
  uint32_t remainder = divide_by_10(result);

  if ((remainder > 5) || (remainder == 5 && ((uint32_t)result->bits[0] & 1))) {
    add_one(result);
  }
  return remainder;
}

// проверка на scale == 28 и остаток != 0 и overflow != 1
static void round_division_result(s21_decimal* result, s21_decimal remainder,
                                  s21_decimal value_2, int* scale, int overflow,
                                  int* res) {
  if (*scale == 28 &&
      (remainder.bits[0] != 0 || remainder.bits[1] != 0 ||
       remainder.bits[2] != 0) &&
      overflow != 1) {
    int comparison = compare_twice_remainder(remainder, value_2);
    if (comparison > 0 ||
        (comparison == 0 && ((uint32_t)result->bits[0] & 1))) {
      s21_decimal temp = *result;
      if (add_one(&temp) == 0) {
        *result = temp;
      } else {
        divide_by_10_with_bankers_rounding(result);
        (*scale)--;
      }
    }
    if (result->bits[0] == 0 && result->bits[1] == 0 && result->bits[2] == 0) {
      *res = 2;
    }
  }
}

static void normalize_negative_scale(s21_decimal* result, int* scale,
                                     int* res) {
  while (*scale < 0 && *res == 0) {
    uint64_t current = (uint64_t)(uint32_t)result->bits[0] * 10ULL;
    result->bits[0] = (uint32_t)current;
    uint32_t carry = (uint32_t)(current >> 32);

    current = (uint64_t)(uint32_t)result->bits[1] * 10ULL + carry;
    result->bits[1] = (uint32_t)current;
    carry = (uint32_t)(current >> 32);

    current = (uint64_t)(uint32_t)result->bits[2] * 10ULL + carry;
    result->bits[2] = (uint32_t)current;
    carry = (uint32_t)(current >> 32);
    if (carry != 0) {
      *res = 1;
    }
    (*scale)++;
  }
}

// главныя функция деления
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int res = 0;
  if (result == NULL) {
    res = 1;
  } else {
    int overflow = 0;
    s21_decimal remainder = {0};
    if (res == 0) {
      s21_zero(result);
    }
    if (value_2.bits[0] == 0 && value_2.bits[1] == 0 && value_2.bits[2] == 0) {
      res = 3;
    }
    if (res == 0) {
      divide_integer_part(value_1, value_2, result, &remainder);
    }
    int scale = 0;
    if (res == 0) {
      int scale_1 = s21_get_scale(value_1);
      int scale_2 = s21_get_scale(value_2);
      scale = (int)scale_1 - (int)scale_2;
    }
    calculate_fraction_part(value_2, result, &remainder, &scale, &overflow,&res);
    normalize_negative_scale(result, &scale, &res);
    round_division_result(result, remainder, value_2, &scale, overflow, &res);

    int sign_1 = s21_get_sign(value_1);
    int sign_2 = s21_get_sign(value_2);

    uint32_t sign = sign_1 ^ sign_2;

    if (res == 1 && sign != 0) {
      res = 2;
    }
    if (res != 0) {
      s21_zero(result);
    } else {
      result->bits[3] = (uint32_t)scale << 16;
      s21_set_sign(result, sign);
    }
  }
  return res;
}