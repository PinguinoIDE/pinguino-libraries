/*  ----------------------------------------------------------------------------
    FILE:           RF433MHz_Tx.c
    PROJECT:        Pinguino
    PURPOSE:        433MHz Wireless Transmitter Modules library
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG:
    2018-01-31 - Régis Blanchot -   first release
    --------------------------------------------------------------------
    TODO:
    --------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    ------------------------------------------------------------------*/

#ifndef __RF433MHZ_TX_C
#define __RF433MHZ_TX_C

#define RF433MHZTRANSMITTER

#if defined(RF433MHZRECEIVER) && defined(RF433MHZTRANSMITTER)
#error "I CAN'T BE RECEIVER AND TRANSMITTER AT THE SAME TIME"
#endif

#ifndef __PIC32MX__
#include <compiler.h>
#include <digitalw.c>
//#include <digitalr.c>
#include <digitalp.c>
#include <delayus.c>
//#include <delayms.c>
#else
#include <digitalw.c>
#include <delay.c>
#endif

#include <typedef.h>
#include <macro.h>
#include <stdarg.h>
#include <RF433MHz.h>
#include <manchester.c>

// Printf
#ifdef RF433MHZPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(RF433MHZPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT)
    #include <printNumber.c>
#endif

RF433MHZ_t rf;

/*  --------------------------------------------------------------------
    The 433.92 Mhz receivers have AGC, if no signal is present the gain
    will be set to its highest level.

    In this condition it will switch high to low at random intervals due
    to input noise. A CRO connected to the data line looks like 433.92
    is full of transmissions.

    Any ASK transmission method must first sent a capture signal of 101010........
    When the receiver has adjusted its AGC to the required level for the
    transmisssion the actual data transmission can occur.

    We send 14 0's 1010... It takes 1 to 3 10's for the receiver to
    adjust to the transmit level.

    The receiver waits until we have at least 10 10's and then a start
    pulse 01. The receiver is then operating correctly and we have
    locked onto the transmission.
    ------------------------------------------------------------------*/

void RF433MHz_init(u8 pin, u16 bauds)
{
    // TX a digital pin as output
    rf.TxPin = pin;
    pinmode(pin, OUTPUT); 

    // Baud rate is defined as the number of symbols sent or received per second
    rf.bauds = bauds;
    
    // Half a bit time in us
    // 1s = 1.000.000 us
    rf.half_bit_interval_us = 500000 / bauds;
}

void RF433MHz_sendZero(void)
{
    Delayus(rf.half_bit_interval_us);
    digitalwrite(rf.TxPin, HIGH);

    Delayus(rf.half_bit_interval_us);
    digitalwrite(rf.TxPin, LOW);
}

void RF433MHz_sendOne(void)
{
    Delayus(rf.half_bit_interval_us);
    digitalwrite(rf.TxPin, LOW);

    Delayus(rf.half_bit_interval_us);
    digitalwrite(rf.TxPin, HIGH);
}

// Start sequence
void RF433MHz_start(void)
{
    u8 i;
            
    for(i = 0; i < SYNC_PULSE_DEF; i++) //send capture pulses
    #if SYNC_BIT_VALUE
        RF433MHz_sendOne(); //end of capture pulses
    RF433MHz_sendZero(); //start data pulse
    #else
        RF433MHz_sendZero(); //end of capture pulses
    RF433MHz_sendOne(); //start data pulse
    #endif
}

// End sequence
// Send 3 terminatings 0's to correctly terminate the previous bit
// and to turn the transmitter off
void RF433MHz_end(void)
{
    #if SYNC_BIT_VALUE
    RF433MHz_sendOne();
    RF433MHz_sendOne();
    RF433MHz_sendOne();
    #else
    RF433MHz_sendZero();
    RF433MHz_sendZero();
    RF433MHz_sendZero();
    #endif
}

void RF433MHz_printChar(u8 c)
{
    u8 i, encoded[2];

    // A lot of zeroes can result to packet lost, 
    // therefore we xor the data with random decoupling mask
    c = c ^ DECOUPLING_MASK;
    Manchester_encode(c, encoded);
    for (i = 0; i < 8; i++)
        (encoded[0] & (1 << i)) ? RF433MHz_sendOne():RF433MHz_sendZero();
    for (i = 0; i < 8; i++)
        (encoded[1] & (1 << i)) ? RF433MHz_sendOne():RF433MHz_sendZero();
}

#if defined(RF433MHZWRITECHAR)
void RF433MHz_writeChar(u8 c)
{
    RF433MHz_start();
    RF433MHz_printChar(c);    
    RF433MHz_end();
}
#endif

#if defined(RF433MHZWRITEBYTES)
void RF433MHz_writeBytes(const u8 *data, u8 numBytes)
{
    u8 i;
            
    RF433MHz_start();
    for (i = 0; i < numBytes; i++)
        RF433MHz_printChar(data[i]);    
    RF433MHz_end();
}
#endif

#if defined(RF433MHZPRINT)
void RF433MHz_print(const u8 *string)
{
    RF433MHz_start();
    while (*string != 0)
        RF433MHz_printChar(*string++);
    RF433MHz_end();
}
#endif

#if defined(RF433MHZPRINTLN)
void RF433MHz_println(const u8 *string)
{
    RF433MHz_start();
    while (*string != 0)
        RF433MHz_printChar(*string++);
    RF433MHz_printChar('\r');
    RF433MHz_printChar('\n');
    RF433MHz_end();
}
#endif

#if defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT)
void RF433MHz_printNumber(long value, u8 base)
{
    RF433MHz_start();
    printNumber(RF433MHz_printChar, value, base);
    RF433MHz_end();
}
#endif

#if defined(RF433MHZPRINTFLOAT)
void RF433MHz_printFloat(float number, u8 digits)
{ 
    RF433MHz_start();
    printFloat(RF433MHz_printChar, number, digits);
    RF433MHz_end();
}
#endif

#if defined(RF433MHZPRINTF)
void RF433MHz_printf(const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    RF433MHz_start();
    while (*c)
        RF433MHz_printChar(*c++);
    RF433MHz_end();
}
#endif

#endif // __RF433MHZ_TX_C
