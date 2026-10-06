#include "s21_decimal.h"

int s21_truncate(s21_decimal value, s21_decimal* result) {
  int res = 0;

  if (result == NULL) {
    res = 1;
  }

  if (res == 0) {
    *result = value;

    int scale = s21_get_scale(value);

    for (int i = 0; i < scale; i++) {
      divide_by_10(result);
    }

    result->bits[3] = 0;
    s21_set_sign(result, s21_get_sign(value));
  }

  return res;
}
int s21_floor(s21_decimal value, s21_decimal* result) {
  int res = 0;

  if (result == NULL) {
    res = 1;
  }

  if (res == 0) {
    *result = value;

    int scale = s21_get_scale(value);
    int sign = s21_get_sign(value);
    int ost = 0;

    for (int i = 0; i < scale; i++) {
      if (divide_by_10(result) != 0) {
        ost = 1;
      }
    }

    result->bits[3] = 0;
    s21_set_sign(result, sign);

    if (sign != 0 && ost != 0) {
      if (add_one(result) != 0) {
        res = 1;
      }
    }
  }

  return res;
}

int s21_negate(s21_decimal value, s21_decimal* result) {
  int res = 0;

  if (result == NULL) {
    res = 1;
  }

  if (res == 0) {
    *result = value;
    s21_set_sign(result, !s21_get_sign(value));
  }

  return res;
}

int s21_round(s21_decimal value, s21_decimal* result) {
  int res = 0;

  if (result == NULL) {
    res = 1;
  }

  if (res == 0) {
    *result = value;

    int scale = s21_get_scale(value);
    uint32_t remainder = 0;

    while (scale > 0) {
      remainder = divide_by_10(result);
      scale--;
    }

    if (remainder >= 5) {
      if (add_one(result) != 0) {
        res = 1;
      }
    }

    result->bits[3] = 0;
    s21_set_sign(result, s21_get_sign(value));
  }

  return res;
}