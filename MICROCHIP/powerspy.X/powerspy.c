/*
 * File:   powerspy.c
 * Author: Manuel Federanko
 *
 * Created on 27 November 2015, 18:58
 * 
 * All information regarding correlation between pins, registers and such
 * has been taken from microchips data sheet for PIC16(L)F1826/27 (DS41391D)
 */


#include <xc.h>
#include <limits.h>
#include "types.h"
#include "math.h"
#include "powerspy.h"

/*
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

const uint8_t get_shift_byte[10] = {NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9};

char receive_buff[RECEIVEBUFF_SIZE] = {0};
int8_t buffpos = -1;
int8_t readpos = 0;

//those numbers have 1 byte more than the timer, since we also need negative numbers
//to signify unset values
int24_t volt_time = -1;
int24_t curr_time = -1;
int24_t delta_t = -1;
uint16_t led_rest = 0;
//int8_t ovflw;

/******************************************************************************
 *init methods*****************************************************************
 ******************************************************************************/

/*
 * prepare the processor, nothing may be turned on
 * according to Pinlayout version 1.4
 */
void initPins() {
    PORTA = 0b00000000;
    ANSELA = 0b00000111;
    TRISA = 0b00000111;

    PORTB = 0b00000000;
    ANSELB = 0b00110000;
    TRISB = 0b00111010;

    nWPUEN = 0;
    WPUB = 0b00001000;

    //see data sheet page 65 & 58 FOSC set in pragmas
    //set the frequency to 32MHz
    SCS0 = 0;
    SCS1 = 0;

    IRCF0 = 0;
    IRCF1 = 1;
    IRCF2 = 1;
    IRCF3 = 1;

    SPLLEN = 1;
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

/*
 * this timer might be used as a screen refresch rate generator
 * in a later release
 */
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

/*
 * used to measure real time for deltat and the led
 */
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
    //we have a frequency of 32MHz with prescale of 2 (FOSC/8 = 4MHz) --> dt = 1/4MHz = 250ns (real time, nice)
    T1CKPS0 = 1;
    T1CKPS1 = 0;

    TMR1IE = 1;
    TMR1IF = 0;

    TMR1ON = 1;
}

/*
 * used to compare to voltage signal
 */
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

/*
 * used to compare amps signal
 */
void initPWMTMR4() {

    //see data sheet page 119
    CCP1SEL = 1; //switch output from RB3 to RB0

    //1
    TRISBbits.TRISB0 = 1; //disable output driver for RB0

    //2
    PR2 = 0xff; //set duty cycle
    CCP1CON = 0b00110000; //lsbs of duty cycle for 10 bit thingy, data sheet page 226

    //3
    CCP1CON |= 0b00001100;

    //4
    //set the duty (1023 = 100%, 0 = 0%)
    //0x7f + 1 + 1 == 511 = 50%
    //8 higher bits
    CCPR1L = 0x8E; //Weil besser.... kommen auf 2.48V
    //2 lower bits
    DC1B0 = 0;
    DC1B1 = 0;
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
    //while (!TMR4IF); //wait until overflow occured
    TMR4IF = 0;

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
    C1IF = 0;
    C1ON = 1;
}

/*
 * Comparator for phase measurement of the Amps
 * uses Voltage reference generated by the pwm
 */
void initCOMP2() {
    //select input channel
    //see data sheet page 165
    //C12IN0-
    C2NCH0 = 0;
    C2NCH1 = 0;

    /* 
     * |C2PCH1|C2PCH0|     SRC|ON PIN|
     * |-----:|-----:|-------:|-----:|
     * |     0|     0|  C12IN+|   RA2|
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
    C2INTP = 1; //falling edge, since the edges are reversed, actually it doesn't matter, since we only need to use the same edges on both signals
    C2IE = 1;
    C2IF = 0;
    C2ON = 1;
}

/*
 * Init the USART module for communication with the bt module
 * the bauderate is 9600
 */
void initBT() {

    //select output pin
    RXDTSEL = 0;
    TXCKSEL = 0;

    //Configure TX
    TXEN = 1;
    SYNC = 0;
    SPEN = 1;

    //Configure RX
    CREN = 1;
    SYNC = 0;
    SPEN = 1;

    //Set Baudrate for BT Module to 9600
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 51; //Datasheet Page 299

    RCIE = 1;
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
uint8_t adc(const int8_t src) {
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
uint8_t readVoltage() {
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
     * and divide it thru 256 since we only use the 8 highest order bits
     */
    adc(0);
    return (float) (50 * ADRESH) / 256.0 - 25.0;
}

/*
 * shift a char of data out of the pic
 * the latch is not handled
 * datapin: DISPLAY_DATA
 * clockpin: DISPLAY_CLK
 */
void so(const uint8_t data, const uint8_t direction) {
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
 * checks if a pattern of characters is valid
 * doesnt work as of now
 */
uint24_t combine(uint24_t nr1, uint24_t nr2) {
    if (nr1 & nr2) return 0;
    if (MASK | ~nr1 != 0xffffff) return 0;
    if (MASK | ~nr2 != 0xffffff) return 0;
    return (nr1 | nr2);
}

/*
 * well, clears the display
 */
void clearDisplay(uint8_t leng) {
    DISPLAY_LAT = 0;
    for (; leng >= 0; leng--)
        so(0, SHIFT_DIR_LSBFIRST);
    DISPLAY_LAT = 1;
}

/*
 * sends one byte of colour information to the led
 * - highest bit first
 */
void sendColour(uint8_t c) {
    if (c & 0b10000000) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b01000000) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00100000) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00010000) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00001000) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00000100) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00000010) LED_HIGHBIT
    else LED_LOWBIT;

    if (c & 0b00000001) LED_HIGHBIT
    else LED_LOWBIT;
}

/******************************************************************************
 *main program*****************************************************************
 ******************************************************************************/

/*
 * compute the time difference, is only invoked by the interrupt service routine
 */
void deltaT(int24_t tm_low, int24_t tm_high) {
    di(); //dont do anything with the following values in the isr while they are reset
    volt_time = -1;
    curr_time = -1;
    //ovflw = -1;
    ei();

    if (tm_low < tm_high) {
        delta_t = tm_high - tm_low;
    } else {
        delta_t = 0x00ffff - tm_low + tm_high;
    }
}

int ledReset() {
    //because we have a prescale of 1:2 we can use only half of the required step count
    if (led_rest < getTime()) {
        if (getTime() - led_rest > 200) return 1;
        else return 0;
    } else if (led_rest > getTime()) {
        if (0xffff - led_rest + getTime() > 200) return 1;
        else return 0;
    }
    return 0;
}

/*
 * reprograms the led to the desired colour
 */
void setLED(uint8_t g, uint8_t r, uint8_t b) {
    sendColour(g);
    sendColour(r);
    sendColour(b);
    led_rest = getTime();
}

void interrupt ISR() {
    //usart data received
    if (RCIE && RCIF) {
        receive_buff[buffpos] = RCREG;
        buffpos++;
        if (buffpos == RECEIVEBUFF_SIZE) buffpos = 0;
        RCIF = 0;
    }

    //in us
    if (TMR1IE && TMR1IF) {
        //if (curr_time >= 0 && volt_time < 0) ovflw = 1;
        //if (volt_time >= 0 && curr_time < 0) ovflw = 1;
        TMR1IF = 0;
    }

    //volts
    if (C1IE && C1IF) {
        volt_time = getTime();
        if (curr_time >= 0) deltaT(curr_time, volt_time);

        C1IF = 0;
    }

    //amps
    if (C2IE && C2IF) {
        curr_time = getTime();
        if (volt_time >= 0) deltaT(volt_time, curr_time);

        C2IF = 0;
    }

    if (TMR2IE && TMR2IF) {

        TMR2IF = 0;
    }
}

int8_t charAvailable() {
    if (buffpos < readpos) {
        return 8 - readpos + buffpos;
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

/*
 * main method, guess what it does!
 */
void main() {
    float res = 1.2;
    di();
    initPins();

    //initADC();
    //initTMR1();
    //initFVR();
    //initPWMTMR4();
    //initCOMP1();
    //initCOMP2();
    initBT();

    PEIE = 1; //enable peripheral interrupts
    ei();
    while (1) {

        if (charAvailable()) {
            if (readNext() == 'g') {
                sendChar((char) 1.2 & 0xff);
                sendChar(((char) 1.2 >> 8) & 0xff);
                sendChar(((char) 1.2 >> 16) & 0xff);
            }
        }
    }

    //clearDisplay(SHIFT_REG_LEN);
}