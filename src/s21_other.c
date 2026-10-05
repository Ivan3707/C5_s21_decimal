#include "s21_decimal.h"

int s21_truncate(s21_decimal value, s21_decimal* result) {
  int res = 0;
  uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;

  for (int i = 0; i < 4; i++) {
    result->bits[i] = value.bits[i];
  }
  uint32_t i = 0;
  while (i < scale && res == 0) {
    divide_by_10(result);
    i++;
  }
  unsigned int sign = ((uint32_t)value.bits[3] >> 31) & 0x1;
  result->bits[3] = sign << 31;

  return res;
}

int s21_floor(s21_decimal value, s21_decimal* result) {
  int res = 0;
  int ost = 0;
  uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;
  uint32_t sign = ((uint32_t)value.bits[3] >> 31) & 0x1;

  for (int i = 0; i < 4; i++) {
    result->bits[i] = value.bits[i];
  }
  uint32_t i = 0;
  while (i < scale && res == 0) {
    if (divide_by_10(result) != 0) {
      ost = 1;
    }
    i++;
  }
  result->bits[3] = sign << 31;

  if (sign != 0 && ost == 1) {
    if (add_one(result) != 0) {
      res = 1;
    }
  }

  return res;
}