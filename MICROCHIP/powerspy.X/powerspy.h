/*
 * File:                powerspy.h
 * Author:              Manuel Federanko
 * Comments:            
 * Revision history:    
 */

#include <xc.h>
#include "types.h"

#ifndef __POWERSPY_H
#define	__POWERSPY_H

#ifdef	__cplusplus
extern "C" {
#endif

#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin) - needed for 32mhz
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

        // CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL disabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

        //xc8 gives a warning when converting to lower data types
        //even when casting to the appropriate type
#pragma warning push
#pragma warning disable 752
#pragma warning enable pop

        //print specific defines
#define _XTAL_FREQ              32000000
#define RX                      RB1
#define TX                      RB2

#define IN_FREQ                 50

#define CURRENT_VAL_IN          RB5
#define CURRENT_PHA_IN          RA0

#define VOLTAGE_VAL_IN          RB4
#define VOLTAGE_PHA_IN          RA1

#define DISPLAY_LAT             RA3
#define DISPLAY_CLK             RA4
#define DISPLAY_DATA            RA7

#define SHIFT_DIR_MSBFIRST      1
#define SHIFT_DIR_LSBFIRST      0

#define STATUS_LED              RA6
#define BUTTON                  RB3

#define PWM_OUT_GEN_VOLT        RB0
#define PWM_IN_REF              RA2

#define SHIFT_REG_LEN           7

#define RET_OK                  0
#define RET_NOK                 1

#define K_CURRENT               'c'
#define K_VOLTAGE               'v'
#define K_ANGLE                 'a'
#define K_APPARENTEPOWER        'A'
#define K_REALPOWER             'r'
#define K_REACTIVEPOWER         'R'

#define VOLT_TO_AMP_FACT        5

#define NRMASK                  0b10000000
#define NR0                     0b10000001
#define NR1                     0b10111101
#define NR2                     0b11001000
#define NR3                     0b10011000
#define NR4                     0b10110100
#define NR5                     0b10010010
#define NR6                     0b10000010
#define NR7                     0b10111001
#define NR8                     0b10000000
#define NR9                     0b10010000

        //shift 1 - 3
#define BIGMASK                 0b110100000000000011100111
#define SMAMASK                 0b111111111111111100011111
#define MASK                    (BIGMASK|SMAMASK)

#define V                       0b111101101011101111111111
#define WFIRST                  0b111111101010101011110111
#define WSECOND                 0b111111111111111100111111
#define AFIRST                  0b111110100111101011100111
#define ASECOND                 0b111111111111111101011111

#define NNR0                    0b111111101111100011100111
#define NNR1                    0b111111111111111011110111
#define NNR2                    0b111110110111100111100111
#define NNR3                    0b111110110111110011100111
#define NNR4                    0b111110100111111011110111
#define NNR5                    0b111110100111110011101111
#define NNR6                    0b111110100111100011101111
#define NNR7                    0b111111111111111011100111
#define NNR8                    0b111110100111100011100111
#define NNR9                    0b111110100111110011100111

#define WAIT_T0H                NOP();
#define WAIT_T0L                NOP();\
                                NOP();\
                                NOP();
#define WAIT_T1H                NOP();\
                                NOP();\
                                NOP();\
                                NOP();
#define WAIT_T1L                NOP();

#define LED_LOWBIT              {\
                                STATUS_LED=1;\
                                WAIT_T0H\
                                STATUS_LED=0;\
                                WAIT_T0L\
                                }
#define LED_HIGHBIT             {\
                                STATUS_LED=1;\
                                WAIT_T1H\
                                STATUS_LED=0;\
                                WAIT_T1L\
                                }

#define DMODE_NONE              0
#define DMODE_CURRENT           1
#define DMODE_VOLTAGE           2
#define DMODE_ANGLE             3
#define DMODE_APPARENT          4
#define DMODE_REAL              5
#define DMODE_REACTIVE          6
#define DMODE_MAX               6

#define getTime()           TMR1

        void initPins();
        void initADC();
        void initTMR2();
        void initTMR1();
        void initFVR();
        void initPWMTMR4();
        void initCOMP1();
        void initCOMP2();
        void adc(const int8_t src);
        uint8_t readVoltage();
        float readCurrent();
        void so(const uint8_t data);
        uint24_t combine(uint24_t nr1, uint24_t nr2);
        void clearDisplay(int8_t leng);
        uint8_t setNr(uint16_t);
        void __interrupt ISR();
        void main();

#ifdef	__cplusplus
}
#endif

#endif

