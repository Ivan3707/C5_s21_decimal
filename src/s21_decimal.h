#ifndef S21_DECIMAL_H
#define S21_DECIMAL_H

#include <stdint.h>

typedef struct {
  int bits[4];
} s21_decimal;

int s21_truncate(s21_decimal value, s21_decimal* result);
int s21_floor(s21_decimal value, s21_decimal* result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);
int s21_negate(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int add_one(s21_decimal* result);
uint32_t divide_by_10(s21_decimal* result);

int s21_get_sign(s21_decimal value);
int s21_get_scale(s21_decimal value);
void s21_set_sign(s21_decimal* value, int sign);
void s21_zero(s21_decimal* value);

#endif