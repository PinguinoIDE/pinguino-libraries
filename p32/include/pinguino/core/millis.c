/*  --------------------------------------------------------------------
    FILE:			millis.c
    PROJECT:		pinguinoX
    PURPOSE:		
    PROGRAMER:		jean-pierre mandon <jp.mandon@gmail.com>
    FIRST RELEASE:	19 Feb. 2011
    LAST RELEASE:	15 Jan. 2015
    --------------------------------------------------------------------
    05 Jul. 2012 gagabi Added support for GENERIC 32 bits boards
    06 Feb. 2012 Z Added volatile for _millis variable
    17 Apr. 2010 Regis Blanchot added millis=f(pbclk)
    26 Feb. 2013 malagas fixed tmr2 value to refresh on each interrupt for MX2X0 boards and micros() function
    15 Jan. 2015 Regis Blanchot fixed PIC32MX2xx support
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

#ifndef __MILLIS__
#define __MILLIS__

#ifdef TMR2INT
#error "TIMER2 interrupt is already used !"
#endif

#include <system.c>
#include <interrupt.c>

/**
 * variables accessed by the ISR must be be declared as "volatile"
 * to prevent caching.
 */

volatile u32 _millis = 0;
volatile u32 _micros;

/*  --------------------------------------------------------------------
    Init. Timer2 to overload every 1 ms
    --------------------------------------------------------------------
    TIMER2 increments on every PBCLK clock cycle
    Freq. = Nb cycles / sec
    Nb cycles / 1ms = Freq / 1000
    prescaler = 1:2 (PR2 is a 16-bit number)
    ------------------------------------------------------------------*/

void millis_init(void)
{
        IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
        IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
        IntClearFlag(INT_TIMER2);
        IntEnable(INT_TIMER2);

        T2CON = 0;
        TMR2 = 0;
        PR2 = GetPeripheralClock() / 1000 / 2;
        // start TIMER2, prescaler = 1:2
        T2CONSET = 0x8010;
        _millis = 0;
}

u32 millis()
{
    return (_millis);
}

u32 micros()
{
    _micros = 1000 * _millis + (1000 * TMR2) / PR2;
    return (_micros);
}

/*  ----------------------------------------------------------------------------
    TMR2 interrupt
    see also ISRwrapper.S
    --------------------------------------------------------------------------*/

void Timer2Interrupt()
{
    // is this an TIMER2 interrupt ?
    //if (IntGetFlag(INT_TIMER2))
    //{
        IntClearFlag(INT_TIMER2);
        _millis++;
    //}
}

#endif /* __MILLIS__ */
