/*  ----------------------------------------------------------------------------
    FILE:           watchdog.c
    PROJECT:        pinguino
    PURPOSE:        pinguino system functions
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  5 Jan. 2011
    LAST RELEASE:   6 Oct. 2015
    ----------------------------------------------------------------------------
    CHANGELOG:
    * 6 Oct. 2015 - RB - changed syntax to be the same on both P8 and P32
                       - added 16F1459 support
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

#ifndef __WATCHDOG_C
#define __WATCHDOG_C

#define __WATCHDOG__

#include <compiler.h>
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <oscillator.c>

u8 boot_from_watchdog = 0;

#define Watchdog_enable()               (WDTCONbits.SWDTEN = 1)
#define Watchdog_disable()              (WDTCONbits.SWDTEN = 0)
#define Watchdog_clear()                clrwdt()

// TO: Watchdog Time-out Flag bit
// 1 = Set by power-up, CLRWDT instruction or SLEEP instruction
// 0 = A WDT time-out occurred

#if defined(__16F1459)
    #define Watchdog_readPostscaler()   (32 * (1 << WDTCONbits.WDTPS))
    #define Watchdog_clearEvent()       (PCONbits.nRWDT = 1)
    #define Watchdog_readEvent()        (!PCONbits.nRWDT)
#else
    #define Watchdog_readPostscaler()   (1 << (System_readFlashMemory(__CONFIG2H)))
    #define Watchdog_clearEvent()       (RCONbits.TO = 1)
    #define Watchdog_readEvent()        (!RCONbits.TO)
#endif

#define Watchdog_event()                (boot_from_watchdog)

void watchdog_init()
{
    // enable watchdog (8.2 seconds)
    if (Watchdog_readEvent())
        boot_from_watchdog = 1;
    //Watchdog_enable();
    Watchdog_disable();
    Watchdog_clearEvent();
    Watchdog_clear();
}

#endif /* __WATCHDOG_C */

