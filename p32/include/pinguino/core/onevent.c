/*	----------------------------------------------------------------------------
    FILE:				onevent.c
    PROJECT:			pinguino 32
    PURPOSE:			event management
    PROGRAMER:			regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:		14 Jan. 2015
    LAST RELEASE:		14 Jan. 2015
    ----------------------------------------------------------------------------
    CHANGELOG:
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

#ifndef __ONEVENT_C
#define __ONEVENT_C

#ifndef ON_EVENT
#define ON_EVENT
#endif

#include <p32xxxx.h>
#include <typedef.h>
#include <macro.h>
#include <interrupt.h>

#define TXCKPS256 0b111 // 1:256 prescale value
#define TXCKPS64  0b110 // 1:64  prescale value
#define TXCKPS32  0b101 // 1:32  prescale value
#define TXCKPS16  0b100 // 1:16  prescale value
#define TXCKPS8   0b011 // 1:8   prescale value
#define TXCKPS4   0b010 // 1:4   prescale value
#define TXCKPS2   0b001 // 1:2   prescale value
#define TXCKPS1   0b000 // 1:1   prescale value

#define T1CKPS256 0b11  // 1:256 prescale value
#define T1CKPS64  0b10  // 1:64  prescale value
#define T1CKPS8   0b01  // 1:8   prescale value
#define T1CKPS1   0b00  // 1:1   prescale value

typedef void (*callback) (void); // type of: void callback()

static callback intFunction[INT_NUM];
u32 intUsed[INT_NUM];

// OnTimerX only
#if defined(TMR1INT) || defined(TMR2INT) || \
    defined(TMR3INT) || defined(TMR4INT) || \
    defined(TMR5INT)

    volatile u32 intCount[5];
    volatile u32 intCountLimit[5];

    u32 prescalerx[] = {1, 2, 4, 8, 16, 32, 64, 256};
    u32 prescaler1[] = {1, 8, 64, 256};
    
#endif

    // Dans main32.c ?
    //IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

/*  --------------------------------------------------------------------
    OnTimer1
    --------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timer X to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    ------------------------------------------------------------------*/

#ifdef TMR1INT
u32 OnTimer1(callback func, u32 timediv, u32 delay)
{
    u32 tckps=0, osc, period;

    if (intUsed[INT_TIMER1] == INT_NOT_USED)
    {
        intUsed[INT_TIMER1] = INT_USED;
        intFunction[INT_TIMER1] = func;
        intCount[1] = 0;
        intCountLimit[1] = delay;

        // TMR1 Count register increments on every PBCLK clock cycle
        osc = GetPeripheralClock();
    
        // Freq (Hz) = Nb ticks/sec.
        switch(timediv)
        {
            case INT_SEC:      period = osc;           break;
            case INT_MILLISEC: period = osc / 1000;    break;
            case INT_MICROSEC: period = osc / 1000000; break;
        }

        // Timer1 period is 16-bit, only 4 prescaler values
        while ((period > 0xFFFF) & (tckps < 4))
        {
            period /= prescaler1[tckps];
            tckps += 1;
        }

        if (tckps > 3)
        {
            tckps = 3;
            intCountLimit[1] = delay * 8;
        }
        
        T1CON    = tckps << 4;              // set prescaler (bit 5-4)
        TMR1     = 0;                       // clear timer register
        PR1      = period;                  // load period register
        IPC1SET  = 7;                       // select interrupt priority and sub-priority
        IFS0CLR  = 1 << INT_TIMER1_VECTOR;  // clear interrupt flag
        IEC0SET  = 1 << INT_TIMER1_VECTOR;  // enable timer 1 interrupt
        T1CONSET = 0x8000;                  // start timer 1

        return INT_TIMER1;
    }
    
    else
    {
        #ifdef DEBUG
        debug("Error : TIMER1 interrupt is already used !");
        #endif
        return INT_USED;
    }
}

/***********************************************************************
* Timer 1 interrupt (Vector 4)
***********************************************************************/

// Tmr1Interrupt is declared as an interrupt routine
void Timer1Interrupt(void) __attribute__ ((interrupt));

// Put the ISR_wrapper in the good place
void ISR_wrapper_vector_4(void) __attribute__ ((section (".vector_4")));

// ISR_wrapper will call the Timer1Interrupt()
void ISR_wrapper_vector_4(void) { Timer1Interrupt(); }

void Timer1Interrupt()
{
    if (IntGetFlag(INT_TIMER1))
    {
        IntClearFlag(INT_TIMER1);
        if (intCount[1]++ >= intCountLimit[1])
        {
            // reset the counter
            intCount[1] = 0;
            // call user's routine
            intFunction[INT_TIMER1]();
        }
    }
}
#endif /* TMR1INT */

#endif /* __ONEVENT_C */
