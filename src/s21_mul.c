#include "s21_decimal_internal.h"

#define PROD_WORDS S21_BIG_WORDS

// Полное умножение 96x96 -> 192 бита
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

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int code = 0;
  if (result == NULL) {
    code = 1;
  } else {
    s21_big_decimal product;
    int scale = s21_get_scale(value_1) + s21_get_scale(value_2);
    int sign = s21_get_sign(value_1) ^ s21_get_sign(value_2);

    mul_mantissa(value_1, value_2, product.bits);
    code = s21_big_to_decimal(product, scale, sign, result);
  }
  return code;
}
