/**
 * @file
 * File:                message.h
 * Author:              Manuel Federanko
 * Version:             1.0
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

        /**
         * Initialises the messaging buffer and pointers.
         */
        void initMessaging();

        /**
         * Counts the available Characters in the receive buffer and returns
         * that number.
         * @return the number of available characters
         */
        int8_t charAvailable();

        /**
         * Reads the next Character and advances the buffer by one.
         * @return the read character
         */
        char readNext();

        /**
         * Reads a package of data and returns a value unequal to 0 if the package
         * is finished.
         * @return 0 if the package is not finshied, otherwise 1
         */
        char readPackage();

        /**
         * Returns 1 if at least one byte has been written to the buffer,
         * otherwise 0.
         * @return 1 if a byte is in the buffer, otherwise 0
         */
        char packageStarted();

        /**
         * The return value behaves in the same way as does readPackage().
         * @return 0 if the package is not finshied, otherwise 1
         */
        char packageFinished();

        /**
         * Returns the type of the package as a Character.
         * @return the type
         */
        char getType();

        /**
         * Sets the position of the buffer to the beginning of the buffer
         * effectively resetting the buffer.
         */
        void seekFront();

        /**
         * Clears the whole buffer with 0s (zeros).
         */
        void clear();

        /**
         * Reads an 8 bit signed Integer from the buffer.
         * @return the 8 bit Integer
         */
        int8_t readInt8();
        /**
         * Reads an 16 bit signed Integer from the buffer.
         * @return the 16 bit Integer
         */
        int16_t readInt16();
        /**
         * Reads an 24 bit signed Integer from the buffer.
         * @return the 24 bit Integer
         */
        int24_t readInt24();
        /**
         * Reads an 32 bit signed Integer from the buffer.
         * @return the 32 bit Integer
         */
        int32_t readInt32();
        /**
         * Reads an 3 byte Float from the buffer.
         * @return the Float
         */
        float readFloat();

        /**
         * Reads a string from the buffer into the specified char array.
         * @param c the destination
         */
        void readString(char **c);

        /**
         * Sends an 8 bit wide Integer variable over the USART register
         * @param i the data
         */
        void sendInt8(int8_t i);
        /**
         * Sends an 16 bit wide Integer variable over the USART register
         * @param i the data
         */
        void sendInt16(int16_t i);
        /**
         * Sends an 24 bit wide Integer variable over the USART register
         * @param i the data
         */
        void sendInt24(int24_t i);
        /**
         * Sends an 32 bit wide Integer variable over the USART register
         * @param i the data
         */
        void sendInt32(int32_t i);

        /**
         * Sends an 8 bit wide unsigned Integer variable over the USART register
         * @param i the data
         */
        void sendUInt8(uint8_t i);

        /**
         * Sends an 16 bit wide unsigned Integer variable over the USART register
         * @param i the data
         */
        void sendUInt16(uint16_t i);

        /**
         * Sends an 24 bit wide unsigned Integer variable over the USART register
         * @param i the data
         */
        void sendUInt24(uint24_t i);

        /**
         * Sends an 32 bit wide unsigned Integer variable over the USART register
         * @param i the data
         */
        void sendUint32(uint32_t i);

        /**
         * Sends a 3 byte wide Float over the USART module.
         * @param f the Float to send
         */
        void sendFloat(float f);

        /**
         * Sends a null-terminated String over the USART module.
         * @param val the pointer to the first element of the String
         */
        void sendString(char *val);

#ifdef	__cplusplus
}
#endif

#endif

