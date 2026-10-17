#ifndef S21_DECIMAL_INTERNAL_H
#define S21_DECIMAL_INTERNAL_H

#include "s21_decimal.h"

#define S21_BIG_WORDS 6

typedef struct {
  uint32_t bits[S21_BIG_WORDS];
} s21_big_decimal;

int s21_big_to_decimal(s21_big_decimal value, int scale, int sign,
                       s21_decimal* result);

#endif
