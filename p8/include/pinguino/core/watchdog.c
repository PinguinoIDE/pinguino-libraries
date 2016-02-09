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
#include <flash.c>

#if defined(__16F1459)
#define _1MS_ 0b00000 // 1:32 (Interval 1 ms nominal)
#define _2MS_ 0b00001 // 1:64 (Interval 2 ms nominal)
#define _4MS_ 0b00010 // 1:128 (Interval 4 ms nominal)
#define _8MS_ 0b00011 // 1:256 (Interval 8 ms nominal)
#define _16MS_ 0b00100 // 1:512 (Interval 16 ms nominal)
#define _32MS_ 0b00101 // 1:1024 (Interval 32 ms nominal)
#define _64MS_ 0b00110 // 1:2048 (Interval 64 ms nominal)
#define _128MS_ 0b00111 // 1:4096 (Interval 128 ms nominal)
#define _256MS_ 0b01000 // 1:8192 (Interval 256 ms nominal)
#define _512MS_ 0b01001 // 1:16384 (Interval 512 ms nominal)
#define _1S_ 0b01010 // 1:32768 (Interval 1s nominal)
#define _2S_ 0b01011 // 1:65536 (Interval 2s nominal) (Reset value)
#define _4S_ 0b01100 // 1:131072 (2 17 ) (Interval 4s nominal)
#define _8S_ 0b01101 // 1:262144 (2 18 ) (Interval 8s nominal)
#define _16S_ 0b01110 // 1:524288 (2 19 ) (Interval 16s nominal)
#define _32S_ 0b01111 // 1:1048576 (2 20 ) (Interval 32s nominal)
#define _64S_ 0b10000 // 1:2097152 (2 21 ) (Interval 64s nominal)
#define _128S_ 0b10001 // 1:4194304 (2 22 ) (Interval 128s nominal)
#define _256S_ 0b10010 // 1:8388608 (2 23 ) (Interval 256s nominal)
#endif

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
    #define Watchdog_readPostscaler()   (1 << (Flash_read(__CONFIG2H)))
    #define Watchdog_clearEvent()       (RCONbits.TO = 1)
    #define Watchdog_readEvent()        (!RCONbits.TO)
#endif

#define Watchdog_event()                (boot_from_watchdog)

void watchdog_init()
{
    #if defined(__16F1459)
    // 
    WDTCONbits.WDTPS  = _1S_; //_4MS_;
    #endif
    // enable watchdog (8.2 seconds)
    if (Watchdog_readEvent())
        boot_from_watchdog = 1;
    //Watchdog_enable();
    Watchdog_disable();
    Watchdog_clearEvent();
    Watchdog_clear();
}

#endif /* __WATCHDOG_C */

