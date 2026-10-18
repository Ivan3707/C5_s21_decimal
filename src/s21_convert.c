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
    uint32_t mantissa = (uint32_t)truncated.bits[0];
    int too_big = (truncated.bits[1] != 0 || truncated.bits[2] != 0);
    int overflow = (sign == 0 && mantissa > (uint32_t)INT_MAX) ||
                   (sign == 1 && mantissa > (uint32_t)INT_MAX + 1u);
    if (too_big || overflow) {
      flag = 1;
    } else if (sign) {
      *dst = (int)(-(int64_t)mantissa);
    } else {
      *dst = (int)mantissa;
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

typedef struct {
  uint32_t w[6];
} s21_big192;

static void s21_big192_from_dec(s21_big192* b, const s21_decimal* d) {
  for (int i = 0; i < 3; i++) b->w[i] = (uint32_t)d->bits[i];
  for (int i = 3; i < 6; i++) b->w[i] = 0;
}

static int s21_big192_div10(s21_big192* b) {
  uint64_t rem = 0;
  for (int i = 5; i >= 0; i--) {
    uint64_t cur = (rem << 32) | b->w[i];
    b->w[i] = (uint32_t)(cur / 10u);
    rem = cur % 10u;
  }
  return (int)rem;
}

static int s21_big192_add_one(s21_big192* b) {
  uint64_t carry = 1;
  for (int i = 0; i < 6 && carry; i++) {
    uint64_t cur = (uint64_t)b->w[i] + carry;
    b->w[i] = (uint32_t)cur;
    carry = cur >> 32;
  }
  return (int)carry;
}

static int s21_big192_fits96(const s21_big192* b) {
  return b->w[3] == 0 && b->w[4] == 0 && b->w[5] == 0;
}

/* Перекладываем 192-бит в s21_decimal (только младшие 96 бит) */
static void s21_big192_to_dec(s21_big192* b, s21_decimal* d) {
  for (int i = 0; i < 3; i++) d->bits[i] = (int)b->w[i];
}

static int s21_conv_is_zero(const s21_decimal* d) {
  return d->bits[0] == 0 && d->bits[1] == 0 && d->bits[2] == 0;
}

/* d = d * 10; возвращает 1 при переполнении 96 бит */
static int s21_conv_mul10(s21_decimal* d) {
  uint64_t carry = 0;
  for (int i = 0; i < 3; i++) {
    uint64_t cur = (uint64_t)(uint32_t)d->bits[i] * 10u + carry;
    d->bits[i] = (int)(uint32_t)cur;
    carry = cur >> 32;
  }
  return carry != 0;
}

static int s21_conv_mul10_n(s21_decimal* d, int n) {
  int ovf = 0;
  for (int i = 0; i < n && !ovf; i++) ovf = s21_conv_mul10(d);
  return ovf;
}

static int s21_conv_add_digit(s21_decimal* d, int digit) {
  uint64_t carry = (uint64_t)digit;
  for (int i = 0; i < 3 && carry; i++) {
    uint64_t cur = (uint64_t)(uint32_t)d->bits[i] + carry;
    d->bits[i] = (int)(uint32_t)cur;
    carry = cur >> 32;
  }
  return carry != 0;
}

static int s21_conv_scale_down(s21_decimal* d, int* scale) {
  int ovf = 0;
  s21_big192 b;
  s21_big192_from_dec(&b, d);

  while (*scale > 28 && !ovf) {
    int rem = s21_big192_div10(&b);
    int last = b.w[0] & 1u; /* чётность усечённого значения */
    if (rem > 5 || (rem == 5 && last)) {
      if (s21_big192_add_one(&b)) {
        ovf = 1;
      }
    }
    if (!ovf) {
      (*scale)--;
    }
  }

  if (!ovf) {
    if (!s21_big192_fits96(&b)) {
      ovf = 1;
    } else {
      s21_big192_to_dec(&b, d);
    }
  }
  return ovf;
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

static int s21_parse_float_string(const char* buf, char* digits, int* ndigits,
                                  int* exp10) {
  int flag = 0;
  int i = 0;
  *ndigits = 0;
  *exp10 = 0;

  if (buf[0] < '0' || buf[0] > '9') {
    flag = 1;
  } else {
    digits[(*ndigits)++] = buf[0];
    i = 1;
    if (buf[i] == '.') {
      i++;
      while (buf[i] >= '0' && buf[i] <= '9') {
        digits[(*ndigits)++] = buf[i++];
      }
    }
    if (buf[i] == 'e' || buf[i] == 'E') {
      *exp10 = atoi(&buf[i + 1]);
    } else {
      flag = 1;
    }
  }
  return flag;
}

static int s21_build_from_digits(s21_decimal* d, const char* digits,
                                 int ndigits) {
  int ovf = 0;
  s21_zero(d);
  for (int i = 0; i < ndigits && !ovf; i++) {
    ovf = s21_conv_mul10(d);
    if (!ovf) ovf = s21_conv_add_digit(d, digits[i] - '0');
  }
  return ovf;
}

static int s21_float_assemble(float abs_src, s21_decimal* dst) {
  char buf[64];
  char digits[64];
  int ndigits = 0;
  int exp10 = 0;
  int flag = 0;

  snprintf(buf, sizeof(buf), "%.6e", abs_src);

  if (s21_parse_float_string(buf, digits, &ndigits, &exp10) != 0) {
    flag = 1;
  } else {
    int scale = 6 - exp10;

    if (s21_build_from_digits(dst, digits, ndigits)) {
      flag = 1;
    } else {
      if (scale < 0) {
        if (s21_conv_mul10_n(dst, -scale)) flag = 1;
        scale = 0;
      }
      if (!flag && scale > 28) {
        if (s21_conv_scale_down(dst, &scale)) flag = 1;
      }
      if (!flag && s21_conv_is_zero(dst)) flag = 1;
      if (!flag) s21_set_scale(dst, scale);
    }
  }
  return flag;
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
      if (s21_float_assemble(abs_src, dst)) {
        flag = 1;
      } else {
        s21_set_sign(dst, sign);
      }
    }
  }
  return flag;
}