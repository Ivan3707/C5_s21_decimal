#include "s21_decimal.h"
#define S21_BIG_WORDS 6

typedef struct {
  uint32_t bits[S21_BIG_WORDS];
} s21_big_decimal;

static int s21_big_to_decimal(s21_big_decimal big, int scale, int sign,
                              s21_decimal* result);

static int s21_big_fits_decimal(s21_big_decimal value);

static uint32_t s21_big_divide_by_10(s21_big_decimal* value);

static uint32_t s21_big_sub(s21_big_decimal big_1, s21_big_decimal big_2,
                            s21_big_decimal* result);

static uint32_t s21_big_add(s21_big_decimal big_1, s21_big_decimal big_2,
                            s21_big_decimal* result);

static int s21_big_compare(s21_big_decimal big_1, s21_big_decimal big_2);

static void s21_decimal_to_big(s21_decimal value, s21_big_decimal* big);

static void s21_align_scales(s21_big_decimal* big_1, s21_big_decimal* big_2,
                             int* scale_1, int* scale_2);

static uint32_t s21_big_multiply_by_10(s21_big_decimal* value);

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int sign_1 = s21_get_sign(value_1);
  int sign_2 = s21_get_sign(value_2);
  int scale_1 = s21_get_scale(value_1);
  int scale_2 = s21_get_scale(value_2);

  int status = 0;

  if (sign_1 == sign_2 && scale_1 == scale_2) {
    int overflow = s21_add_mantissa(value_1, value_2, result);

    if (overflow) {
      if (scale_1 == 0) {
        status = sign_1 ? 2 : 1;
      } else {
        uint32_t remainder = s21_divide_by_10_with_high(result, overflow);
        scale_1--;
        if (remainder > 5 ||
            (remainder == 5 && ((uint32_t)result->bits[0] & 1u)))
          add_one(result);
      }
    }
    if (status == 0) {
      result->bits[3] = 0;
      s21_set_sign(result, sign_1);
      s21_set_scale(result, scale_1);
    }
  } else if (sign_1 != sign_2 && scale_1 == scale_2) {
    int comparison = s21_compare_mantissa(value_1, value_2);

    if (comparison == 1) {
      s21_sub_mantissa(value_1, value_2, result);

      result->bits[3] = 0;
      s21_set_sign(result, sign_1);
      s21_set_scale(result, scale_1);
    } else if (comparison == -1) {
      s21_sub_mantissa(value_2, value_1, result);
      result->bits[3] = 0;
      s21_set_sign(result, sign_2);
      s21_set_scale(result, scale_2);
    } else {
      s21_zero(result);
    }
  } else {
    s21_big_decimal big_1;
    s21_big_decimal big_2;
    s21_big_decimal big_result;
    int result_sign = 0;
    int has_result = 1;

    s21_decimal_to_big(value_1, &big_1);
    s21_decimal_to_big(value_2, &big_2);
    s21_align_scales(&big_1, &big_2, &scale_1, &scale_2);

    if (sign_1 == sign_2) {
      s21_big_add(big_1, big_2, &big_result);
      result_sign = sign_1;
    } else {
      int comparison = s21_big_compare(big_1, big_2);

      if (comparison > 0) {
        s21_big_sub(big_1, big_2, &big_result);
        result_sign = sign_1;
      } else if (comparison < 0) {
        s21_big_sub(big_2, big_1, &big_result);
        result_sign = sign_2;
      } else {
        s21_zero(result);
        has_result = 0;
      }
    }

    if (has_result) {
      status = s21_big_to_decimal(big_result, scale_1, result_sign, result);
    }
  }
  return status;
}

static void s21_align_scales(s21_big_decimal* big_1, s21_big_decimal* big_2,
                             int* scale_1, int* scale_2) {
  while (*scale_1 < *scale_2) {
    s21_big_multiply_by_10(big_1);
    (*scale_1)++;
  }

  while (*scale_1 > *scale_2) {
    s21_big_multiply_by_10(big_2);

    (*scale_2)++;
  }
}

static void s21_decimal_to_big(s21_decimal value, s21_big_decimal* big) {
  for (int i = 0; i < S21_BIG_WORDS; i++) {
    big->bits[i] = 0;
  }
  for (int i = 0; i < 3; i++) {
    big->bits[i] = (uint32_t)value.bits[i];
  }
}

static uint32_t s21_big_multiply_by_10(s21_big_decimal* big) {
  uint32_t carry = 0;

  for (int i = 0; i < S21_BIG_WORDS; i++) {
    uint64_t multi = (uint64_t)big->bits[i] * 10u + carry;
    big->bits[i] = (uint32_t)multi;
    carry = (uint32_t)(multi >> 32);
  }
  return carry;
}

static int s21_big_compare(s21_big_decimal big_1, s21_big_decimal big_2)

{
  int res = 0;

  for (int i = 5; i >= 0 && res == 0; i--) {
    if (big_1.bits[i] > big_2.bits[i]) {
      res = 1;
    } else if (big_1.bits[i] < big_2.bits[i]) {
      res = -1;
    }
  }
  return res;
}

static uint32_t s21_big_add(s21_big_decimal big_1, s21_big_decimal big_2,
                            s21_big_decimal* result) {
  uint32_t carry = 0;

  for (int i = 0; i < S21_BIG_WORDS; i++) {
    uint64_t sum = ((uint64_t)big_1.bits[i] + big_2.bits[i] + carry);

    carry = (uint32_t)(sum >> 32);

    result->bits[i] = (uint32_t)sum;
  }
  return carry;
}

static uint32_t s21_big_sub(s21_big_decimal big_1, s21_big_decimal big_2,
                            s21_big_decimal* result)

{
  uint32_t borrow = 0;

  for (int i = 0; i < S21_BIG_WORDS; i++) {
    uint32_t a = big_1.bits[i];
    uint32_t b = big_2.bits[i];

    uint64_t needed = (uint64_t)b + borrow;

    uint32_t difference = (uint32_t)((uint64_t)a - needed);

    borrow = (uint64_t)a < needed;

    result->bits[i] = (int)difference;
  }
  return borrow;
}

static uint32_t s21_big_divide_by_10(s21_big_decimal* value) {
  uint32_t remainder = 0;

  for (int i = S21_BIG_WORDS - 1; i >= 0; i--) {
    uint64_t current = ((uint64_t)remainder << 32) + value->bits[i];

    value->bits[i] = (uint32_t)(current / 10u);
    remainder = (uint32_t)(current % 10u);
  }

  return remainder;
}

static int s21_big_fits_decimal(s21_big_decimal value) {
  int res = 1;

  for (int i = S21_BIG_WORDS - 1; i > 2 && res == 1; i--) {
    if (value.bits[i] != 0) {
      res = 0;
    }
  }
  return res;
}

static int s21_big_to_decimal(s21_big_decimal big, int scale, int sign,
                              s21_decimal* result) {
  int status = 0;

  s21_zero(result);

  while (!s21_big_fits_decimal(big) && scale > 0) {
    s21_big_divide_by_10(&big);
    scale--;
  }

  if (!s21_big_fits_decimal(big)) {
    status = sign ? 2 : 1;
  }

  if (status == 0) {
    for (int i = 0; i < 3; i++) {
      result->bits[i] = (int)big.bits[i];
    }

    s21_set_scale(result, scale);
    s21_set_sign(result, sign);
  }

  return status;
}
