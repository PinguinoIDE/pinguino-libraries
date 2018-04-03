/*	----------------------------------------------------------------------------
    FILE:           delayus.c
    PROJECT:        pinguino
    PURPOSE:        pinguino delays functions
    PROGRAMER:      jean-pierre mandon
    FIRST RELEASE:  2008
    ----------------------------------------------------------------------------
    CHANGELOG:
    * 2015-01-17    rblanchot - delays are now based on System_getPeripheralFrequency
    * 2016-05-02    rblanchot - delays use now _cpu_clock_
    ----------------------------------------------------------------------------
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
    --------------------------------------------------------------------------*/

#ifndef __DELAYUS_C__
#define __DELAYUS_C__

#include <compiler.h>
#include <typedef.h>
#include <macro.h>

//extern u32 _cpu_clock_;

// Cycles per second = FOSC/4000000

void Delayus(u16 us)
{
    //u8 status = isInterrupts();
    //if (status) noInterrupts();    
    #ifdef __XC8__
    while(--us)
    {
        nop();
        nop();
        nop();
    }
    #else
    // valid up to 40.000 us
    if (us > 1000)
    {
        us = us / 3;
        us = us * 5;
    }
    while(--us);
    #endif
    //if (status) interrupts();    
}

#endif // __DELAYUS_C__ 
