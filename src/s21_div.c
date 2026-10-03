#include "s21_decimal.h"

static int divide_by_10(s21_decimal *result){
    int res = 0;
    uint64_t current = (uint32_t)result->bits[2];
    result->bits[2] = current / 10;
    uint32_t remainder = current % 10;

    current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[1];
    result->bits[1] = current / 10;
    remainder = current % 10;

    current = ((uint64_t)remainder << 32) | (uint32_t)result->bits[0];
    result->bits[0] = current / 10;
    remainder = current % 10;
    if (remainder != 0){
        res = 1;
    }

    return res;
}

static int add_one(s21_decimal *result){
    int res = 0;
    uint64_t current = (uint32_t)result->bits[0] + 1;
    result->bits[0] = current & 0xFFFFFFFF;
    uint32_t x = current >> 32;
    if (x){
        current = (uint32_t)result->bits[1] + 1;
        result->bits[1] = current & 0xFFFFFFFF;
        x = current >> 32;
        if (x){
            current = (uint32_t)result->bits[2] + 1;
            result->bits[2] = current & 0xFFFFFFFF;
            x = current >> 32;
            if (x){
                res = 1;
            }
        }
    }
    return res;

}

int s21_truncate(s21_decimal value, s21_decimal *result){
    int res = 0;
    uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;

    for (int i = 0; i<4; i++){
        result->bits[i] = value.bits[i];
    }
    int i = 0;
    while(i < scale && res == 0) {
        divide_by_10(result);
        i++;
    }
    unsigned int sign = ((uint32_t)value.bits[3] >> 31) & 0x1;
    result->bits[3] = sign << 31;

    return res;
}

int s21_floor(s21_decimal value, s21_decimal *result){
    int res = 0;
    int ost = 0;
    uint32_t scale = ((uint32_t)value.bits[3] >> 16) & 0xFF;
    uint32_t sign = ((uint32_t)value.bits[3] >> 31) & 0x1;

    for (int i = 0; i<4; i++){
        result->bits[i] = value.bits[i];
    }
    int i = 0;
    while(i < scale && res == 0) {
        if (divide_by_10(result) == 1) {
            ost = 1;
        }
        i++;
    }
    result->bits[3] = sign << 31;
        
    if (sign != 0 && ost == 1){
        if (add_one(result) != 0){
            res = 1;
        }
    }

    return res;
}


int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    s21_decimal remainder = {0};

    result->bits[0] = 0;
    result->bits[1] = 0;
    result->bits[2] = 0;
    if (value_2.bits[0] == 0 &&
        value_2.bits[1] == 0 &&
        value_2.bits[2] == 0) {
        res = 3;
    }
    int i = 95;
    while (i >= 0 && res == 0) {
        int index = i / 32;
        int offset = i % 32;
        int bit = ((uint32_t)value_1.bits[index] >> offset) & 1;

        uint64_t current;

        current = ((uint64_t)remainder.bits[0] << 1) | bit;
        remainder.bits[0] = (uint32_t)current;
        uint32_t carry = current >> 32;

        current = ((uint64_t)remainder.bits[1] << 1) | carry;
        remainder.bits[1] = (uint32_t)current;
        carry = current >> 32;

        current = ((uint64_t)remainder.bits[2] << 1) | carry;
        remainder.bits[2] = (uint32_t)current;


        int quotient = 0;
        if ((uint32_t)remainder.bits[2] > (uint32_t)value_2.bits[2]) {
            quotient = 1;
        } else if ((uint32_t)remainder.bits[2] == (uint32_t)value_2.bits[2]) {
            if ((uint32_t)remainder.bits[1] > (uint32_t)value_2.bits[1]) {
                quotient = 1;
            } else if ((uint32_t)remainder.bits[1] == (uint32_t)value_2.bits[1]) {
                if ((uint32_t)remainder.bits[0] >= (uint32_t)value_2.bits[0]) {
                    quotient = 1;
                }
            }
        }
        if (quotient) {
            uint64_t sub;
            uint32_t borrow = 0;

            uint32_t a = (uint32_t)remainder.bits[0];
            uint32_t b = (uint32_t)value_2.bits[0];

            borrow = a < b;
            remainder.bits[0] = a - b;

            a = (uint32_t)remainder.bits[1];
            b = (uint32_t)value_2.bits[1];

            sub = (uint64_t)b + borrow;
            borrow = (uint64_t)a < sub;
            remainder.bits[1] = (uint32_t)((uint64_t)a - sub);

            a = (uint32_t)remainder.bits[2];
            b = (uint32_t)value_2.bits[2];

            sub = (uint64_t)b + borrow;
            remainder.bits[2] = (uint32_t)((uint64_t)a - sub);
        }
        
        current = ((uint64_t)result->bits[0] << 1) | quotient;
        result->bits[0] = current & 0xFFFFFFFF;

        current = ((uint64_t)result->bits[1] << 1) | (current >> 32);
        result->bits[1] = current & 0xFFFFFFFF;

        current = ((uint64_t)result->bits[2] << 1) | (current >> 32);
        result->bits[2] = current & 0xFFFFFFFF;

        i--;
    }
    uint32_t sign_1 = ((uint32_t)value_1.bits[3] >> 32) & 1;
    uint32_t scale_1 = ((uint32_t)value_1.bits[3] >> 16) & 0xFF;
    uint32_t sign_2 = ((uint32_t)value_2.bits[3] >> 32) & 1;
    uint32_t scale_2 = ((uint32_t)value_2.bits[3] >> 16) & 0xFF;
    if (sign_1 == 1 && sign_2 == 1) {
        result->bits[3] = 0;
    } else {
        result->bits[3] = sign_1 | sign_2 << 32;
    }
    return res;
}