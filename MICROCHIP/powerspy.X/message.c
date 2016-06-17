/*
 * File:   message.c
 * Author: redxef
 *
 * Created on 18 March 2016, 09:25
 */


#include <xc.h>
#include "message.h"
#include "powerspy.h"

int8_t readpos;
char reading;

void initMessaging()
{
        buffpos = -1;
        readpos = 0;
        reading = 0;
}

int8_t charAvailable()
{
        if (buffpos < readpos) {
                return RECEIVEBUFF_SIZE - readpos + buffpos;
        }
        return buffpos - readpos;
}

char readNext()
{
        char ret = receive_buff[readpos++];
        if (readpos == RECEIVEBUFF_SIZE)
                readpos = 0;
        return ret;
}

char packageStarted()
{
        int8_t i;
        for (i = 0; i < RECEIVEBUFF_SIZE; i++) {
                if (receive_buff[i] == START_OF_TEXT)
                        return 1;
        }
        return 0;
}

char packageFinished()
{
        int8_t i;
        if (!packageStarted())
                return 0;
        for (i = 1; i < RECEIVEBUFF_SIZE; i++)
                if (receive_buff[i] == END_OF_TEXT)
                        return 1;
        return 0;
}

char getType()
{
        int8_t i;
        for (i = 0; i < RECEIVEBUFF_SIZE; i++) {
                if (receive_buff[i] == START_OF_TEXT) {
                        return receive_buff[i + 1];
                }
        }
        return NONE;
}

void seekFront()
{
        int8_t i;
        for (i = 0; i < RECEIVEBUFF_SIZE; i++) {
                if (receive_buff[i] == START_OF_TEXT) {
                        readpos = i + 2;
                }
        }
}

void clear()
{
        int8_t i;
        for (i = 0; i < RECEIVEBUFF_SIZE; i++) {
                receive_buff[i] = 0;
        }
        reading = 0;
        readpos = 0;
}

int8_t readInt8()
{
        int8_t res = 0;

        seekFront();
        res = (int8_t) readNext();
        clear();

        return res;
}

int16_t readInt16()
{
        int16_t res = 0;

        seekFront();
        res = readNext();
        res <<= 8;
        res |= readNext();
        clear();

        return res;
}

int24_t readInt24()
{
        int24_t res = 0;

        seekFront();
        res = readNext();
        res <<= 8;
        res |= readNext();
        res <<= 8;
        res |= readNext();
        clear();

        return res;
}

int32_t readInt32()
{
        int32_t res = 0;

        seekFront();
        res = readNext();
        res <<= 8;
        res |= readNext();
        res <<= 8;
        res |= readNext();
        res <<= 8;
        res |= readNext();
        clear();

        return res;
}

float readFloat()
{
        int24_t i = readInt24();
        float *res = (float *) & i;
        return *res;
}

void readString(char **c)
{

}

void _sendchar_(char c)
{
        TXREG = c;
        while (!TRMT);
        __delay_ms(1);
}

void sendInt8(int8_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(INT8);
        _sendchar_(i);
}

void sendInt16(int16_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(INT16);
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendInt24(int24_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(INT24);
        _sendchar_((char) (i >> 16 & 0xff));
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendInt32(int32_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(INT32);
        _sendchar_((char) (i >> 24 & 0xff));
        _sendchar_((char) (i >> 16 & 0xff));
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendUInt8(uint8_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(UINT8);
        _sendchar_(i);
}

void sendUInt16(uint16_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(UINT16);
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendUInt24(uint24_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(UINT24);
        _sendchar_((char) (i >> 16 & 0xff));
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendUInt32(uint32_t i)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(UINT32);
        _sendchar_((char) (i >> 24 & 0xff));
        _sendchar_((char) (i >> 16 & 0xff));
        _sendchar_((char) (i >> 8 & 0xff));
        _sendchar_((char) (i & 0xff));
}

void sendFloat(float f)
{
        uint24_t *ptr = (uint24_t *) & f;
        _sendchar_(START_OF_TEXT);
        _sendchar_(FLOAT);
        _sendchar_((char) (*ptr >> 16 & 0xff));
        _sendchar_((char) (*ptr >> 8 & 0xff));
        _sendchar_((char) (*ptr & 0xff));
}

void sendString(char *val)
{
        _sendchar_(START_OF_TEXT);
        _sendchar_(STRING);
        while (*val) {
                _sendchar_(*val);
                val++;
        }
        _sendchar_(END_OF_TEXT);
}