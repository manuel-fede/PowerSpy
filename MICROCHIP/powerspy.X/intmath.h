/* 
 * File:                intmath.h
 * Author:              Manuel Federanko
 * Comments:            performs integer math operations
 * Revision history:    none
 */

#include "types.h"

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __INTMATH_H
#define	__INTHMAT_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SEC_1           (0xff)
#define FULL_ROTATION   (SEC_1<<2)
#define MIN_SIN_RES     (-0xff)
#define MAX_SIN_RES     (0xff)
        
        int16_t isin(int16_t z);
        int16_t icos(int16_t z);

#ifdef	__cplusplus
}
#endif
#endif

