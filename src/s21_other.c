#include "s21_decimal.h"

int s21_truncate(s21_decimal value, s21_decimal* result) {
  int res = 0;
  if (result == NULL) {
    res = 1;
  }
  uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;

  if (res == 0){
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
  }
  return res;
}

int s21_floor(s21_decimal value, s21_decimal* result) {
  int res = 0;
  if (result == NULL) {
    res = 1;
  }
  int ost = 0;
  uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;
  uint32_t sign = ((uint32_t)value.bits[3] >> 31) & 0x1;

  if (res == 0){
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
  }
  return res;
}

int s21_negate(s21_decimal value, s21_decimal *result){
  int res = 0;
  if (result == NULL) {
    res = 1;
  }
  if (res == 0){
    for (int i = 0; i < 4; i++) {
      result->bits[i] = value.bits[i];
    }
    result->bits[3] ^= (1U << 31);
  }
  return res; 
}

int s21_round(s21_decimal value, s21_decimal *result){
  int res = 0;
  if (result == NULL) {
    res = 1;
  }
  if (res == 0){
    for (int i = 0; i < 4; i++) {
      result->bits[i] = value.bits[i];
    }
    uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;
    uint32_t remainder = 0;
    while (scale > 0) {
      remainder = divide_by_10(result);
      scale--;
    }
    if (remainder >= 5) {add_one(result);}
    uint32_t sign = ((uint32_t)value.bits[3] >> 31) & 0x1;
    result->bits[3] = sign << 31;
  }
  return res;
}