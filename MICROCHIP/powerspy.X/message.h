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

#define NONE                    '0'
#define STRING                  '1'
#define INT8                    '2'
#define INT16                   '3'
#define INT24                   '4'
#define INT32                   '5'
#define UINT8                   '6'
#define UINT16                  '7'
#define UINT24                  '8'
#define UINT32                  '9'
#define FLOAT                   'F'

#define START_OF_TEXT             2
#define END_OF_TEXT               3
#define RECEIVEBUFF_SIZE          8

        char receive_buff[RECEIVEBUFF_SIZE];
        int8_t buffpos;
        void initMessaging();
        int8_t charAvailable();
        char readNext();
        char readPackage();
        char packageStarted();
        char packageFinished();
        char getType();
        void seekFront();
        int8_t readInt8();
        int16_t readInt16();
        int24_t readInt24();
        int32_t readInt32();
        float readFloat();
        void readString(char **c);
        void clear();
        void sendInt8(int8_t i);
        void sendInt16(int16_t i);
        void sendInt24(int24_t i);
        void sendInt32(int32_t i);
        void sendUInt8(uint8_t i);
        void sendUInt16(uint16_t i);
        void sendUInt24(uint24_t i);
        void sendUint32(uint32_t i);
        void sendFloat(float f);
        void sendString(char *val);

#ifdef	__cplusplus
}
#endif

#endif

