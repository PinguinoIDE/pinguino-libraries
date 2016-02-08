/*  --------------------------------------------------------------------
    FILE:       serial.c
    PROJECT:    Pinguino
    PURPOSE:    UART Library for 8-bit Pinguino
    PROGRAMER:  Jean-Pierre MANDON 2008 jp.mandon@free.fr
    --------------------------------------------------------------------
    CHANGELOG:
    23-11-2012  rblanchot  added __18f120,1320,14k22,2455,4455,46j50 support
    19-01-2013  rblanchot  support of all clock frequency
    14-04-2014  rblanchot  added printNumber and printFloat function
    24-11-2015  rblanchot  added PIC16F1459 support
    28-01-2016  agentric   fixed getstring
    --------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2, or (at your option) any
    later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    In other words, you are welcome to use, share and improve this program.
    You are forbidden to forbid anyone else to use, share and improve
    what you give them.   Help stamp out software-hoarding!
    ------------------------------------------------------------------*/

#ifndef __SERIAL__
#define __SERIAL__

#include <compiler.h>
#include <typedef.h>
#include <macro.h>
//#include <stdlib.h>       // no longer used (09-11-2012)
#include <delayms.c>
#include <oscillator.c>

// Printf
#if defined(SERIALPRINTF)
    #include <printFormated.c>
    #include <stdarg.h>
#endif

// PrintFloat
#if defined(SERIALPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT)
    #include <printNumber.c>
#endif

//extern u32 _cpu_clock_

// RX buffer length
#ifndef RXBUFFERLENGTH
    #if defined(__16F1459) || \
        defined(__18f1220) || defined(__18f1320) || \
        defined(__18f14k22)
        #define RXBUFFERLENGTH 64
    #else
        #define RXBUFFERLENGTH 128
    #endif
#endif

#define BaudRateDivisor(f, b)   ((f/(4*b))-1)

char rx[RXBUFFERLENGTH];            // serial buffer
u8 wpointer=1,rpointer=1;           // write and read pointer

/***********************************************************************
 * Serial.begin()
 * Setup PIC18F UART
 **********************************************************************/

void Serial_begin(u32 baudrate)
{
    // TO FIX
    #if defined(__16F1459)
    u16 spbrg = (u16)BaudRateDivisor(48*1000000UL, baudrate);
    #else
    u16 spbrg = (u16)BaudRateDivisor(System_getCpuFrequency(), baudrate);
    #endif
    
    #if defined(__16F1459)
    
        // 8-bit asynchronous operation
        RCSTA = 0;                  // 8-bit RX (RX9=0)
        TXSTA = 0;                  // 8-bit TX (TX9=0), asynchronous (SYNC=0)
        BAUDCON = 0;                // polarity : non-inverted
        
        // IO's
        TRISBbits.TRISB5 = 1;       // RX is an input
        //TRISBbits.TRISB7 = 0;       // see SPEN bit below

        // Baud Rate
        SPBRGH = high8(spbrg);      // set UART speed SPBRGH
        SPBRGL = low8(spbrg);       // set UART speed SPBRGL
        TXSTAbits.BRGH = 1;         // High Baud Rate
        BAUDCONbits.BRG16 = 1;      // Use 16-bit baud rate generator

        // Enable EUSART
        TXSTAbits.TXEN = 1;         // Transmit Enabled
        RCSTAbits.CREN = 1;         // Receiver Enabled
        RCSTAbits.SPEN = 1;         // Serial Port Enabled (RX/TX pins as input/output)

        // Enable RX interrupt
        PIR1bits.RCIF = 0;         // Clear RX interrupt flag
        PIE1bits.RCIE = 1;         // Enable interrupt on RX

    #elif defined(__18f1220) || defined(__18f1320) || \
          defined(__18f14k22) || defined(__18lf14k22)

        TXSTAbits.BRGH=1;           // set BRGH bit
        BAUDCTLbits.BRG16=1;        // set 16 bits SPBRG
        SPBRGH=high8(spbrg);        // set UART speed SPBRGH
        SPBRG=low8(spbrg);          // set UART speed SPBRGL
        RCSTA=0x90;                 // set RCEN and SPEN
        BAUDCTLbits.RCIDL=1;        // set receive active
        TXSTAbits.TXEN = 1;         // enable TX

        // Enable RX interrupt
        PIR1bits.RCIF = 0;         // Clear RX interrupt flag
        IPR1bits.RCIP = 1;         // Define high priority for RX interrupt
        PIE1bits.RCIE = 1;         // Enable interrupt on RX

    #elif defined(__18f2455) || defined(__18f4455) || \
          defined(__18f2550) || defined(__18f4550) || \
          defined(__18f25k50) || defined(__18f45k50)

        TRISCbits.TRISC7= 1;        // Rx1    set input
        TXSTAbits.BRGH=1;           // set BRGH bit
        BAUDCONbits.BRG16=1;        // set 16 bits SPBRG
        SPBRGH=high8(spbrg);        // set UART speed SPBRGH
        SPBRG=low8(spbrg);          // set UART speed SPBRGL
        RCSTA=0x90;                 // set RCEN and SPEN
        BAUDCONbits.RCIDL=1;        // set receive active
        TXSTAbits.TXEN=1;           // enable TX

        // Enable RX interrupt
        PIR1bits.RCIF = 0;         // Clear RX interrupt flag
        IPR1bits.RCIP = 1;         // Define high priority for RX interrupt
        PIE1bits.RCIE = 1;         // Enable interrupt on RX

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

        TRISCbits.TRISC7= 1;        // Rx1    set input
        TXSTA1bits.BRGH=1;          // set BRGH bit
        BAUDCON1bits.BRG16=1;       // set 16 bits SPBRG
        SPBRGH1=high8(spbrg);       // set UART speed SPBRGH
        SPBRG1=low8(spbrg);         // set UART speed SPBRGL
        RCSTA1=0x90;                // set RCEN and SPEN
        BAUDCON1bits.RCIDL=1;       // set receive active
        TXSTA1bits.TXEN=1;          // enable TX

        // Enable RX interrupt
        PIR1bits.RC1IF = 0;         // Clear RX interrupt flag
        IPR1bits.RC1IP = 1;         // Define high priority for RX interrupt
        PIE1bits.RC1IE = 1;         // Enable interrupt on RX

        //PIR1bits.TX1IF = 0;         // Clear TX interrupt flag
        //PIE1bits.TX1IE = 0;         // Disable TX interrupt
    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif

    //wpointer=1;                     // initialize write pointer
    //rpointer=1;                     // initialize read pointer

    INTCONbits.PEIE = 1;            // Enable peripheral interrupts

    #if defined(__16F1459)
    INTCONbits.GIE  = 1;            // Enable global interrupts
    #else
    //RB: IPEN is managed both by bootloader and main.c
    //RCONbits.IPEN = 1;            //  enable interrupt priorities
    INTCONbits.GIEH = 1;            // Enable global HP interrupts
    INTCONbits.GIEL = 1;            // Enable global LP interrupts
    #endif
    
    //Delayms(1000);                  // AG : 12-11-2012
}

/***********************************************************************
 * Serial.available()
 * return true if a new character has been received, otherwise false
 **********************************************************************/

#define Serial_available()          (wpointer != rpointer)

/***********************************************************************
 * Serial.flush()
 * Clear RX buffer
 **********************************************************************/

#define Serial_flush()              { wpointer=1; rpointer=1; }

/***********************************************************************
 * Serial.write()
 * Write a char on Serial port
 **********************************************************************/

void printChar(u8 c)
{
    Serial_putchar(c);
}

void Serial_putchar(u8 caractere)
{
    #if defined(__16F1459)  || \
        defined(__18f1220)  || defined(__18f1320)   || \
        defined(__18f14k22) || defined(__18lf14k22) || \
        defined(__18f2455)  || defined(__18f4455)   || \
        defined(__18f2550)  || defined(__18f4550)   || \
        defined(__18f25k50) || defined(__18f45k50)

        //while (!PIR1bits.TXIF);     // Ready ?
        while (!TXSTAbits.TRMT);
        TXREG=caractere;            // yes, send char

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f27j53) || defined(__18f47j53)

        while (!TXSTA1bits.TRMT);   // Ready ?
        TXREG1=caractere;           // yes, send char

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif
}

/***********************************************************************
 * USB SERIAL print routine (SERIAL.print)
 * 16-08-2011: fixed bug in print - Régis Blanchot & Tiew Weng Khai
 * 11-09-2012: added FLOAT support - Régis Blanchot
 * 04-03-2014: updated (no more printf call) - Régis Blanchot 
 * write a string on SERIAL port
 **********************************************************************/

#if defined(SERIALPRINTSTRING) || defined(SERIALPRINTLN) || \
    defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT)

void Serial_print(const char *s)
{
    while (*s++)
        Serial_putchar(*s);
}
#endif /* SERIALPRINTSTRING */

/***********************************************************************
 * USB SERIAL print routine (SERIAL.println)
 * added by regis blanchot 04/03/2014
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on SERIAL port
 **********************************************************************/

#if defined(SERIALPRINTLN)
void Serial_println(const char *string)
{
    const char * ln = "\n\r";
    Serial_print(string);
    Serial_print(ln);
}
#endif /* SERIALPRINTLN */

/***********************************************************************
 * USB SERIAL printNumber routine (SERIAL.printNumber)
 * added by regis blanchot 14/06/2011
 * write a number on SERIAL port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT)
void printNumber(long value, u8 base)
{  
    Serial_printNumber(value, base);
}
#endif /* SERIALPRINTNUMBER */

/***********************************************************************
 * USB SERIAL printFloat routine (SERIAL.printFloat)
 * added by regis blanchot 14/06/2011
 * write a float number on SERIAL port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(SERIALPRINTFLOAT)
void printFloat(float number, u8 digits)
{ 
    Serial_printFloat(number, digits);
}
#endif /* SERIALPRINTFLOAT */

/***********************************************************************
 * Serial.printf
 * updated by regis blanchot <rblanchot@gmail.com> - 14/04/2014
 * write a formated string on Serial port
 **********************************************************************/

#if defined(SERIALPRINTF)
void Serial_printf(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    pprintf(Serial_putchar, fmt, args);
    va_end(args);
}
#endif /* SERIALPRINTF__ */

/***********************************************************************
 * Serial.read()
 * Get a char from Serial port
 **********************************************************************/

#if defined(SERIALREAD) || defined(SERIALGETKEY) || defined(SERIALGETSTRING)
u8 Serial_read()
{
    u8 caractere=0;

    if (Serial_available())
    {
        PIE1bits.RCIE = 0;             // Atomic operation start
        caractere = rx[rpointer++];
        if (rpointer == RXBUFFERLENGTH)
            rpointer = 1;
        PIE1bits.RCIE = 1;             // Atomic operation end
    }
    return(caractere);
}
#endif /* SERIALREAD */

/***********************************************************************
 * Serial.getKey
 * updated by regis blanchot <rblanchot@gmail.com> - 14/04/2014
 * get a key from Serial port
 **********************************************************************/

#if defined(SERIALGETKEY) || defined(SERIALGETSTRING)
u8 Serial_getkey()
{
    u8 c;
    while (!(Serial_available()));
    c = Serial_read();
    Serial_flush();
    return (c);
}
#endif /* SERIALGETKEY__ */

/***********************************************************************
 * Serial.getString
 * updated by regis blanchot <rblanchot@gmail.com> - 14/04/2014
 * get a string from Serial port
 **********************************************************************/

#if defined(SERIALGETSTRING)
u8 * Serial_getstring()
{
    // static attribute to return local array
    static u8 buffer[80];
    u8 c;
    u8 i = 0;

    //buffer = (u8 *) malloc(80);
    do {
        c = Serial_getkey();
        Serial_putchar(c);
        // 28-01-2016 - agentric - added if (c != '\r')
        if (c != '\r')
            buffer[i++] = c;
    } while (c != '\r');
    
    /*
    while (c != '\r')
    {
        c = Serial_getkey();
        Serial_putchar(c);
        buffer[i++] = c;
    } ;
     */
    buffer[i] = '\0';
    return (buffer);
}
#endif /* SERIALGETSTRING__ */

/***********************************************************************
 * Interruption routine called by main.c
 **********************************************************************/

void serial_interrupt(void)
{
    u8 caractere;
    u8 newwp;

    #if defined(__16F1459)  || \
        defined(__18f1220)  || defined(__18f1320)   || \
        defined(__18f14k22) || defined(__18lf14k22) || \
        defined(__18f2455)  || defined(__18f4455)   || \
        defined(__18f2550)  || defined(__18f4550)   || \
        defined(__18f25k50) || defined(__18f45k50)

    if (PIR1bits.RCIF)
    { 
        PIR1bits.RCIF=0;            // clear RX interrupt flag
        caractere=RCREG;            // take received char

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    if (PIR1bits.RC1IF) 
    {
        PIR1bits.RC1IF=0;           // clear RX interrupt flag
        caractere=RCREG1;           // take received char

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif

        if (wpointer!=RXBUFFERLENGTH-1)  // if not last place in buffer
            newwp=wpointer+1;       // place=place+1

        else
            newwp=1;                // else place=1

        if (rpointer!=newwp)        // if read pointer!=write pointer
            rx[wpointer++]=caractere;// store received char

        if (wpointer==RXBUFFERLENGTH)// if write pointer=length buffer
            wpointer=1;             // write pointer = 1
    }
}

/**********************************************************************/

#endif /* __SERIAL__ */
