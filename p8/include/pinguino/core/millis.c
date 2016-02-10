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
    10 Feb. 2016 - Régis Blanchot - changed from Timer0 to Timer1 for PIC16F
                                    Timer1 is the only 16-bit timer available on 16F
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

#include <compiler.h>           // compatibility between SDCC and XC8
#include <typedef.h>            // u8, u32, ...
#include <macro.h>              // interrupts() and noInterrupts
#include <oscillator.c>         // System_getPeripheralFrequency()

volatile u32 _millis;
volatile u16 _PERIOD_;

void updateMillisReloadValue(void )   /* Call from System_setIntOsc() */
{
    /* Atomic operation */
    
    #if defined(__16F1459) || defined(__16F1708)
    
    PIE1bits.TMR1IE = 0;
    _PERIOD_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    PIE1bits.TMR1IE = 1;

    #else

    INTCONbits.TMR0IE = 0;
    _PERIOD_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    INTCONbits.TMR0IE = 1;

    #endif
}

/*  --------------------------------------------------------------------
    if Fosc = 48 MHz then Fosc/4 = 12MHz
    which means 12.E-06 cycles/sec = 12.000 cycles/ms
    if TMR0 is loaded with 65536 - 12000
    overload will occur after 12.000 cycles = 1ms
    ------------------------------------------------------------------*/

void millis_init(void)
{
    noInterrupts();             // Disable global interrupts
    
    _PERIOD_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);

    #if defined(__16F1459) || defined(__16F1708)

    T1CON = 0b00000001;         //T1_ON | T1_SOURCE_FOSCDIV4 | T1_PS_1_1;
    T1GCONbits.TMR1GE = 0;      // Ignore T1DIG effection 
    TMR1H = high8(_PERIOD_);
    TMR1L =  low8(_PERIOD_);
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;

    #else

    T0CON = 0b00001000;         //T0_ON | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
    TMR0H = high8(_PERIOD_);
    TMR0L =  low8(_PERIOD_);
    INTCON2bits.TMR0IP = 1;     //INT_HIGH_PRIORITY;
    INTCONbits.TMR0IF  = 0;
    INTCONbits.TMR0IE  = 1;     //INT_ENABLE;

    #endif

    interrupts();               // Enable global interrupts

    _millis = 0;
}

u32 millis()
{
    u32 temp;

    /* Atomic operation */

    #if defined(__16F1459) || defined(__16F1708)
    
    PIE1bits.TMR1IE = 0;
    temp = _millis;
    PIE1bits.TMR1IE = 1;

    #else

    INTCONbits.TMR0IE = 0;      //INT_DISABLE;
    temp = _millis;
    INTCONbits.TMR0IE = 1;      //INT_ENABLE;

    #endif

    return (temp);
}

// called by interruption service routine in main.c
void millis_interrupt(void)
{
    #if defined(__16F1459) || defined(__16F1708)
    
    if (PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        TMR1H = high8(_PERIOD_);
        TMR1L =  low8(_PERIOD_);
        _millis++;
    }

    #else

    if (INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        TMR0H = high8(_PERIOD_);
        TMR0L =  low8(_PERIOD_);
        _millis++;
    }

    #endif
}

#endif /* _MILLIS_C_ */
