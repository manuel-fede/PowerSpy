/*
 * File:   power_spy.c
 * Author: redxef
 *
 * Created on 27 November 2015, 18:58
 * 
 * All information regarding correlation between pins, registers and such
 * has been taken from microchips data sheet for PIC16(L)F1826/27 (DS41391D)
 */


#include <xc.h>
#include <limits.h>
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

/******************************************************************************/
/*init methods*****************************************************************/
/******************************************************************************/

/*
 * prepare the processor, nothing may be turned on
 */
void init() {
    //see Pinlayout.md
    PORTA = 0b00000000;
    ANSELA = 0b00000111;
    TRISA = 0b00000111;
    
    PORTB = 0b00000000;
    ANSELB = 0b00110000;
    TRISB = 0b00111010;

    nWPUEN = 0;
    WPUB = 0b00000001; //RB0 is a button and needs a pullup

    //see data sheet page 57
    //set the frequency to 4MHz
    IRCF0 = 1;
    IRCF1 = 0;
    IRCF2 = 1;
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

void initTMR1() {
    /*
     * see data sheet page 177
     * set the clock source
     * |TMR1CS1|TMR1CS0|              CLK SRC|
     * |------:|------:|--------------------:|
     * |      0|      0|               FOSC/4|
     * |      0|      1|                 FOSC|
     * |      1|      0|External CLK on T1CKI|
     * |      1|      1|          Cap. S. OSC|
     */

    TMR1CS0 = 0;
    TMR1CS1 = 0;

    //set prescaler, ranges from 1 - 8
    //we have a frequency of 4MHz (FOSC/4 = 1MHz) --> dt = 1/1MHz = 1us (real time, nice)
    T1CKPS0 = 0;
    T1CKPS1 = 0;

    TMR1IE = 1;
    TMR1IF = 0;

    TMR1ON = 1;
}

void initFVR() {
    /*
     * |CDAFVR1|CDAFVR0|VOLTS|
     * |------:|------:|----:|
     * |      0|      0|1.024|
     * |      0|      1|2.048|
     * |      1|      0|4.096|
     * |      1|      1|    -|
     */
    //set FVR Buffer2 for comparing
    CDAFVR0 = 0;
    CDAFVR1 = 0;
    FVREN = 1; //enable ref
    while (!FVRRDY); //wait until its ready
}

//see data sheet page 208 and 209
void initPWMTMR4() {
    //1
    TRISBbits.TRISB0 = 1; //disable output driver for RB0
    
    //2
    PR2 = 0xff; //set duty cycle
    
    //3
    CCP1CON = 0b00001100;
    
    //4
    //set the duty (1023 = 100%, 0 = 0%)
    //0x7f + 1 + 1 == 511 = 50%
    //8 higher bits
    CCPR1L = 0x7f;
    //2 lower bits
    DC1B0 = 1;
    DC1B1 = 1;
    //5
    //see data sheet page 227, tmr 4 is used
    C1TSEL0 = 1;
    C1TSEL1 = 0;
    TMR4IF = 0;
    
    //see data sheet page 189
    T4CKPS0 = 0;
    T4CKPS1 = 0;
    
    TMR4ON = 1;
    
    //6
    while (!TMR4IF); //wait until overflow occured
    
    TRISBbits.TRISB0 = 0;
}

/*
 * see data sheet page 164
 * input for both comparators
 * |C1NCH1|C1NCH0|    SRC|ON PIN|
 * |-----:|-----:|------:|-----:|
 * |     0|     0|C12IN0-|   RA0|
 * |     0|     1|C12IN1-|   RA1|
 * |     1|     0|C12IN2-|   RA2|
 * |     1|     1|C12IN3-|   RA3|
 */

/*
 * Comparator for phase measurement of the Voltage
 * uses FVR Buffer2
 */
void initCOMP1() {
    //select input channel
    //see data sheet page 164
    //C12IN1-
    C1NCH0 = 1;
    C1NCH1 = 0;


    /* 
     * |C1PCH1|C1PCH0|     SRC|ON PIN|
     * |-----:|-----:|-------:|-----:|
     * |     0|     0|   C1IN+|   RA3|
     * |     0|     1|     DAC|     -|
     * |     1|     0|FVR BUF2|     -|
     * |     1|     1|  C12IN+|   RA2|
     */
    //select compare source
    //see data sheet page 164
    //FVR BUF2
    C1PCH0 = 0;
    C1PCH1 = 1;

    //turn comp 1 on
    C1INTP = 1; //falling edge, since the edges are reversed
    C1IE = 1;
    C1ON = 1;
}

void initCOMP2() {
    //select input channel
    //see data sheet page 165
    //C12IN0-
    C2NCH0 = 0;
    C2NCH1 = 0;

    /* 
     * |C2PCH1|C2PCH0|     SRC|ON PIN|
     * |-----:|-----:|-------:|-----:|
     * |     0|     0|  C12IN+|   RA3|
     * |     0|     1|     DAC|     -|
     * |     1|     0|FVR BUF2|     -|
     * |     1|     1|     VSS|     -|
     */
    //select compare source
    //see data sheet page 165
    //C12IN+
    C2PCH0 = 0;
    C2PCH1 = 0;

    //turn comp 2 on
    C2INTP = 1; //falling edge, since the edges are reversed
    C2IE = 1;
    C2ON = 1;

}

/******************************************************************************/
/*core methods*****************************************************************/
/******************************************************************************/

/*
 * give the desired channel <n>
 * where n ranges from 0 to 11
 * 
 * src: the desired channel
 */
uint8_t adc(uint8_t src) {
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
     * select the channel
     * since the bit combination is sorted we can do this:
     */
    if (src > 11) return RET_NOK; //return errcode if not a valid src is specified

    CHS0 = (bit) (src >> 0) & 0x01;
    CHS1 = (bit) (src >> 1) & 0x01;
    CHS2 = (bit) (src >> 2) & 0x01;
    CHS3 = (bit) (src >> 3) & 0x01;
    CHS4 = (bit) (src >> 4) & 0x01;

    //convert
    ADON = 1;
    GO_nDONE = 1;
    while (GO_nDONE);
    ADON = 0;
    //the result is in ADRESH
    return RET_OK;
}

/******************************************************************************/
/*convenient methods***********************************************************/
/******************************************************************************/

/*
 * unused as of now, will probably not be implemented
 */
char readVoltage() {
    return 230;
}

/*
 * returns the current in amps as float
 * since the voltage is regulated, we can just
 * measure at any time
 */
float readCurrent() {
    /*
     * function for current: curr(volt)=10*volt-25
     * note: this only applies, when the processor is powered with 5Volts
     * to convert from unitless to 0-5V we need to multiply ADRESH with 5
     * and divide it thru 1024
     */
    adc(0);
    return (float) (50 * ADRESH) / 1024.0 - 25.0;
}

/*
 * shift a char of data out of the pic
 * the latch is not handled
 * datapin: DISPLAY_DATA
 * clockpin: DISPLAY_CLK
 */
void so(const char data, const char direction) {
    signed char c;
    if (direction) {
        for (c = CHAR_BIT - 1; c >= 0; c--) {
            DISPLAY_DATA = (data >> c) & 0x01;
            DISPLAY_CLK = 1;
            DISPLAY_CLK = 0;
        }
    } else {
        for (c = 0; c < CHAR_BIT; c++) {
            DISPLAY_DATA = (data >> c) & 0x01;
            DISPLAY_CLK = 1;
            DISPLAY_CLK = 0;
        }
    }
}

/*
 * well, clears the display
 */
void clearDisplay(byte leng) {
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
    if (TMR1IF) {
        TMR1IF = 0;
        if (!TMR2IE) return;
        //todo hande timer overflow
    }

    if (C1IF) {
        C1IF = 0;
        if (!C1IE) return;
        //todo save time
    }

    if (C2IF) {
        C2IF = 0;
        if (!C2IE) return;
        //todo save time
    }

    /*
    if (TMR2IF) {
        TMR2IF = 0;
        if (!TMR2IE) return;
    }
     */
}

/*
 * main method, guess what it does!
 */
void main() {
    init();
    //clearDisplay(SHIFT_REG_LEN);
    //initADC();
    //initTMR2();
    //initTMR1();
    //initFVR();
    initPWMTMR4();
    //initCOMP1();
    //initCOMP2();
}