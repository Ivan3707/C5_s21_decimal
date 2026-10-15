#ifndef S21_DECIMAL_H
#define S21_DECIMAL_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
  int bits[4];
} s21_decimal;

int s21_truncate(s21_decimal value, s21_decimal* result);
int s21_floor(s21_decimal value, s21_decimal* result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);
int s21_negate(s21_decimal value, s21_decimal* result);
int s21_round(s21_decimal value, s21_decimal* result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal* result);

int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);

int add_one(s21_decimal* result);
uint32_t divide_by_10(s21_decimal* result);
uint32_t s21_divide_by_10_with_high(s21_decimal* result, uint32_t high);

int s21_get_sign(s21_decimal value);
int s21_get_scale(s21_decimal value);
int s21_add_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result);
int s21_sub_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result);
void s21_set_sign(s21_decimal* value, int sign);
void s21_zero(s21_decimal* value);
void s21_set_scale(s21_decimal* value, int scale);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);                         

#endif