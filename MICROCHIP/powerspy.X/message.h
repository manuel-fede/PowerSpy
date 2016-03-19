/* 
 * File:                message.h
 * Author:              Manuel Federanko
 * Comments:            
 * Revision history:    
 */

#ifndef __MESSAGE_H
#define	__MESSAGE_H

#include <xc.h> 
#include "types.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define STRING              '1'
#define INT8                '2'
#define INT16               '3'
#define INT24               '4'
#define INT32               '5'
#define FLOAT               '6'

#define START_OF_TEXT       2
#define END_OF_TEXT         3
#define RECEIVEBUFF_SIZE    8

    char receive_buff[RECEIVEBUFF_SIZE];
    int8_t buffpos;
    int8_t readpos;
    int8_t charAvailable();
    char readNext();
    void sendChar(char c);
    void sendInt8(int8_t i);
    void sendInt16(int16_t i);
    void sendInt24(int24_t i);
    void sendInt32(int32_t i);
    void sendFloat(float f);
    void sendString(char *val);

#ifdef	__cplusplus
}
#endif

#endif

