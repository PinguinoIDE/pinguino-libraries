/*  --------------------------------------------------------------------
    FILE:			delayms.c
    PROJECT:		pinguino
    PURPOSE:		pinguino delays functions
    PROGRAMER:		jean-pierre mandon
    FIRST RELEASE:	2008
    --------------------------------------------------------------------
    CHANGELOG:
    * 2013-01-17    rblanchot - delays are now based on SystemGetClock()
    * 2015-09-09    rblanchot - PIC16F / workaround to "undefined symbol: _Delay1KTCYx"
    * 2016-01-13    rblanchot - added #ifndef __DELAYMS__ if the lib is called from another one 
    * 2016-04-06    rblanchot - added Delay1TCYx, etc ... 
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

#ifndef __DELAYMS_C__
#define __DELAYMS_C__

#include <compiler.h>
#include <typedef.h>
#include <macro.h>

extern u32 _cpu_clock_;

// 20 cycles
u16 umul16(u16 multiplier, u16 multiplicand)
{
    u16 product;

    #define uLB16(x) (*(u8 *)(&x))
    #define uHB16(x) (*(((u8 *)(&x))+1))

    product =  (uLB16(multiplier) * uLB16(multiplicand));
    product += (uLB16(multiplier) * uHB16(multiplicand)) << 8;
    product += (uHB16(multiplier) * uLB16(multiplicand)) << 8;

    return product;
}

// 100 cycles
u32 udiv32(u32 dividend, u32 divisor)
{
    u32 quotient;
    u8  counter;

    quotient = 0;
    if(divisor != 0)
    {
        counter = 1;
        while((divisor & 0x8000000UL) == 0)
        {
            divisor <<= 1;
            counter++;
        }
        do {
            quotient <<= 1;
            if(divisor <= dividend)
            {
                dividend -= divisor;
                quotient |= 1;
            }
            divisor >>= 1;
        } while(--counter != 0);
    }
    return quotient;
}

/**
    31000 Hz < Freq. 8-bit PIC Clock < 64MHz
    7750 < Cycles per second = Clock / 4 < 16.000.000
    8 < Cycles per millisecond < 16.000
    0 < Cycles per microsecond < 16
    
    F MHz = F * 1000000 Cycles/s
          = F * 1000    Cycles/ms
          = F *         Cycles/us
**/

// DECFSZ f,d Decrement f (1 cycle), skip if zero (2 cycles)
// GOTO 2 cycles
// BANKSEL 1 cycle
// MOV 1 cycle

#ifdef _XC8_
    #define KLOOPD1     3
    #define KLOOPD2     5
    #define KLOOPZ      10
    #define KWHILE      16
#else
    #define KLOOPD1     7
    #define KLOOPD2     5
    #define KLOOPZ      12
    #define KWHILE      16
#endif

#define KLOOP       (255*KLOOPD1+KLOOPD2) // XC8 = 770 / SDCC = 1790

/* -------------------------------------------------------------
 * XC8
 * -------------------------------------------------------------
    goto    $+3                             // 2 cycles
loop:
    decfsz  Delayms@d1,f,c                  // 3 cycles, 4 if 0
    goto    loop
    decfsz  Delayms@d2,f,c                  // 5 cycles, 4 if 0
    goto    loop
   -----------------------------------------------------------*/

/* -------------------------------------------------------------
 * SDCC
 * -------------------------------------------------------------
loopd2:                                     // 5 cycles, 6 if 0
    decf    r0x05, w
    movwf   r0x06
    movff   r0x06, r0x05
    movf    r0x06, w
    bz      exit
loopd1:                                     // 7 cycles, 6 if 0
    decf    r0x04, w
    movwf   r0x06
    movff   r0x06, r0x04
    movf    r0x06, w
    bz      loopd2
    bra     loopd1
   -----------------------------------------------------------*/

void Delayms(u16 ms)                            // 4 cycles (incl. return)
{
    u16 d1ms;
    u8  dloop1, dloop2;
    u8  d1, d2, d3;
    
    // < 250 cycles
    // @48MHz : 12000/250 +/- 1/50 ms =  20 us
    // @4MHz  :  1000/250             = 250 us
    d1ms   = udiv32(_cpu_clock_, 4000UL) - KWHILE - KLOOPZ;         // +/-12000
    dloop2 = udiv32(d1ms, KLOOP);                                   // XC8=15  / SDCC=6
    dloop1 = udiv32(((d1ms-umul16(dloop2,KLOOP))+KLOOPD2), KLOOPD1);// XC8=150 / SDCC=210

    while(--ms)                                     // 10 cycles
    {
        d1=dloop1+1;                                // 2 cycles (incf+movwf)
        d2=dloop2+1;                                // 2 cycles (incf+movwf)
        d3=d1;

        // XC8  : (150⋅3+5)+(255⋅3+5)⋅15 = 12005 cycles = 1ms @ 48MHz
        // SDCC : (210⋅7+5)+(255⋅7+5)⋅6  = 12215 cycles = 1ms @ 48MHz
        while(--d2)
        {
            while(--d1);
            #ifdef _XC8_
            while(--d3);
            #endif
        }
    }
}

#endif /* __DELAYMS_C__ */

/*  DO NOT REMOVE ------------------------------------------------------
    {
        d1=D1+1;
        d2=D2+1;
        // (95⋅5+5)+(255⋅5+5)⋅9 = 12000 cycles = 1ms @ 48MHz
        #asm
        loop:
            banksel Delayms@d1
            decfsz  Delayms@d1, f
            goto    $+3
            banksel Delayms@d2
            decfsz  Delayms@d2, f
            goto    loop
        #endasm
    }
    ------------------------------------------------------------------*/
