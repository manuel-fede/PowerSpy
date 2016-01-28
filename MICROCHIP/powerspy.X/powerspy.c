/*
 * File:   power_spy.c
 * Author: redxef
 *
 * Created on 27 November 2015, 18:58
 */


#include <xc.h>
#include "powerspy.h"

/*
void init() {
    OSCCON = 0b01110000;
    PORTB = 0;
    ANSELB = 0;
    TRISB = 0b00000010;
    RXDTSEL = 0;
    TXCKSEL = 0;
    
    //Configure TX    RB2 = TX Pin
    TXEN = 1;
    SYNC = 0;
    SPEN = 1;

    CREN = 1;
    SYNC = 0;
    SPEN = 1;
    
    //SetBaudrate for BT Module to 9600
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 12;
}


void main(void) {
    init();
    for (;;) {
        if (RCIF) { //only if data has been received
            TXREG = RCREG;
            while(!TRMT); //wait until all has been sent
        }
    }
}
 */

/*
 * shift a char of data out of the pic
 * the latch is not handled
 * datapin: DISPLAY_DATA
 * clockpin: DISPLAY_CLK
 */
void so(const char data, const char direction) {
    signed char c;
    if (direction) {
        for (c = CHAR_LEN - 1; c >= 0; c--) {
            DISPLAY_DATA = (data >> c) & 0x01;
            DISPLAY_CLK = 1;
            DISPLAY_CLK = 0;
        }
    } else {
        for (c = 0; c < CHAR_LEN; c++) {
            DISPLAY_DATA = (data >> c) & 0x01;
            DISPLAY_CLK = 1;
            DISPLAY_CLK = 0;
        }
    }
}

/******************************************************************************/
/*init methods*****************************************************************/
/******************************************************************************/

/*
 * prepare the processor, nothing may be turned on
 */
void init() {
    PORTB = 0b00000000; //we basically only have inputs --> no need to predefine the values of ports
    ANSELB = 0b00010000; //RB4 is analog
    TRISB = 0b00010001; //RB4 && RB0 are inputs

    PORTA = 0b00000000; //same as on portb
    ANSELA = 0b00000001; //we can use the same pin for both vals (RA0)
    TRISA = 0b00000001; //RA0 is ofc input

    nWPUEN = 0;
    WPUB = 0b00100000; //RB5 is a button and needs a pullup

    //see data sheet page 57
    //set the frequency to 4MHz
    IRCF0 = 0;
    IRCF1 = 1;
    IRCF2 = 0;
    IRCF3 = 1;
}

/*
 * set up the adc for an 8 bit conversion
 * with Vdd and Vss as references
 */


void initADC() {
    //see page 139 of datasheet
    //left justify, we have a resolution of 10 bit, 8 bit in ADRESH
    ADFM = 0;

    //see data sheet page 146
    //set the conversion clock speed to FOSC/2
    ADCS0 = 0;
    ADCS1 = 0;
    ADCS2 = 0;

    //see data sheet page 146 || 139
    //set the references to Vdd and Vss
    ADPREF0 = 0;
    ADPREF1 = 0;
    ADNREF = 0;
}

void initTMR2() {
    /*
     * prescaler vals for timers 2/4/6
     * overflow_freq(*) = fosc_val/UCHAR_MAX
     * 
     * *calculated with _XTAL_FREQ = 4_000_000
     *  
     * |TxCKPS1|TxCKPS0|PRESCALER|FOSC_VAL|OVERFLOW_FREQ|RATE@50|
     * |-------|-------|---------|--------|-------------|-------|
     * |      0|      0|      1:1|  FOSC/4|       15_625|  312.5|
     * |      0|      1|      1:4| FOSC/16|     3_906.25| 78.125|
     * |      1|      0|     1:16| FOSC/64|      976.562|19.5312|
     * |      1|      1|     1:64|FOSC/256|      244.141|4.88282|
     * 
     */

    //see data sheet page 189 & 191
    //set the prescaler to 1:4 to get a rate of ~80
    T2CKPS0 = 1;
    T2CKPS1 = 0;

    //see data sheet page 189 & 191
    //set the postscaler to 1:1
    T2OUTPS0 = 0;
    T2OUTPS1 = 0;
    T2OUTPS2 = 0;
    T2OUTPS3 = 0;

    //see data sheet page 190
    //defaults to 0xff on reset, i want to be clear of what i want
    PR2 = 0xff;


    TMR2IE = 1;
    TMR2IF = 0;

    //see data sheet page 191
    TMR2ON = 1;
}

/******************************************************************************/
/*core methods*/
/******************************************************************************/

/*
 * give the desired channel an<n>
 * where n ranges from 0 to 11
 * 
 * src: the desired channel
 */
char adc(char src) {
    //select the channel
    //since the bit combination is sorted we can do this:
    CHS0 = (src >> 0) & 0x01;
    CHS1 = (src >> 1) & 0x01;
    CHS2 = (src >> 2) & 0x01;
    CHS3 = (src >> 3) & 0x01;
    CHS4 = (src >> 4) & 0x01;

    //convert
    ADON = 1;
    GO_nDONE = 1;
    while (GO_nDONE);
    ADON = 0;

    return ADRESH;
}

/******************************************************************************/
/*convenient methods***********************************************************/
/******************************************************************************/

/*
 * src is an char ranging from 0 to 11
 * 
 * |SOURCE|REGISTER_PIN|EXTERNAL PIN/18|
 * |------|------------|---------------|
 * |AN0   |RA0         |17             |
 * |AN1   |RA1         |18             |
 * |AN2   |RA2         | 1             |
 * |AN3   |RA3         | 2             |
 * |AN4   |RA4         | 3             |
 * |AN5   |RB6         |12             |
 * |AN6   |RB7         |13             |
 * |AN7   |RB5         |11             |
 * |AN8   |RB4         |10             |
 * |AN9   |RB3         | 9             |
 * |AN10  |RB2         | 8             |
 * |AN11  |RB1         | 7             |
 */

/*
 * unused as of now, will probably not be implemented
 */
char readVoltage() {
    return -1;
}

/*
 * returns the current in amps as float
 * this as of now doesn't read
 * at the peak of the signal
 */

//note: check the value for negative currents

float readCurrent() {
    float amps;
    //remove the offset (2.5 volts means 0 amps)
    adc(0);
    //function for current: curr(volt)=10*volt-25
    //note: this only applies, when the processor is powered with 5Volts
    return 10.0 * ADRESH - 25.0;
}

/*
 * well, clears the display
 */
void clearDisplay(signed char leng) {
    DISPLAY_LAT = 0;
    for (; leng >= 0; leng--)
        so(0, SHIFT_DIR_LSBFIRST);
    DISPLAY_LAT = 1;
}

/******************************************************************************/
/*main program*****************************************************************/

/******************************************************************************/


void interrupt ISR() {
    //rate for reading data
    if (TMR2IF) {
        TMR2IF = 0;
        if (!TMR2IE) return;

    }
}

/*
 * main method, guess what it does!
 */
void main() {
    init();
    clearDisplay(SHIFT_REG_LEN);
    initADC();
    initTMR2();
}