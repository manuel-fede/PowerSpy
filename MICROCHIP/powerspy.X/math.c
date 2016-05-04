/*
 * File:   math.c
 * Author: Manuel Federanko
 *
 * Created on 12 February 2016, 19:30
 */

#include "math.h"

float _abs(float f) {
        return (f > 0) ? f : -f;
}

int8_t _signum(float f) {
        if (f > 0.0f) return 1;
        else if (f < 0.0f) return -1;
        return 0;
}

float _round_down(const float f_, const int8_t precision) {
        float f = _abs(f_);
        int8_t it = 0;
        while (it < precision) {
                f *= 10;
                it++;
        }
        f = (float) (int24_t) f;
        while (it > 0) {
                f /= 10;
                it--;
        }
        return _signum(f_) * f;
}

float _round_up(const float f_, const int8_t precision) {
        float f = _abs(f_);
        int8_t it = 0;
        while (it < precision) {
                f *= 10;
                it++;
        }
        f = (float) ((int24_t) f + 1);
        while (it > 0) {
                f /= 10;
                it--;
        }
        return _signum(f_) * f;
}

float _round(const float f_, const int8_t precision) {
        float f = _abs(f_);
        int8_t it = 0;
        int24_t num;
        while (it <= precision) {
                f *= 10;
                it++;
        }
        num = (int24_t) f;
        if (num % 10 > 4) num += 10;
        f = (float) (num / 10);
        while (it > 1) {
                f /= 10;
                it--;
        }
        return _signum(f_) * f;
}

uint24_t _combinatorics(uint24_t n) {
        uint24_t res = 1;
        while (n) res *= n--;
        return res;
}

float _pow(float nr, int8_t power) {
        float buff = nr;

        if (power == 0) {
                return 1.0;
        }

        power--;
        while (power--) nr *= buff;
        return _round(nr, RES_SCALE);
}

float _sqrt(float nr, int8_t scale) {
        float ap, old_ap;

        ap = nr / 2;

        while (scale) {
                old_ap = ap;
                ap = (old_ap + nr / old_ap) / 2;
                scale--;
        }
        return _round(ap, RES_SCALE);
}

float _taylor_cos(float nr, int8_t scale) {
        char sign = 0;
        int16_t flw = 2;
        float res = 1;
        float buff = 0;
        float prev_res;

        do {
                prev_res = res;

                //math
                buff = _pow(nr, flw) / _combinatorics(flw);
                res = (sign) ? res + buff : res - buff;
                //end math

                sign = !sign;
                flw += 2;
                buff = prev_res - res;
                buff = _abs(buff);
        } while (buff > 1.0 / (float) scale);
        return res;
}

float _cos(float nr) {
        if (nr == 0) return 1.0;
        while (nr > PI2) nr -= PI2;
        return _taylor_cos(nr, PREC);
}

float _taylor_sin(float nr, int8_t scale) {
        char sign = 0;
        int16_t flw = 3;
        float res = nr;
        float prev_res;
        float buff = 0;

        do {
                prev_res = res;

                //math
                buff = _pow(nr, flw) / _combinatorics(flw);
                res = (sign) ? res + buff : res - buff;
                //end math

                sign = !sign;
                flw += 2;
                buff = prev_res - res;
                buff = _abs(buff);
        } while (buff > 1.0 / (float) scale);
        return res;
}

float _sin(float nr) {
        if (nr == 0) {
                return 0.0;
        }
        while (nr > PI2) nr -= PI2;
        return _taylor_sin(nr, PREC);
}