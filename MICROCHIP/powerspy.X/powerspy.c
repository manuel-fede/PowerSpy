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
//#include <math.h>
#include "types.h"
#include "message.h"
#include "powerspy.h"
#include "intmath.h"

const uint8_t get_shift_byte[10] = {NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9};


//remember interrupt time for deltat
uint16_t volt_time = 0;
uint16_t curr_time = 0;

//0th bit: volts set
//1st bit: current set
//2nd bit: volts first
//3rd bit: current first
volatile uint8_t flag = 0;

//config for current measurement
float i_u_offs = -12.5;
float i_u_diode_offs = -0.04;

uint16_t led_rest = 0;

volatile uint8_t mode = DMODE_NONE;

/******************************************************************************
 *init methods*****************************************************************
 ******************************************************************************/

/*
 * prepare the processor, nothing may be turned on
 * according to Pinlayout version 1.4
 */
void initPins()
{
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

        //config int for RB5 (see data sheet page 81)
        IOCBN5 = 1;
        IOCIE = 1;
}

/*
 * set up the adc for an 8 bit conversion
 * with Vdd and Vss as references
 */
void initADC()
{
        //see page 139 of datasheet
        //left justify, we have a resolution of 10 bit, 8 bit in ADRESH
        ADFM = 0;

        //see data sheet page 146
        //set the conversion clock speed to FOSC/64
        ADCS0 = 0;
        ADCS1 = 1;
        ADCS2 = 1;

        //see data sheet page 146 || 139
        //set the references to Vdd and Vss
        ADPREF0 = 0;
        ADPREF1 = 0;
        ADNREF = 0;

        ADON = 1;
}

/*
 * this timer might be used as a screen refresch rate generator
 * in a later release
 */
void initTMR2()
{
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
void initTMR1()
{
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
void initFVR()
{
        /*
         * |CDAFVR1|CDAFVR0|VOLTS|
         * |------:|------:|----:|
         * |      0|      0|1.024|
         * |      0|      1|2.048|
         * |      1|      0|4.096|
         * |      1|      1|    -|
         */

        //set the fvr for adc to 1.024 volts
        ADFVR0 = 1;
        ADFVR1 = 0;
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
void initPWMTMR4()
{

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
        CCPR1L = 0x7f; //Weil besser.... kommen auf 2.48V

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
void initCOMP1()
{
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
void initCOMP2()
{
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
void initBT()
{

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

        while (RCIF) {
                RCREG;
        }

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
void adc(const int8_t src)
{
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

        CHS0 = (bit) (src >> 0) & 0x01;
        CHS1 = (bit) (src >> 1) & 0x01;
        CHS2 = (bit) (src >> 2) & 0x01;
        CHS3 = (bit) (src >> 3) & 0x01;
        CHS4 = (bit) (src >> 4) & 0x01;

        __delay_us(5);

        //convert
        GO_nDONE = 1;
        while (GO_nDONE);
        //the result is in ADRESH and ADRESL
}

/******************************************************************************/
/*convenient methods***********************************************************/
/******************************************************************************/

/*
 * unused as of now, will probably not be implemented
 */
uint8_t readVoltage()
{
        return 230;
}

/*
 * returns the current in amps as float
 * since the voltage is regulated, we can just
 * measure at any time
 */
float readCurrent()
{
        /*
         * function for current: curr(volt)=5*volt-12.5
         * note: this only applies, when the processor is powered with 5Volts
         * to convert from unitless to 0-5V we need to multiply ADRESH with 5
         * and divide it thru 256 since we only use the 8 highest order bits
         */
        ADFM = 1;
        adc(7);

        return (ADRES * 5.0 / 1024.0 + i_u_diode_offs) * 5.0 + i_u_offs;
}

float readVdd()
{
        ADFM = 1;
        adc(0b00011111); //fvr buffer output

        return (1024.0 / ADRES) * 1.024;
}

/*
 * shift a char of data out of the pic
 * the latch is not handled
 * datapin: DISPLAY_DATA
 * clockpin: DISPLAY_CLK
 */
void so(const uint8_t data)
{
        uint8_t c;
        for (c = 0; c < CHAR_BIT; c++) {
                DISPLAY_DATA = (data >> c) & 0x01;
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                DISPLAY_CLK = 1;
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                DISPLAY_CLK = 0;
        }
}

/*
 * well, clears the display
 */
void clearDisplay(int8_t leng)
{
        for (; leng >= 0; leng--)
                so(0xff);
}

uint8_t setNr(uint16_t nr)
{
        uint8_t it = 0;
        while (nr) {
                so(get_shift_byte[nr % 10]);
                nr /= 10;
                it++;
        }
        return it;
}

/*
 * sends one byte of colour information to the led
 * - highest bit first
 */
void sendColour(uint8_t c)
{
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
 * the flag byte should be cleared afterwards
 */
uint16_t deltaT(uint16_t tm_low, uint16_t tm_high)
{
        if (tm_low < tm_high) //no overflow bc obv
                return tm_high - tm_low;
        else //no overflow bc now tm_low >= tm_high
                return 0xffff - tm_low + tm_high;
}

uint8_t ledReset()
{
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
void setLED(uint8_t g, uint8_t r, uint8_t b)
{
        sendColour(g);
        sendColour(r);
        sendColour(b);
        led_rest = getTime();
}

void interrupt ISR()
{
        //usart data received
        if (RCIE && RCIF) {
                receive_buff[buffpos] = RCREG;
                buffpos++;
                buffpos %= RECEIVEBUFF_SIZE;
                RCIF = 0;
        }

        //in us
        if (TMR1IE && TMR1IF) {
                TMR1IF = 0;
        }

        //volts
        if (C1IE && C1IF) {
                if (!(flag & 0x01)) { //is volt not set
                        volt_time = getTime();
                        if (flag & 0x02) //is current set
                                flag |= 0x08;
                        flag |= 0x01;
                }

                C1IF = 0;
        }

        //amps
        if (C2IE && C2IF) {
                if (!(flag & 0x02)) { //is current not set
                        curr_time = getTime();
                        if (flag & 0x01) //is volt set
                                flag |= 0x04;
                        flag |= 0x02;
                }

                C2IF = 0;
        }

        if (TMR2IE && TMR2IF) {
                TMR2IF = 0;
        }

        if (IOCBN5 && IOCBF5) {
                mode++;
                mode %= DMODE_MAX;

                setLED((mode & 0x01) << 3, (mode & 0x02) << 3, (mode & 0x04) << 2);
                IOCBF5 = 0;
        }
}

/*
 * main method, guess what it does!
 */
void main()
{
        uint24_t angle;
        float current;
        float voltage;
        float apparent;
        float real;
        float reactive;
        uint16_t so_buff;

        PEIE = 0;
        GIE = 0;

        initPins();
        initFVR();
        initADC();
        initTMR1();
        initPWMTMR4();
        initCOMP1();
        initCOMP2();
        initBT();
        initMessaging();

        PEIE = 1;
        GIE = 0;

        while (1) {


                if ((flag & 0x02) && (flag & 0x01)) { //volts and current
                        if (flag & 0x04) //volts first
                                angle = (deltaT(volt_time, curr_time) >> 2); // (*250/1000) == (/4) == (>>2)
                        else if (flag & 0x08) //current first
                                angle = (deltaT(curr_time, volt_time) >> 2);

                        flag = 0;

                        angle /= 1000;
                        angle *= FULL_ROTATION * 50;
                        //we want to preserve accuracy, thus we need to multiply now
                        //if we did it before the abore division, we would have an overflow
                        //below we would likely have 0 bc the result would be a fraction
                        angle /= 1000;

                        i_u_offs = -readVdd()* 5.0 / 2.0;
                        current = readCurrent();
                        voltage = readVoltage();
                        apparent = voltage * current;
                        real = (apparent * icos(angle)) / MAX_SIN_RES;
                        reactive = (apparent * isin(angle)) / MAX_SIN_RES;

                        DISPLAY_LAT = 0;
                        switch (mode) {
                                case DMODE_NONE:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = 5;
                                        break;
                                case DMODE_CURRENT:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = ASECOND;
                                        so(so_buff & 0xff);
                                        so(so_buff >> 8 & 0xff);
                                        so(so_buff >> 16 & 0xff);
                                        so_buff = setNr((uint16_t) current);
                                        break;
                                case DMODE_VOLTAGE:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = V;
                                        so(so_buff & 0xff);
                                        so(so_buff >> 8 & 0xff);
                                        so(so_buff >> 16 & 0xff);
                                        so_buff = setNr((uint16_t) voltage);
                                        break;
                                case DMODE_APPARENT:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = V & ASECOND;
                                        so(so_buff & 0xff);
                                        so(so_buff >> 8 & 0xff);
                                        so(so_buff >> 16 & 0xff);
                                        so_buff = setNr((uint16_t) apparent);
                                        break;
                                case DMODE_REAL:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = WSECOND;
                                        so(so_buff & 0xff);
                                        so(so_buff >> 8 & 0xff);
                                        so(so_buff >> 16 & 0xff);
                                        so_buff = setNr((uint16_t) real);
                                        break;
                                case DMODE_REACTIVE:
                                        clearDisplay(SHIFT_REG_LEN);
                                        so_buff = V & AFIRST & ASECOND;
                                        so(so_buff & 0xff);
                                        so(so_buff >> 8 & 0xff);
                                        so(so_buff >> 16 & 0xff);
                                        so_buff = setNr((uint16_t) reactive);
                                        break;
                        }

                        //fill the last few registers in order to position the thingies correctly
                        while (SHIFT_REG_LEN - 2 - so_buff) {
                                so(0xff);
                                so_buff--;
                        }
                        DISPLAY_LAT = 1;

                        sendChar(K_CURRENT);
                        sendFloat((current < 0) ? -current : current);
                        sendChar(K_APPARENTEPOWER);
                        sendFloat((apparent < 0) ? -apparent : apparent);
                        sendChar(K_REALPOWER);
                        sendFloat((real < 0) ? -real : real);
                        sendChar(K_REACTIVEPOWER);
                        sendFloat((reactive < 0) ? -reactive : reactive);
                }

                GIE = 1;
                __delay_ms(100);
                GIE = 0;
        }
}