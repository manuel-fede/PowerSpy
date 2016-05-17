/*
 * File:   intmath.c
 * Author: redxef
 *
 * Created on 12 May 2016, 19:34
 */


#include "intmath.h"
#include "types.h"

int16_t isin_(int16_t z) {
        return (3 * z) / 2 - (z * z * z) / (SEC_1 * SEC_1 << 1);
}

int16_t isin(int16_t z) {
        while (z > SEC_1 << 2)
                z -= SEC_1<<2;

        if (z > (SEC_1 << 1) + SEC_1) //4th quad
                return -isin_((SEC_1 << 2) - z);

        if (z > SEC_1 << 1) //3rd quad
                return -isin_((z - (SEC_1 << 1)));

        if (z > SEC_1) //2nd quad
                return isin_(((SEC_1 << 1) - z));

        //1st quad
        return isin_(z);
}

int16_t icos(int16_t z) {
        return isin(SEC_1 + z);
}
