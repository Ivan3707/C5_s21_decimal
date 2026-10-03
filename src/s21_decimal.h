#ifndef S21_DECIMAL_H
#define S21_DECIMAL_H

#include <stdint.h>

typedef struct {
    int bits[4];
} s21_decimal;

int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

#endif