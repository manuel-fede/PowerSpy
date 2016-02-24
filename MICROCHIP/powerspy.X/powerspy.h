/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

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
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

    //xc8 gives a warning when converting to lower data types
    //even when casting to the appropriate type
#pragma warning push
#pragma warning disable 752
#pragma warning enable pop

    //print specific defines
#define _XTAL_FREQ          4000000
#define RX                  RB1
#define TX                  RB2

#define IN_FREQ             50

#define CURRENT_VAL_IN      RB5
#define CURRENT_PHA_IN      RA0

#define VOLTAGE_VAL_IN      RB4
#define VOLTAGE_PHA_IN      RA1

#define DISPLAY_LAT         RA3
#define DISPLAY_CLK         RA4
#define DISPLAY_DATA        RA7

#define SHIFT_DIR_MSBFIRST  1
#define SHIFT_DIR_LSBFIRST  0

#define STATUS_LED          RA6
#define BUTTON              RB3

#define PWM_OUT_GEN_VOLT    RB0
#define PWM_IN_REF          RA2

#define SHIFT_REG_LEN       7

#define RET_OK              0
#define RET_NOK             1

#define NRMASK              0b01111111
#define NR0                 0b01111110
#define NR1                 0b01000010
#define NR2                 0b00110111
#define NR3                 0b01100111
#define NR4                 0b01001011
#define NR5                 0b01101101
#define NR6                 0b01111101
#define NR7                 0b01000110
#define NR8                 0b01111111
#define NR9                 0b01101111

    //shift 1 - 3
#define BIGMASK             0b001011111111111100011000
#define SMAMASK             0b000000000000000011100000
#define MASK                (BIGMASK|SMAMASK)

#define V                   0b000010010100010000000000
#define WFIRST              0b000000010101010100001000
#define WSECOND             0b000000000000000011000000
#define AFIRST              0b000001011000010100011000
#define ASECOND             0b000000000000000010100000

#define NNR0                0b000000010000011100011000
#define NNR1                0b000000000000000100001000
#define NNR2                0b000001001000010000011000
#define NNR3                0b000001001000000100011000
#define NNR4                0b000001011000000100001000
#define NNR5                0b000001011000001100010000
#define NNR6                0b000001011000011100010000
#define NNR7                0b000000000000000100011000
#define NNR8                0b000001011000011100011000
#define NNR9                0b000001011000001100011000

#define WAIT_T0H            NOP();NOP();NOP();
#define WAIT_T1H            NOP();NOP();NOP();NOP();NOP();NOP();
#define WAIT_T0L            NOP();NOP();NOP();NOP();NOP();NOP();NOP();
#define WAIT_T1L            NOP();NOP();NOP();NOP();

#define LED_LOWBIT()        STATUS_LED=1;WAIT_T0H;STATUS_LED=0;WAIT_T0L;
#define LED_HIGHBIT()       STATUS_LED=1;WAIT_T1H;STATUS_LED=0;WAIT_T1L;

    //read the time from tmr1
#define getTime()           (TMR1H<<8|TMR1L)

    void initPins();
    void initADC();
    void initTMR2();
    void initTMR1();
    void initFVR();
    void initPWMTMR4();
    void initCOMP1();
    void initCOMP2();
    uint8_t adc(const int8_t src);
    uint8_t readVoltage();
    float readCurrent();
    void so(const uint8_t data, const uint8_t direction);
    uint24_t combine(uint24_t nr1, uint24_t nr2);
    void clearDisplay(uint8_t leng);
    void interrupt ISR();
    void main();

#ifdef	__cplusplus
}
#endif

#endif

