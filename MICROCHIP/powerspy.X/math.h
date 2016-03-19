/* 
 * File:                math.h
 * Author:              Manuel Federanko
 * Comments:            
 * Revision history:    
 */

#include <xc.h>
#include "types.h"

#ifndef __MATH_H
#define	__MATH_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PREC        20
#define RES_SCALE   3
#define E           2.7182818284590452
#define PI          3.1415926535897932
#define PI_2        1.5707963267948966
#define PI2         6.2831853071795865

    float _abs(float f);
    int8_t _signum(float f);
    float _round_down(const float f_, const int8_t precision);
    float _round_up(const float f_, const int8_t precision);
    float _round(const float f_, const int8_t precision);
    uint24_t _combinatorics(uint24_t n);
    float _pow(float nr, int8_t power);
    float _sqrt(float nr, int8_t scale);
    float _cos(float nr);
    float _sin(float nr);

#ifdef	__cplusplus
}
#endif

#endif

