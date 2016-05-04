/*
 * File:   str.c
 * Author: redxef
 *
 * Created on 18 March 2016, 12:05
 */


#include <xc.h>
#include "str.h"
#include "math.h"

uint8_t _strlen(char *c)
{
        int i = 0;
        while (*c) {
                c++;
                i++;
        }
        return i;
}

void reverse(char *c, uint8_t leng)
{
        uint8_t i;
        char buff;
        for (i = 0; i < leng / 2; i++) {
                buff = c[i];
                c[i] = c[leng - i];
                c[leng - i] = buff;
        }
}

void print(char *flw, char* app)
{
        while (*app)
                *flw++ = *app++;
}

void flt_to_str(const float f_, char *c, const uint8_t max_precision)
{
        float f = _abs(f_);
        int it;
        int i;
        int dec;

        it = 0;
        dec = (int) f;
        while (dec < f && it < max_precision) {
                f *= 10;
                dec = (int) f;
                it++;
        }

        i = 0;
        while (it >= 0) {
                c[i] = '0' + dec % 10;
                dec /= 10;
                i++;
                it--;
        }
        c[i] = '.';
        i++;
        while (dec > 0) {
                c[i] = '0' + dec % 10;
                dec /= 10;
                i++;
        }
        c[i] = (_signum(f_) < 0) ? '-' : '+';
        i++;
        c[i] = '\0';
        reverse(c, _strlen(c) - 1);
}