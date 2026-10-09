#include "s21_decimal.h"

#define PROD_WORDS 6
#define MAX_SCALE 28

static void mul_mantissa(s21_decimal a, s21_decimal b, uint32_t* prod) {
  for (int i = 0; i < PROD_WORDS; i++) {
    prod[i] = 0;
  }
  for (int i = 0; i < 3; i++) {
    uint64_t carry = 0;
    for (int j = 0; j < 3; j++) {
      uint64_t current = (uint64_t)(uint32_t)a.bits[i] * (uint32_t)b.bits[j] +
                         prod[i + j] + carry;
      prod[i + j] = (uint32_t)current;
      carry = current >> 32;
    }
    prod[i + 3] = (uint32_t)carry;
  }
}

static int prod_fits_96(const uint32_t* prod) {
  return prod[3] == 0 && prod[4] == 0 && prod[5] == 0;
}

static uint32_t prod_divide_by_10(uint32_t* prod) {
  uint64_t remainder = 0;
  for (int i = PROD_WORDS - 1; i >= 0; i--) {
    uint64_t current = (remainder << 32) | prod[i];
    prod[i] = (uint32_t)(current / 10);
    remainder = current % 10;
  }
  return (uint32_t)remainder;
}

static void prod_increment(uint32_t* prod) {
  for (int i = 0; i < PROD_WORDS; i++) {
    prod[i]++;
    if (prod[i] != 0) {
      break;
    }
  }
}

static int prod_to_decimal(uint32_t* prod, int scale, int sign,
                           s21_decimal* result) {
  int code = 0;
  int finished = 0;

  while (!finished) {
    uint32_t last_digit = 0;
    int sticky = 0;  

    while (scale > 0 && (scale > MAX_SCALE || !prod_fits_96(prod))) {
      if (last_digit != 0) {
        sticky = 1;
      }
      last_digit = prod_divide_by_10(prod);
      scale--;
    }
    if (last_digit > 5 ||
        (last_digit == 5 && (sticky || (prod[0] & 1u) != 0))) {
      prod_increment(prod);
    }
    finished = prod_fits_96(prod) || scale == 0;
  }

  if (!prod_fits_96(prod)) {
    code = sign ? 2 : 1;
  } else {
    s21_zero(result);
    for (int i = 0; i < 3; i++) {
      result->bits[i] = (int)prod[i];
    }
    s21_set_scale(result, scale);
    s21_set_sign(result, sign);
  }
  return code;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int code = 0;
  if (result == NULL) {
    code = 1;
  } else {
    uint32_t prod[PROD_WORDS];
    int scale = s21_get_scale(value_1) + s21_get_scale(value_2);
    int sign = s21_get_sign(value_1) ^ s21_get_sign(value_2);

    mul_mantissa(value_1, value_2, prod);
    code = prod_to_decimal(prod, scale, sign, result);
  }
  return code;
}