/*
 * File:   blue.c
 * Author: redxef
 *
 * Created on 27 November 2015, 18:58
 */


#include <xc.h>

void init() {
    //default init
    OSCCON = 0b01110000;
    PORTB = 0;
    ANSELB = 0;
    TRISB = 0;
    RXDTSEL = 0;
    TXCKSEL = 0;

    //Configure TX    RB2 = TX Pin
    TXEN = 1;
    SYNC = 0;
    SPEN = 1;

    //Set Baudrate for BT Moduel to 9600
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 12;
}

void main() {
    init();
    char msg[] = "hello";
    while (1) {
        for (int i = 0; i < 5; i++) {
            TXREG = msg[i];
            //__delay_ms(100); probably won't need that
        }
        TXREG = '\n';
        __delay_ms(500);
    }
}
