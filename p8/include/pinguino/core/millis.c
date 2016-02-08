/*  --------------------------------------------------------------------
    FILE:           millis.c
    PROJECT:        pinguino
    PURPOSE:        timeline
    PROGRAMER:      Jean-pierre Mandon
                    Régis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  19 Sep. 2008
    LAST RELEASE:   27 Jan. 2016
    --------------------------------------------------------------------
    CHANGELOG :
            2011 - Régis Blanchot - added interrupt.c functions
    14 May. 2012 - JP Mandon      - changed long to u32 and Millis to millis
                                    thanks to Mark Harper]
    31 Jan. 2013 - Régis Blanchot - use of System_getPeripheralFrequency()
    09 Sep. 2015 - Régis Blanchot - added Pinguino 1459
    12 Dec. 2015 - Régis Blanchot - added __DELAYMS__ flag
    27 Jan. 2016 - Régis Blanchot - added PIC16F1708 support
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
    ------------------------------------------------------------------*/

#ifndef _MILLIS_C_
#define _MILLIS_C_

#include <compiler.h>
#include <typedef.h>
//#include <interrupt.h>
//#include <interrupt.c>
#if !defined(__16F1459) &&  !defined(__16F1708)
#include <oscillator.c>         // System_getPeripheralFrequency()
#endif

volatile u32 _millis;
#if defined(__16F1459) || defined(__16F1708)
volatile u8 _PR0_;
#else
volatile u16 _PR0_;
#endif

void updateMillisReloadValue(void )   /* Call from System_setIntOsc() */
{
    /* Atomic operation */
    INTCONbits.TMR0IE = 0;      //INT_DISABLE;
    #if defined(__16F1459) || defined(__16F1708)
    //_PR0_ = (0xFFFF - System_getPeripheralFrequency() / 1000) >> 8;
    _PR0_ = 69;
    #else
    _PR0_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    #endif
    INTCONbits.TMR0IE = 1;      //INT_ENABLE;
}

void millis_init(void)
{
//    intUsed[INT_TMR0] = INT_USED;

    /*
    T0CON = 0x80;               // TMR0 on, 16 bits counter, prescaler=2
    INTCON |= 0xA0;             // set GIE and TMR0IE
    */
    
    // if Fosc = 48 MHz then Fosc/4 = 12MHz
    // which means 12.E-06 cycles/sec = 12.000 cycles/ms
    // if TMR0 is loaded with 65536 - 12000
    // overload will occur after 12.000 cycles = 1ms
    
    #if defined(__16F1459) || defined(__16F1708)
    INTCONbits.GIE     = 0;     // Disable global interrupts
    #else
    INTCONbits.GIEH    = 0;     // Disable global HP interrupts
    INTCONbits.GIEL    = 0;     // Disable global LP interrupts
    #endif
    
    #if defined(__16F1459) || defined(__16F1708)
    OPTION_REG = 0b00000111;    // Clock source FOSC/4, prescaler 1:256
    //_PR0_ = (0xFFFF - System_getPeripheralFrequency() / 1000) >> 8;
    _PR0_ = 69;
    TMR0 = _PR0_;
    #else
    T0CON = 0b00001000;         //T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
    _PR0_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    TMR0H = high8(_PR0_);
    TMR0L =  low8(_PR0_);
    #endif
    
    #if !defined(__16F1459) && !defined(__16F1708)
    INTCON2bits.TMR0IP = 1;     //INT_HIGH_PRIORITY;
    #endif
    INTCONbits.TMR0IF  = 0;
    INTCONbits.TMR0IE  = 1;     //INT_ENABLE;

    #if defined(__16F1459) || defined(__16F1708)
    INTCONbits.GIE     = 1;     // Enable global interrupts
    #else
    T0CONbits.TMR0ON   = 1;
    INTCONbits.GIEH    = 1;     // Enable global HP interrupts
    INTCONbits.GIEL    = 1;     // Enable global LP interrupts
    #endif

    _millis = 0;
}

u32 millis()
{
    u32 temp;
    /* Atomic operation for multibyte value */
    INTCONbits.TMR0IE = 0;      //INT_DISABLE;
    temp = _millis;
    INTCONbits.TMR0IE = 1;      //INT_ENABLE;
    return (temp);
}

// called by interruption service routine in main.c    if (INTCONbits.TMR0IF)
void millis_interrupt(void)
{
    if (INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        #if defined(__16F1459) || defined(__16F1708)
        TMR0 = _PR0_;
        #else
        TMR0H = high8(_PR0_);
        TMR0L =  low8(_PR0_);
        #endif
        _millis++;
    }
}

#endif /* _MILLIS_C_ */
