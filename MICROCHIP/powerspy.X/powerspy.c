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
#include "message.h"
#include "powerspy.h"

//sin in scale of 0 to 100
__EEPROM_DATA(0, 2, 3, 5, 6, 8, 9, 11);
__EEPROM_DATA(13, 14, 16, 17, 19, 20, 22, 23);
__EEPROM_DATA(25, 26, 28, 29, 31, 32, 34, 35);
__EEPROM_DATA(37, 38, 40, 41, 43, 44, 45, 47);
__EEPROM_DATA(48, 50, 51, 52, 54, 55, 56, 58);
__EEPROM_DATA(59, 60, 61, 63, 64, 65, 66, 67);
__EEPROM_DATA(68, 70, 71, 72, 73, 74, 75, 76);
__EEPROM_DATA(77, 78, 79, 80, 81, 82, 83, 84);
__EEPROM_DATA(84, 85, 86, 87, 88, 88, 89, 90);
__EEPROM_DATA(90, 91, 92, 92, 93, 94, 94, 95);
__EEPROM_DATA(95, 96, 96, 96, 97, 97, 98, 98);
__EEPROM_DATA(98, 99, 99, 99, 99, 99, 100, 100);
__EEPROM_DATA(100, 100, 100, 100, 100, 0, 0, 0);

const uint8_t get_shift_byte[10] = {NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9};


//remember interrupt time for deltat
uint16_t volt_time = 0;
uint16_t curr_time = 0;

//0th bit: volts set
//1st bit: current set
//2nd bit: volts first
//3rd bit: current first
//4th bit: button
volatile uint8_t flag = 0;

//config for current measurement
int24_t i_u_offs = -12500;
uint8_t i_u_diode_offs = 7;

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
int24_t readCurrent()
{
        /*
         * function for current: curr(volt)=5*volt-12.5
         * note: this only applies, when the processor is powered with 5Volts
         * to convert from unitless to 0-5V we need to multiply ADRESH with 5
         * and divide it thru 256 since we only use the 8 highest order bits
         */
        ADFM = 1;
        adc(7);

        //return (ADRES * 5000 / 1024 + i_u_diode_offs) * 5.0 + i_u_offs;
        //measured val * value range / scale + offset
        //return ((ADRES + i_u_diode_offs) * 5000) / 1024 + i_u_offs;
        return (1000 * (5 * (ADRES + i_u_diode_offs) + i_u_offs)) / 1024;
}

uint16_t readVdd()
{
        ADFM = 1;
        adc(0b00011111); //fvr buffer output

        //return (1024.0 / ADRES) * 1.024;
        // flt_vdd = (1024.0/ ADRES) * 1.024
        // int1000 = (1024 * 1000) / ADRES
        //return (1024000) / ADRES
        return ADRES<<2;
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
                SHIFT_DELAY
                SHIFT_DELAY
                SHIFT_DELAY
                SHIFT_DELAY
                DISPLAY_CLK = 1;
                SHIFT_DELAY
                SHIFT_DELAY
                SHIFT_DELAY
                SHIFT_DELAY
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

int8_t sin_(int8_t z)
{
        return eeprom_read(z);
}

//z is in the range of 0 ... 100

int8_t sin(int16_t z)
{
        //us /= 50; //convert from us to winkel grad (not deg)
        int16_t buff;
        while (z > FULL_ROTATION)
                z -= FULL_ROTATION;

        if (z > (HALF_ROTATION) + QUARTER_ROTATION) { //4th quad
                buff = FULL_ROTATION;
                buff -= z;
                return -sin_((int8_t) buff);
        }
        if (z > HALF_ROTATION) {//3rd quad
                buff = z;
                buff -= HALF_ROTATION;
                return -sin_((int8_t) buff);
        }
        if (z > QUARTER_ROTATION) {//2nd quad
                buff = HALF_ROTATION;
                buff -= z;
                return sin_((int8_t) buff);
        }

        //1st quad
        return sin_((int8_t) z);
}

int8_t cos(int16_t us)
{
        return sin(QUARTER_ROTATION + us);
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

void setUnit(uint24_t u)
{
        so(u >> 16 & 0xff);
        so(u >> 8 & 0xff);
        so(u & 0xff);
}

void setVal(int16_t v)
{
        int8_t i;
        for (i = 0; i < SHIFT_REG_LEN - 3; i++) {
                so(get_shift_byte[v % 10]);
                v /= 10;
        }
}

void __interrupt ISR()
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
}

/*
 * main method, guess what it does!
 */
void main()
{
        int bmode = DMODE_NONE;
        int24_t angle;
        int24_t current;
        int24_t voltage;
        int24_t apparent;
        int24_t real;
        int24_t reactive;
        int8_t i;

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

        clearDisplay(SHIFT_REG_LEN);

        while (1) {

                GIE = 1;
                __delay_ms(50);
                GIE = 0;


                //calculate the values and send only of both phases have been recorded
                if ((flag & 0x02) && (flag & 0x01)) { //volts and current
                        if (flag & 0x04) //volts first
                                angle = (deltaT(volt_time, curr_time) >> 2); // (*250/1000) == (/4) == (>>2)
                        else if (flag & 0x08) //current first
                                angle = (deltaT(curr_time, volt_time) >> 2);

                        flag &= 0xf0;

                        //angle now in us

                        //d = -5/2 vdd
                        i_u_offs = readVdd() * 5;
                        i_u_offs >>= 1;
                        i_u_offs = -i_u_offs;
                        
                        sendUInt8(K_RAWVOLTAGE);
                        sendInt24(ADRES);
                        
                        current = readCurrent();
                        voltage = readVoltage();
                        apparent = voltage * current;
                        real = (apparent * cos(angle)) / MAX_SIN_RES;
                        reactive = (apparent * sin(angle)) / MAX_SIN_RES;


                        sendUInt8(K_RAWCURRENT);
                        sendInt24(ADRES);
                        sendUInt8(K_OFFS);
                        sendInt24(i_u_offs);
                        sendUInt8(K_CURRENT);
                        sendInt24(current);
                        sendUInt8(K_APPARENTEPOWER);
                        sendInt24(apparent);
                        sendUInt8(K_REALPOWER);
                        sendInt24(real);
                        sendUInt8(K_REACTIVEPOWER);
                        sendInt24(reactive);

                        DISPLAY_LAT = 0;
                        switch (bmode) {
                                case DMODE_NONE:
                                default:
                                        setUnit(UNIT_NONE);
                                        setVal(0);
                                        setLED(0x00, 0x00, 0x00); //out
                                        break;
                                case DMODE_CURRENT:
                                        setUnit(UNIT_A);
                                        setVal(current / 1000);
                                        setLED(0x00, LED_INTENSE, 0x00); //red
                                        break;
                                case DMODE_REAL:
                                        setUnit(UNIT_W);
                                        setVal(real / 1000);
                                        setLED(LED_INTENSE, 0x00, 0x00); //green
                                        break;
                                case DMODE_APPARENT:
                                        setUnit(UNIT_VA);
                                        setVal(apparent / 1000);
                                        setLED(0x00, 0x00, LED_INTENSE); //blue
                                        break;
                                case DMODE_REACTIVE:
                                        setUnit(UNIT_VA);
                                        setVal(reactive / 1000);
                                        setLED(LED_INTENSE, 0x00, LED_INTENSE); //green + blue
                                        break;
                                case DMODE_VOLTAGE:
                                        setUnit(UNIT_V);
                                        setVal(voltage);
                                        setLED(LED_INTENSE, LED_INTENSE, 0x00); //green + red
                                        break;
                        }
                        DISPLAY_LAT = 1;
                }


                if (!BUTTON && !(flag & 0x10)) { //button has not been set
                        bmode++;
                        bmode %= DMODE_MAX;

                        flag |= 0x10;
                        __delay_ms(10);
                } else {
                        flag &= ~0x10;
                }
        }
}
