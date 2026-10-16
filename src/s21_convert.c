#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal* dst) {
  int flag = 0;
  if (dst == NULL) {
    flag = 1;
  } else {
    s21_zero(dst);
    int sign = (src < 0) ? 1 : 0;
    uint32_t magnitude = (src < 0) ? (uint32_t)(-(int64_t)src) : (uint32_t)src;
    dst->bits[0] = (int)magnitude;
    s21_set_sign(dst, sign);
  }
  return flag;
}

int s21_from_decimal_to_int(s21_decimal src, int* dst) {
  int flag = 0;
  if (dst == NULL) {
    flag = 1;
  } else {
    s21_decimal truncated;
    s21_truncate(src, &truncated);
    int sign = s21_get_sign(src);
    if (truncated.bits[1] != 0 || truncated.bits[2] != 0) {
      flag = 1;
    } else {
      uint32_t mantissa = (uint32_t)truncated.bits[0];
      if ((sign == 0 && mantissa > (uint32_t)INT_MAX) ||
          (sign == 1 && mantissa > (uint32_t)INT_MAX + 1u)) {
        flag = 1;
      } else {
        if (sign) {
          *dst = (int)(-(int64_t)mantissa);
        } else {
          *dst = (int)mantissa;
        }
      }
    }
  }
  return flag;
}

int s21_from_decimal_to_float(s21_decimal src, float* dst) {
  int flag = 0;
  if (dst == NULL) {
    flag = 1;
  } else {
    double mantissa = 0.0;
    for (int i = 2; i >= 0; i--) {
      mantissa = mantissa * 4294967296.0 + (uint32_t)src.bits[i];
    }

    int scale = s21_get_scale(src);
    double result = mantissa / pow(10.0, (double)scale);

    if (s21_get_sign(src)) {
      result = -result;
    }

    *dst = (float)result;
  }
  return flag;
}

static int s21_float_check(float abs_src) {
  int flag = 0;
  if (isnan(abs_src) || isinf(abs_src)) {
    flag = 1;
  } else if (abs_src != 0.0f && abs_src < 1e-28f) {
    flag = 1;
  } else if (abs_src > 7.9228162514264337593543950335e28f) {
    flag = 1;
  }
  return flag;
}

static int s21_parse_float_string(const char* buf, uint32_t* mantissa,
                                  int* digits_after_point, int* exp10) {
  int flag = 0;
  *mantissa = 0;
  *digits_after_point = 0;

  int i = 0;
  if (buf[0] < '0' || buf[0] > '9') {
    flag = 1;
  } else {
    *mantissa = (uint32_t)(buf[0] - '0');
    i = 1;

    if (buf[i] == '.') {
      i++;
      while (buf[i] != 'e' && buf[i] != 'E' && buf[i] != '\0') {
        if (buf[i] < '0' || buf[i] > '9') {
          flag = 1;
          break;
        }
        *mantissa = *mantissa * 10u + (uint32_t)(buf[i] - '0');
        (*digits_after_point)++;
        i++;
      }
    }

    if (flag == 0 && (buf[i] == 'e' || buf[i] == 'E')) {
      *exp10 = atoi(&buf[i + 1]);
    } else if (flag == 0) {
      flag = 1;
    }
  }
  return flag;
}

static uint32_t s21_scale_up(uint32_t mantissa, int* scale) {
  while (*scale < 0) {
    mantissa = mantissa * 10u;
    (*scale)++;
  }
  return mantissa;
}

static uint32_t s21_scale_down(uint32_t mantissa, int* scale) {
  while (*scale > 28) {
    uint32_t rem = mantissa % 10u;
    mantissa /= 10u;
    if (rem > 5 || (rem == 5 && (mantissa & 1u))) {
      mantissa++;
    }
    (*scale)--;
  }
  return mantissa;
}

static void s21_pack_decimal(s21_decimal* dst, uint32_t mantissa, int scale,
                             int sign) {
  dst->bits[0] = (int)mantissa;
  dst->bits[1] = 0;
  dst->bits[2] = 0;
  s21_set_scale(dst, scale);
  s21_set_sign(dst, sign);
}

int s21_from_float_to_decimal(float src, s21_decimal* dst) {
  int flag = 0;
  if (dst == NULL) {
    flag = 1;
  } else {
    s21_zero(dst);

    int sign = signbit(src) ? 1 : 0;
    float abs_src = fabsf(src);

    if (s21_float_check(abs_src) != 0) {
      flag = 1;
    } else if (abs_src != 0.0f) {
      char buf[64];
      snprintf(buf, sizeof(buf), "%.6e", abs_src);

      uint32_t mantissa = 0;
      int digits_after_point = 0;
      int exp10 = 0;

      if (s21_parse_float_string(buf, &mantissa, &digits_after_point, &exp10) !=
          0) {
        flag = 1;
      } else {
        int scale = digits_after_point - exp10;

        mantissa = s21_scale_up(mantissa, &scale);

        if (scale > 28) {
          mantissa = s21_scale_down(mantissa, &scale);
        }

        if (mantissa == 0) {
          flag = 1;
        } else {
          s21_pack_decimal(dst, mantissa, scale, sign);
        }
      }
    }
  }
  return flag;
}