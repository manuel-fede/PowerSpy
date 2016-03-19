/*
 * File:   message.c
 * Author: redxef
 *
 * Created on 18 March 2016, 09:25
 */


#include <xc.h>
#include "message.h"

void initMessaging() {
    buffpos = -1;
    readpos = 0;
}

int8_t charAvailable() {
    if (buffpos < readpos) {
        return RECEIVEBUFF_SIZE - readpos + buffpos;
    }
    return buffpos - readpos;
}

char readNext() {
    char ret = receive_buff[readpos];
    readpos++;
    if (readpos == RECEIVEBUFF_SIZE) readpos = 0;
    return ret;
}

void sendChar(char c) {
    TXREG = c;
    while (!TRMT);
}

void sendInt8(int8_t i) {
    sendChar(START_OF_TEXT);
    sendChar(INT8);
    sendChar(i);
    sendChar(END_OF_TEXT);
}
void sendInt16(int16_t i) {
    sendChar(START_OF_TEXT);
    sendChar(INT16);
    sendChar((char) (i >> 8 & 0xff));
    sendChar((char) (i & 0xff));
    sendChar(END_OF_TEXT);
}

void sendInt24(int24_t i) {
    sendChar(START_OF_TEXT);
    sendChar(INT24);
    sendChar((char) (i >> 16 & 0xff));
    sendChar((char) (i >> 8 & 0xff));
    sendChar((char) (i & 0xff));
    sendChar(END_OF_TEXT);
}

void sendInt32(int32_t i) {
    sendChar(START_OF_TEXT);
    sendChar(INT32);
    sendChar((char) (i >> 24 & 0xff));
    sendChar((char) (i >> 16 & 0xff));
    sendChar((char) (i >> 8 & 0xff));
    sendChar((char) (i & 0xff));
    sendChar(END_OF_TEXT);
}

void sendFloat(float f) {
    int *ptr = (int *) &f;
    sendChar(START_OF_TEXT);
    sendChar(FLOAT);
    sendChar((char) ((*ptr) >> 16 & 0xff));
    sendChar((char) (*ptr >> 8 & 0xff));
    sendChar((char) (*ptr & 0xff));
    sendChar(END_OF_TEXT);
}

void sendString(char *val) {
    sendChar(START_OF_TEXT);
    sendChar(STRING);
    while (*val) {
        sendChar(*val);
        val++;
    }
    sendChar(END_OF_TEXT);
}