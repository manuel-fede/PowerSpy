/* 
 * File:                str.h
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef __STR_H
#define	__STR_H

#include <xc.h>
#include "types.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
    uint8_t _strlen(char *c);
    void reverse(char *c, uint8_t leng);
    void flt_to_str(const float f_, char *c, const uint8_t max_precision);

#ifdef	__cplusplus
}
#endif

#endif

