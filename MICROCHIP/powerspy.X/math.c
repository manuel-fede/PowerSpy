/*
 * File:   math.c
 * Author: Manuel Federanko
 *
 * Created on 12 February 2016, 19:30
 */


#define SCALE   20
#define E       2.7182818284590452
#define PI      3.1415926535897932
#define PI_2    1.5707963267948966
#define PI2     6.2831853071795865

/*
 * 
 */

void _abs(float *n) {
    if (*n < 0) (*n) = -(*n);
}

void _combinatorics(unsigned int *res, unsigned int n) {
    (*res) = 1;
    while (n) (*res) *= n--;
}

void _pow(float *nr, int power) {
    float buff = (*nr);

    if (power == 0) {
        (*nr) = 1;
    }

    power--;
    while (power--) (*nr) *= buff;
}

void _taylor_cos(float *res, float nr, int scale) {
    char sign = 0;
    int flw = 2;
    (*res) = 1;
    float prev_res;
    unsigned int buff_0 = 0;
    float buff_1 = 0, buff_2 = 0;

    do {
        prev_res = (*res);

        //math
        _combinatorics(&buff_0, flw);
        buff_1 = nr;
        _pow(&buff_1, flw);
        buff_2 = buff_1 / buff_0;
        (*res) = (sign) ? (*res) + buff_2 : (*res) - buff_2;
        //end math

        sign = !sign;
        flw += 2;
        buff_2 = prev_res - (*res);
        _abs(&buff_2);
    } while (buff_2 > 1.0 / (float) scale);
}

void _cos(float *res, float nr) {
    if (nr == 0) {
        (*res) = 1.0;
        return;
    }
    while (nr > PI2) nr -= PI2;
    _taylor_cos(res, nr, SCALE);
}

void _taylor_sin(float *res, float nr, int scale) {
    char sign = 0;
    int flw = 3;
    (*res) = nr;
    float prev_res;
    unsigned int buff_0 = 0;
    float buff_1 = 0, buff_2 = 0;

    do {
        prev_res = (*res);
        
        //math
        _combinatorics(&buff_0, flw);
        buff_1 = nr;
        _pow(&buff_1, flw);
        buff_2 = buff_1 / buff_0;
        (*res) = (sign) ? (*res) + buff_2 : (*res) - buff_2;
        //end math
        
        sign = !sign;
        flw += 2;
        buff_2 = prev_res - (*res);
        _abs(&buff_2);
    } while (buff_2 > 1.0 / (float) scale);
}

void _sin(float *res, float nr) {
    if (nr == 0) {
        (*res) = 0.0;
        return;
    }
    while (nr > PI2) nr -= PI2;
    _taylor_sin(res, nr, SCALE);
}