/*  --------------------------------------------------------------------
    FILE:       serial.c
    PROJECT:    Pinguino
    PURPOSE:    UART Library for 8-bit Pinguino
    PROGRAMER:  Jean-Pierre MANDON 2008 jp.mandon@free.fr
    --------------------------------------------------------------------
    CHANGELOG:
    23-11-2012  Regis Blanchot - added __18f120,1320,14k22,2455,4455,46j50 support
    19-01-2013  Regis Blanchot - support of all clock frequency
    14-04-2014  Regis Blanchot - added printNumber and printFloat function
    24-11-2015  Regis Blanchot - added PIC16F1459 support
    28-01-2016  Andre Gentric  - fixed getString
    09-05-2017  Regis Blanchot - added multi-module support
    13-09-2017  Regis Blanchot - readChar returns -1 if nothing is received
    20-09-2017  Regis Blanchot - fixed Serial_printChar (Serial_write)
    12-12-2017  Regis Blanchot - added Serial_printX function
    15-02-2018  Regis BLanchot - added Serial_printChar2(u8 module, u8 c);
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
    ------------------------------------------------------------------*/

#ifndef __SERIAL__
#define __SERIAL__

#include <compiler.h>
#include <typedef.h>        // u8, u16, ...
#include <const.h>          // UARTSW, UART1 and UART2
#include <macro.h>
#include <serial.h>
#include <stdarg.h>         // variadic functions
#include <interrupt.h>      // timers definitions
#include <digitalp.c>       // pinmode
#include <digitalw.c>       // digitalwrite
#include <digitalr.c>       // digitalread
//#include <digitalt.c>       // toggle
//#include <delayms.c>        // Delayms
//#include <delayus.c>        // Delayus
//#include <stdlib.h>       // no longer used (09-11-2012)
//#include <oscillator.c>

// Printf
#if defined(SERIALPRINTF)  || defined(SERIALPRINTFSW) || \
    defined(SERIALPRINTF1) || defined(SERIALPRINTF2)
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(SERIALPRINTFLOAT)  || defined(SERIALPRINTX)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT) || \
    defined(SERIALPRINTX)
    #include <printNumber.c>
#endif

// RX buffer length
#ifndef UART_RXBUFFERLENGTH
    #if defined(__16F1459) || \
        defined(__18f1220) || defined(__18f1320) || \
        defined(__18f14k22)
        #define UART_RXBUFFERLENGTH 64
    #else
        #define UART_RXBUFFERLENGTH 128
    #endif
#endif

// Current CPU clock frequency
extern u32 _cpu_clock_;

// Write and read pointer
#if defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

volatile u8 UART_Wpointer[3], UART_Rpointer[3];         // UARTSW, UART1 and UART2
volatile char UART_RxBuffer[3][UART_RXBUFFERLENGTH];    // Serial buffer

#else

volatile u8 UART_Wpointer[2], UART_Rpointer[2];         // UARTSW and UART1 only
volatile char UART_RxBuffer[2][UART_RXBUFFERLENGTH];    // Serial buffer

#endif

#ifdef SERIALUSEPORTSW
volatile u8 UART_BitPointer = 0;
volatile u8 UART_RxFlag = 0;
volatile u8 UART_DelayFlag = 0;
volatile u8 UART_HalfBitDelayFlag = 0;
volatile u8 UART_HalfBitDelayH;
volatile u8 UART_HalfBitDelayL;
int UART_RXpin;
int UART_TXpin;
#endif

#if defined(SERIALUSEPORT1) || defined(SERIALUSEPORT2)
u8 UART_SPBRGH;
u8 UART_SPBRGL;
#endif
 
u8 UART_Module;                         // Current active UART module
u8 UART_Data;

/***********************************************************************
 * Serial.begin()
 * Setup PIC18F UART
 * Software UART :
 * Each bit has a fixed time duration determined by the transmission rate
 * 1200 bps (bits per second) UART will have a 1/1200s bit width (833.33us)
 * 2400 bps (bits per second) UART will have a 1/2400s bit width (416.67us)
 * 4800 bps (bits per second) UART will have a 1/4800s bit width (208.33us)
 * 9600 bps (bits per second) UART will have a 1/9600s bit width (104.17us)
 * spbrg = number of cycles to complete the bit width 
 **********************************************************************/

//void Serial_begin(int module, int baudrate, ...)
void Serial_begin(int module, u32 baudrate, va_list args)
{
    //va_list args;
    u16 UART_HalfBitDelay;
    u16 spbrg = (u16)Serial_baudRateDivisor(_cpu_clock_, (u16)baudrate);
    
    #if defined(SERIALUSEPORT1) || defined(SERIALUSEPORT2)
    UART_SPBRGH = high8(spbrg);
    UART_SPBRGL =  low8(spbrg);
    #endif

    UART_Module = (u8)module;
    
    /******************************************************************/

    #if defined(SERIALUSEPORTSW)
    #ifdef __XC8__
    UART_HalfBitDelay = spbrg/2 - 7;
    #else
    UART_HalfBitDelay = spbrg/2 - 7;
    #endif
    UART_HalfBitDelayH = high8(0xFFFF - UART_HalfBitDelay);
    UART_HalfBitDelayL =  low8(0xFFFF - UART_HalfBitDelay);
    #endif
    
    /******************************************************************/

    #if defined(SERIALUSEPORTSW)
    if (UART_Module == UARTSW)
    {
        // Get the arguments
        UART_TXpin = va_arg(args, int);
        UART_RXpin = va_arg(args, int);

        // Setup pins direction
        pinmode(UART_TXpin, OUTPUT);
        pinmode(UART_RXpin, INPUT);

        // Line idling
        digitalwrite(UART_TXpin, UARTIDLEBIT);

        // Configure the Timer interrupt for half 1/baudrate
        noInterrupts();
          
        TMR1H = UART_HalfBitDelayH;
        TMR1L = UART_HalfBitDelayL;
        T1CON = T1_ON | T1_PS_1_1;

        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        T1GCON = 0;
        #endif

        #ifndef __16F1459
        IPR1bits.TMR1IP = 1;    // INT_HIGH_PRIORITY
        #endif

        PIR1bits.TMR1IF = 0;    // Setting flag to 0
        PIE1bits.TMR1IE = 1;    // INT_ENABLE
        
        interrupts();
    }
    #endif // SERIALUSEPORTSW

    /******************************************************************/

    #if defined(SERIALUSEPORT1)
    if (UART_Module == UART1)
    {
        #if defined(__16F1459)
        
            // 8-bit asynchronous operation
            RCSTA = 0;                  // 8-bit RX (RX9=0)
            TXSTA = 0;                  // 8-bit TX (TX9=0), asynchronous (SYNC=0)
            BAUDCON = 0;                // polarity : non-inverted
            
            // IO's
            TRISBbits.TRISB5 = 1;       // RX is an input
            //TRISBbits.TRISB7 = 0;       // see SPEN bit below

            // Baud Rate
            SPBRGH = UART_SPBRGH;       // set UART speed SPBRGH
            SPBRGL = UART_SPBRGL;       // set UART speed SPBRGL
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

            // 8-bit asynchronous operation
            RCSTA = 0;                  // 8-bit RX (RX9=0)
            TXSTA = 0;                  // 8-bit TX (TX9=0), asynchronous (SYNC=0)
            BAUDCON = 0;                // polarity : non-inverted

            TXSTAbits.BRGH=1;           // set BRGH bit
            BAUDCTLbits.BRG16=1;        // set 16 bits SPBRG
            SPBRGH=UART_SPBRGH;         // set UART speed SPBRGH
            SPBRG=UART_SPBRGL;          // set UART speed SPBRGL
            RCSTA=0x90;                 // set RCEN and SPEN
            BAUDCTLbits.RCIDL=1;        // set receive active
            TXSTAbits.TXEN = 1;         // enable TX

            // Enable RX interrupt
            PIR1bits.RCIF = 0;         // Clear RX interrupt flag
            IPR1bits.RCIP = 1;         // Define high priority for RX interrupt
            PIE1bits.RCIE = 1;         // Enable interrupt on RX

        #elif defined(__18f2455)  || defined(__18f4455)  || \
              defined(__18f2550)  || defined(__18f4550)  || \
              defined(__18f25k50) || defined(__18f45k50)

            // 8-bit asynchronous operation
            RCSTA = 0;                  // 8-bit RX (RX9=0)
            TXSTA = 0;                  // 8-bit TX (TX9=0), asynchronous (SYNC=0)
            BAUDCON = 0;                // polarity : non-inverted

            // IO's
            TRISCbits.TRISC7= 1;        // RX as input
            //TRISCbits.TRISC6= 0;        // TX as output
            
            // Baud Rate
            SPBRGH=UART_SPBRGH;         // set UART speed SPBRGH
            SPBRG=UART_SPBRGL;          // set UART speed SPBRGL
            TXSTAbits.BRGH=1;           // High Baud Rate
            BAUDCONbits.BRG16=1;        // use 16 bits SPBRG

            BAUDCONbits.RCIDL=1;        // set receive active
            // Enable EUSART
            TXSTAbits.TXEN = 1;         // Transmit Enabled
            RCSTAbits.CREN = 1;         // Receiver Enabled
            RCSTAbits.SPEN = 1;         // Serial Port Enabled (RX/TX pins as input/output)

            // Enable RX interrupt
            PIR1bits.RCIF = 0;         // Clear RX interrupt flag
            IPR1bits.RCIP = 1;         // Define high priority for RX interrupt
            PIE1bits.RCIE = 1;         // Enable interrupt on RX

        #elif defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53)

            // IO's
            TRISCbits.TRISC7= 1;        // Rx1    set input

            // Baud Rate
            SPBRGH1=UART_SPBRGH;        // set UART speed SPBRGH
            SPBRG1=UART_SPBRGL;         // set UART speed SPBRGL
            TXSTA1bits.BRGH=1;          // set BRGH bit
            BAUDCON1bits.BRG16=1;       // set 16 bits SPBRG

            BAUDCON1bits.RCIDL=1;       // set receive active
            // Enable EUSART
            TXSTA1bits.TXEN=1;          // Transmit Enabled
            RCSTA1=0x90;                // set RCEN and SPEN

            // Enable RX interrupt
            PIR1bits.RC1IF = 0;         // Clear RX interrupt flag
            IPR1bits.RC1IP = 1;         // Define high priority for RX interrupt
            PIE1bits.RC1IE = 1;         // Enable interrupt on RX

            //PIR1bits.TX1IF = 0;         // Clear TX interrupt flag
            //PIE1bits.TX1IE = 0;         // Disable TX interrupt

        #else

            #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

        #endif
    }
    #endif // SERIALUSEPORT1

    /******************************************************************/

    #if defined(SERIALUSEPORT2)
    if (UART_Module == UART2)
    {
        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)

            // IO's
            // The EUSART control will automatically reconfigure the pin
            // from input to output as needed.
            
            TRISBbits.TRISB0= 0;        // TX2 (output)
            TRISBbits.TRISB1= 1;        // TX2 (input)

            // Baud Rate
            SPBRGH2=UART_SPBRGH;        // set UART speed SPBRGH
            SPBRG2=UART_SPBRGL;              // set UART speed SPBRGL
            TXSTA2bits.BRGH=1;          // set BRGH bit
            BAUDCON2bits.BRG16=1;       // set 16 bits SPBRG

            BAUDCON2bits.RCIDL=1;       // set receive active
            // Enable EUSART
            TXSTA2bits.TXEN=1;          // Transmit Enabled
            RCSTA2=0x90;                // set RCEN and SPEN

            // Enable RX interrupt
            PIR3bits.RC2IF = 0;         // Clear RX interrupt flag
            IPR3bits.RC2IP = 1;         // Define high priority for RX interrupt
            PIE3bits.RC2IE = 1;         // Enable interrupt on RX

            //PIR1bits.TX1IF = 0;         // Clear TX interrupt flag
            //PIE1bits.TX1IE = 0;         // Disable TX interrupt

        #else

            #error "This processor does not have a UART2."

        #endif
    }
    #endif // SERIALUSEPORT2

    //va_end(args);                   // cleans up the list

    UART_Wpointer[UART_Module] = 1;       // initialize write pointer
    UART_Rpointer[UART_Module] = 1;       // initialize read pointer

    interrupts();                   // Enable global interrupts
    
    //Delayms(1000);                  // AG : 12-11-2012
}

#if defined(SERIALUSEPORTSW)
void SerialSW_begin(u32 baudrate, ...)
{
    va_list args;
    va_start(args, baudrate);
    Serial_begin(UARTSW, baudrate, args);
    va_end(args);
}
#endif // SERIALUSEPORTSW

/***********************************************************************
 * Serial.available()
 * return true if a new character has been received, otherwise false
 **********************************************************************/

//#if defined(SERIALAVAILABLE) || defined(SERIALAVAILABLE1) || defined(SERIALAVAILABLE2) 
#define Serial_available(module)    (UART_Wpointer[module] != UART_Rpointer[module])

/***********************************************************************
 * Serial.flush()
 * Clear RX buffer
 **********************************************************************/

void Serial_flush(u8 module)
{
    UART_Wpointer[module] = 1;
    UART_Rpointer[module] = 1;
}

/***********************************************************************
 * Serial.sendBit()
 * Write a bit on Software Serial port
 **********************************************************************/

#ifdef SERIALUSEPORTSW
void Serial_writeBit(u8 b)
{
    // Wait until the Half Bit Delay Flag is set
    while (!UART_HalfBitDelayFlag);
    UART_HalfBitDelayFlag = 0;

    // Write a bit
    digitalwrite(UART_TXpin, b);

    // Wait until the Half Bit Delay Flag is set
    while (!UART_HalfBitDelayFlag);
    UART_HalfBitDelayFlag = 0;
}
#endif // SERIALUSEPORTSW

/***********************************************************************
 * Serial.printChar()
 * Write a char on Serial port
 **********************************************************************/

void Serial_printChar(u8 module, u8 c)
{
    #ifdef SERIALUSEPORTSW
    u8 i;

    if (module == UARTSW)
    {
        Serial_writeBit(UARTIDLEBIT);    // Line idling
        Serial_writeBit(UARTSTARTBIT);   // Start bit

        // Send 8 bits of data, lsb first
        for (i = 0; i < 8; i++)
            Serial_writeBit((c >> i) & 0x01);
        
        Serial_writeBit(UARTSTOPBIT);    // Stop bit
    }
    #endif // SERIALUSEPORTSW

    /******************************************************************/

    #ifdef SERIALUSEPORT1

    #if defined(__16F1459)  || \
        defined(__18f1220)  || defined(__18f1320)   || \
        defined(__18f14k22) || defined(__18lf14k22) || \
        defined(__18f2455)  || defined(__18f4455)   || \
        defined(__18f2550)  || defined(__18f4550)   || \
        defined(__18f25k50) || defined(__18f45k50)

    if (module == UART1)
    {
        while (!TXSTAbits.TRMT);        // Ready ?
        TXREG = c;                      // yes, send char
    }
        
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f27j53) || defined(__18f47j53)

    if (module == UART1)
    {
        while (!TXSTA1bits.TRMT);       // Ready ?
        TXREG1 = c;                     // yes, send char
    }

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT1

    /******************************************************************/

    #ifdef SERIALUSEPORT2

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f27j53) || defined(__18f47j53)

    if (module == UART2)
    {
        while (!TXSTA2bits.TRMT);   // Ready ?
        TXREG2 = c;                 // yes, send char
    }

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT2
}

/***********************************************************************
 * Serial.printChar2()
 * Write a char on Serial port
 **********************************************************************/

void Serial_printChar2(u8 c)
{
    #ifdef SERIALUSEPORTSW
    u8 i;

    if (UART_Module == UARTSW)
    {
        Serial_writeBit(UARTIDLEBIT);    // Line idling
        Serial_writeBit(UARTSTARTBIT);   // Start bit

        // Send 8 bits of data, lsb first
        for (i = 0; i < 8; i++)
            Serial_writeBit((c >> i) & 0x01);
        
        Serial_writeBit(UARTSTOPBIT);    // Stop bit
    }
    #endif // SERIALUSEPORTSW

    /******************************************************************/

    #ifdef SERIALUSEPORT1

    #if defined(__16F1459)  || \
        defined(__18f1220)  || defined(__18f1320)   || \
        defined(__18f14k22) || defined(__18lf14k22) || \
        defined(__18f2455)  || defined(__18f4455)   || \
        defined(__18f2550)  || defined(__18f4550)   || \
        defined(__18f25k50) || defined(__18f45k50)

    if (UART_Module == UART1)
    {
        while (!TXSTAbits.TRMT);        // Ready ?
        TXREG = c;                      // yes, send char
    }
        
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f27j53) || defined(__18f47j53)

    if (UART_Module == UART1)
    {
        while (!TXSTA1bits.TRMT);       // Ready ?
        TXREG1 = c;                     // yes, send char
    }

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT1

    /******************************************************************/

    #ifdef SERIALUSEPORT2

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f27j53) || defined(__18f47j53)

    if (UART_Module == UART2)
    {
        while (!TXSTA2bits.TRMT);   // Ready ?
        TXREG2 = c;                 // yes, send char
    }

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT2
}

/*
void printChar(u8 c)
{
    Serial_printChar(c);
}
*/

/***********************************************************************
 * USB SERIAL print routine (SERIAL.print)
 * 16-08-2011: fixed bug in print - Régis Blanchot & Tiew Weng Khai
 * 11-09-2012: added FLOAT support - Régis Blanchot
 * 04-03-2014: updated (no more printf call) - Régis Blanchot 
 * write a string on SERIAL port
 **********************************************************************/

#if defined(SERIALPRINTSTRING) || defined(SERIALPRINTLN)    || \
    defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT) || \
    defined(SERIALPRINTX)

void Serial_print(u8 module, const char *s)
{
    //UART_Module = module;
    while (*s)
        Serial_printChar(module, *s++);
}

#endif /* SERIALPRINTSTRING ... */

/***********************************************************************
 * USB SERIAL printX routine (SERIAL.printX)
 * added by regis blanchot on 09/12/2017
 * useful mixed print function with a smaller footprint than printf
 * writes a string followed by a number and jump to the next line
 **********************************************************************/

#if defined(SERIALPRINTX)

void Serial_printX(u8 module, const char *s, s32 value, u8 base)
{
    Serial_print(module, s);
    if (base == BIN)
        Serial_print(module, (const char *)"0b");
    if (base == HEX)
        Serial_print(module, (const char *)"0x");
    if (base == FLOAT)
        Serial_printFloat(module, (float)value, 2);
    else
        Serial_printNumber(module, value, base);
    Serial_print(module, (const char *)"\n\r");
}
        
#endif /* SERIALPRINTX ... */

/***********************************************************************
 * USB SERIAL print routine (SERIAL.println)
 * added by regis blanchot on 04/03/2014
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on SERIAL port
 **********************************************************************/

#if defined(SERIALPRINTLN)
void Serial_println(u8 module, const char *string)
{
    const char * ln = "\n\r";
    Serial_print(module, string);
    Serial_print(module, ln);
}
#endif /* SERIALPRINTLN */

/***********************************************************************
 * USB SERIAL printNumber routine (SERIAL.printNumber)
 * added by regis blanchot on 14/06/2011
 * write a number on SERIAL port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT) || \
    defined(SERIALPRINTX)
void Serial_printNumber(u8 module, s32 value, u8 base)
{  
    UART_Module = module;
    printNumber(Serial_printChar2, value, base);
}
#endif /* SERIALPRINTNUMBER */

/***********************************************************************
 * USB SERIAL printFloat routine (SERIAL.printFloat)
 * added by regis blanchot on 14/06/2011
 * write a float number on SERIAL port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(SERIALPRINTFLOAT) || defined(SERIALPRINTX)
void Serial_printFloat(u8 module, float number, u8 digits)
{ 
    UART_Module = module;
    printFloat(Serial_printChar2, number, digits);
}
#endif /* SERIALPRINTFLOAT */

/***********************************************************************
 * Serial.printf
 * updated by regis blanchot <rblanchot@gmail.com> on 14/04/2014
 * write a formated string on Serial port
 **********************************************************************/

#if defined(SERIALPRINTF)
void Serial_printf(u8 module, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    UART_Module = module;
    pprintf(Serial_printChar2, fmt, args);
    va_end(args);
}
#endif /* SERIALPRINTF */

#if defined(SERIALPRINTFSW)
void SerialSW_printf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    UART_Module = UARTSW;
    pprintf(Serial_printChar2, fmt, args);
    va_end(args);
}
#endif /* SERIALPRINTFSW */

#if defined(SERIALPRINTF1)
void Serial1_printf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    UART_Module = UART1;
    pprintf(Serial_printChar2, fmt, args);
    va_end(args);
}
#endif /* SERIALPRINTF1 */

#if defined(SERIALPRINTF2)
void Serial2_printf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    UART_Module = UART2;
    pprintf(Serial_printChar2, fmt, args);
    va_end(args);
}
#endif /* SERIALPRINTF2 */

/***********************************************************************
 * Serial.getBit()
 * Read a bit on Software Serial port
 **********************************************************************/

#if defined(SERIALREAD) || defined(SERIALGETKEY) || defined(SERIALGETSTRING)

#ifdef SERIALUSEPORTSW
#define Serial_readBit()    digitalread(UART_RXpin)
#endif // SERIALUSEPORTSW

#endif

/***********************************************************************
 * Serial.read()
 * Get a char from Serial port
 **********************************************************************/

#if defined(SERIALREAD) || defined(SERIALGETKEY) || defined(SERIALGETSTRING)

u8 Serial_readChar(u8 module)
{
    //#ifdef SERIALUSEPORTSW
    //u8 i, newwp;
    //#endif // SERIALUSEPORTSW

    UART_Data = 0;

    /******************************************************************/

    #ifdef SERIALUSEPORTSW
    if (module == UARTSW)
    {
        if (Serial_available(module))
        {
            UART_Data = UART_RxBuffer[module][UART_Rpointer[module]++];
            if (UART_Rpointer[module] == UART_RXBUFFERLENGTH)
                UART_Rpointer[module] = 1;

            return UART_Data;
        }
    }
    #endif // SERIALUSEPORTSW

    /******************************************************************/

    #ifdef SERIALUSEPORT1
    if (module == UART1)
    {
        if (Serial_available(module))
        {
            // Atomic operation start
            PIE1bits.RCIE = 0;

            UART_Data = UART_RxBuffer[module][UART_Rpointer[module]++];
            if (UART_Rpointer[module] == UART_RXBUFFERLENGTH)
                UART_Rpointer[module] = 1;

            // Atomic operation end
            PIE1bits.RCIE = 1;

            return UART_Data;
        }
    }
    #endif // SERIALUSEPORT1

    /******************************************************************/

    #ifdef SERIALUSEPORT2
    if (module == UART2)
    {
        //if (Serial_available(module))
        {
            // Atomic operation start
            PIE3bits.RC2IE = 0;

            UART_Data = UART_RxBuffer[module][UART_Rpointer[module]++];
            if (UART_Rpointer[module] == UART_RXBUFFERLENGTH)
                UART_Rpointer[module] = 1;

            // Atomic operation end
            PIE3bits.RC2IE = 1;

            return UART_Data;
        }
    }
    #endif // SERIALUSEPORT2
    
    return (-1);
}
#endif /* SERIALREAD */

/***********************************************************************
 * Serial.getKey
 * updated by regis blanchot <rblanchot@gmail.com> - 14/04/2014
 * get a key from Serial port
 **********************************************************************/

#if defined(SERIALGETKEY) || defined(SERIALGETSTRING)
u8 Serial_getKey(u8 module)
{
    u8 c;
    
    while (!(Serial_available(module)));
    c = Serial_readChar(module);
    Serial_flush(module);
    return c;
}
#endif /* SERIALGETKEY__ */

/***********************************************************************
 * Serial.getString
 * updated by regis blanchot <rblanchot@gmail.com> - 14/04/2014
 * get a string from Serial port
 **********************************************************************/

#if defined(SERIALGETSTRING)
u8 * Serial_getString(u8 module)
{
    // static attribute to return local array
    static u8 buffer[80];
    u8 c;
    u8 i = 0;

    //buffer = (u8 *) malloc(80);
    do {
        c = Serial_getKey(module);
        //UART_Module = module;
        Serial_printChar(module, c);
        // 28-01-2016 - agentric - added if (c != '\r')
        if (c != '\r')
            buffer[i++] = c;
    } while (c != '\r');
    
    /*
    while (c != '\r')
    {
        c = Serial_getKey();
        Serial_printChar(c);
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
    u8 c, newwp;

    /******************************************************************/

    #if defined(SERIALUSEPORTSW)

    if (PIR1bits.TMR1IF)
    {
        // Enable interrupt again
        PIR1bits.TMR1IF = 0;

        // Reload the Timer registers
        TMR1H = UART_HalfBitDelayH;
        TMR1L = UART_HalfBitDelayL;

        // Set the HalfBitDelay flag
        UART_HalfBitDelayFlag = 1;
        
        if (UART_BitPointer == 0 && Serial_readBit() == UARTSTARTBIT)
            UART_RxFlag = 1;
        
        if (UART_RxFlag)
        {
            // Read data, one bit at a time
            c |= Serial_readBit() << UART_BitPointer++;

            if (UART_BitPointer == 7)
            {
                // Did we reach this end of the buffer ?
                if (UART_Wpointer[UARTSW] != UART_RXBUFFERLENGTH - 1)
                    newwp = UART_Wpointer[UARTSW] + 1;
                else
                    newwp = 1;

                // Store received char if read pointer != write pointer
                if (UART_Rpointer[UARTSW] != newwp)
                    UART_RxBuffer[UARTSW][UART_Wpointer[UARTSW]++] = c;

                // Go back to the begining of the buffer if the end has been reached
                if (UART_Wpointer[UARTSW] == UART_RXBUFFERLENGTH)
                    UART_Wpointer[UARTSW] = 1;
                
                c = 0;
                UART_RxFlag = 0;
                UART_BitPointer = 0;
            }
        }
    }

    #endif // SERIALUSEPORTSW

    /******************************************************************/

    #if defined(SERIALUSEPORT1)

    #if defined(__16F1459)  || \
        defined(__18f1220)  || defined(__18f1320)   || \
        defined(__18f14k22) || defined(__18lf14k22) || \
        defined(__18f2455)  || defined(__18f4455)   || \
        defined(__18f2550)  || defined(__18f4550)   || \
        defined(__18f25k50) || defined(__18f45k50)

    if (PIR1bits.RCIF)
    { 
        PIR1bits.RCIF=0;            // clear RX interrupt flag
        c = RCREG;                  // get received char

        // Did we reach the end of the buffer ?
        if (UART_Wpointer[UART1] != UART_RXBUFFERLENGTH - 1)
            newwp = UART_Wpointer[UART1] + 1;
        else
            newwp = 1;

        // Store received char if read pointer != write pointer
        if (UART_Rpointer[UART1] != newwp)
            UART_RxBuffer[UART1][UART_Wpointer[UART1]++] = c;

        // Go back to the begining of the buffer if the end has been reached
        if (UART_Wpointer[UART1] == UART_RXBUFFERLENGTH)
            UART_Wpointer[UART1] = 1;
    }

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)
 
    if (PIR1bits.RC1IF) 
    {
        PIR1bits.RC1IF=0;           // clear RX interrupt flag
        c = RCREG1;                 // get received char

        // Did we reach the end of the buffer ?
        if (UART_Wpointer[UART1] != UART_RXBUFFERLENGTH - 1)
            newwp = UART_Wpointer[UART1] + 1;
        else
            newwp = 1;

        // Store received char if read pointer != write pointer
        if (UART_Rpointer[UART1] != newwp)
            UART_RxBuffer[UART1][UART_Wpointer[UART1]++] = c;

        // Go back to the begining of the buffer if the end has been reached
        if (UART_Wpointer[UART1] == UART_RXBUFFERLENGTH)
            UART_Wpointer[UART1] = 1;
    }

    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT1

    /******************************************************************/

    #if defined(SERIALUSEPORT2)

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)

    if (PIR3bits.RC2IF) 
    {
        PIR3bits.RC2IF=0;           // clear RX interrupt flag
        c = RCREG2;                 // get received char

        // Did we reach this end of the buffer ?
        if (UART_Wpointer[UART2] < UART_RXBUFFERLENGTH)
            newwp = UART_Wpointer[UART2] + 1;
        else
            newwp = 1;

        // Store received char if read pointer != write pointer
        if (UART_Rpointer[UART2] != newwp)
            UART_RxBuffer[UART2][UART_Wpointer[UART2]++] = c;

        // Go back to the begining of the buffer if the end has been reached
        if (UART_Wpointer[UART2] == UART_RXBUFFERLENGTH)
            UART_Wpointer[UART2] = 1;
    }
    
    #else

        #error "Processor Not Yet Supported. Please, Take Contact with Developpers."

    #endif // Processor's list
    
    #endif // SERIALUSEPORT2
}

/**********************************************************************/

#endif /* __SERIAL__ */
