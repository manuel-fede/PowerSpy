/*
 * File:   intmath.c
 * Author: redxef
 *
 * Created on 12 May 2016, 19:34
 */


#include "intmath.h"
#include "types.h"

/*
 * sin(z) = 3/2*z-(z*z*z)/(0xff*0x7f)
 */
int16_t isin_(int16_t z)
{
        int32_t n1 = 3 * z;
        int32_t n2 = z * z * z; //0xff^3 > 2^23 thus we need 32 bits
        int32_t d1 = 2;
        int32_t d2 = QUARTER_ROTATION;
        d2 *= HALF_ROTATION;
        int32_t res = n1 / d1 - n2 / d2;
        return (int16_t) res;
}

int16_t isin(int16_t z)
{
        int16_t buff;
        while (z > FULL_ROTATION)
                z -= FULL_ROTATION;

        if (z > (HALF_ROTATION) + QUARTER_ROTATION) { //4th quad
                buff = FULL_ROTATION;
                buff -= z;
                return -isin_(buff);
        }
        if (z > HALF_ROTATION) {//3rd quad
                buff = z;
                buff -= HALF_ROTATION;
                return -isin_(buff);
        }
        if (z > QUARTER_ROTATION) {//2nd quad
                buff = HALF_ROTATION;
                buff -= z;
                return isin_(buff);
        }

        //1st quad
        return isin_(z);
}

int16_t icos(int16_t z)
{
        return isin(QUARTER_ROTATION + z);
}
