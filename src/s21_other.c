#include "s21_decimal.h"

uint32_t divide_by_10(s21_decimal* result) {
  uint64_t current = (uint32_t)result->bits[2];
  result->bits[2] = current / 10;
  uint32_t remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[1];
  result->bits[1] = current / 10;
  remainder = current % 10;

  current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[0];
  result->bits[0] = current / 10;
  remainder = current % 10;

  return remainder;
}

int add_one(s21_decimal* result) {
  int res = 0;
  uint64_t current = (uint32_t)result->bits[0] + 1;
  result->bits[0] = current & 0xFFFFFFFF;
  uint32_t x = current >> 32;
  if (x) {
    current = (uint32_t)result->bits[1] + 1;
    result->bits[1] = current & 0xFFFFFFFF;
    x = current >> 32;
    if (x) {
      current = (uint32_t)result->bits[2] + 1;
      result->bits[2] = current & 0xFFFFFFFF;
      x = current >> 32;
      if (x) {
        res = 1;
      }
    }
  }
  return res;
}

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