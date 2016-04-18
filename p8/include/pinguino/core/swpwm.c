/*	--------------------------------------------------------------------
    FILE:			swpwm.c
    PROJECT:		pinguino
    PURPOSE:		software PWM control functions
    PROGRAMER:		Régis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    Changelog :
    18 Apr. 2016 - Régis Blanchot - first release
    --------------------------------------------------------------------
    To do :
    * Change to Timer 1
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

#ifndef __SWPWM__
#define __SWPWM__

#include <compiler.h>       // sfr's
#include <typedef.h>        // u8, u16, u32, ...
#include <macro.h>          // BitSet, ...

// Uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
#define SWPWMRES  127       // PWM resolution

volatile u8 gDutyCycle[8];
volatile u8 gCounter=0;
volatile u8 gPinActivated=0;
volatile t16 _swpwm_period;

/*  --------------------------------------------------------------------
    SWPWM_setFrequency
    --------------------------------------------------------------------
    @descr:     calculate Timer2 prescaler and period to get the frequency
    @param:     frequency in hertz (range 2929Hz .. 12MHz)
    @return:    value of PR2
    ------------------------------------------------------------------*/

u16 SWPWM_setFrequency(u32 freq)
{
    u16 cycles = 10; //_cpu_clock_ / (4 * freq * SWPWMRES);
    
    _swpwm_period.w = 0xFFFF - cycles;

    noInterrupts();             // Disable global interrupts

    #if defined(__16F1459)
    
    #else
    
    // Configure Timer0
    TMR0H = _swpwm_period.h8;
    TMR0L = _swpwm_period.l8;
    //Int.setPriority(INT_TMR0, INT_LOW_PRIORITY);
    //INTCON2bits.TMR0IP = INT_HIGH_PRIORITY;
    INTCON2bits.TMR0IP = 1;
    //Int.clearFlag(INT_TMR0);
    INTCONbits.TMR0IF = 0;
    //Int.enable(INT_TMR0);
    INTCONbits.TMR0IE = 1;
    //Int.start(INT_TMR0);
    //TMR0 = T0_ON | T0_16BIT | T0_PS_OFF;
    T0CON = 0b10001000;

    #endif

    interrupts();               // Enable global interrupts

    return cycles;
}

/*  --------------------------------------------------------------------
    PWM_setDutyCycle
    --------------------------------------------------------------------
    Set dutycycle with 8-bits resolution, allowing 256 PWM steps.
    @param pin:		PortB pin (0 to 7)
    @param duty:	8-bit duty cycle
    ------------------------------------------------------------------*/

void SWPWM_setDutyCycle(u8 pin, u8 duty)
{
    // Configure pins as output
    BitClear(TRISB, pin);

    // Declare pin as activated
    BitSet(gPinActivated, pin);

    // Set duty cycle
    gDutyCycle[pin] = duty;
}

/*  --------------------------------------------------------------------
    PWM_setPercentDutyCycle
    --------------------------------------------------------------------
    Set a percentage duty cycle, allowing max 100 PWM steps.
    Allowed range: 0..100
    The duty cycle will be set to the specified percentage of the maximum
    for the current PWM frequency.
    Note: The number of available PWM steps can be lower than 100 with
    (very) high PWM frequencies.
    ------------------------------------------------------------------*/

void SWPWM_setPercentDutyCycle(u8 pin, u8 percent)
{
    u8 duty;

    if (percent == 0)
        duty = 0;
    else if (percent >= 100)
        duty = SWPWMRES;
    else
        duty = percent * SWPWMRES / 100;

    SWPWM_setDutyCycle(pin, duty);
}

// Compute output level by comparing each register with the gCounter
// The associated pin is made LOW or HIGH according to the result of comparison
// For speed reason we used LAT registers rather than digitalWrite function

void swpwm_interrupt()
{
    volatile u8 i;
    
    //if (Int.isFlagSet(INT_TMR0))
    if (INTCONbits.TMR0IF)
    {
        //Int.clearFlag(INT_TMR0);
        INTCONbits.TMR0IF = 0;
        //
        TMR0H = _swpwm_period.h8;
        TMR0L = _swpwm_period.l8;
        //
        gCounter++;
        gCounter &= SWPWMRES;
        
        /*
        for (i=0; i<8; i++)
        {
            if (BitRead(gPinActivated, i))
            {
                if ( gCounter < gDutyCycle[i] )
                    BitSet(LATB, i);
                else
                    BitClear(LATB, i);
            }
        }
        */
        if (gPinActivated & Bit(0))
        {
            if ( gCounter < gDutyCycle[0] )
                LATBbits.LATB0 = 1;
            else
                LATBbits.LATB0 = 0;
        }
        if (gPinActivated & Bit(1))
        {
            if ( gCounter < gDutyCycle[1] )
                LATBbits.LATB1 = 1;
            else
                LATBbits.LATB1 = 0;
        }
        if (gPinActivated & Bit(2))
        {
            if ( gCounter < gDutyCycle[2] )
                LATBbits.LATB2 = 1;
            else
                LATBbits.LATB2 = 0;
        }
        if (gPinActivated & Bit(3))
        {
            if ( gCounter < gDutyCycle[3] )
                LATBbits.LATB3 = 1;
            else
                LATBbits.LATB3 = 0;
        }
        if (gPinActivated & Bit(4))
        {
            if ( gCounter < gDutyCycle[4] )
                LATBbits.LATB4 = 1;
            else
                LATBbits.LATB4 = 0;
        }
        if (gPinActivated & Bit(5))
        {
            if ( gCounter < gDutyCycle[5] )
                LATBbits.LATB5 = 1;
            else
                LATBbits.LATB5 = 0;
        }
        if (gPinActivated & Bit(6))
        {
            if ( gCounter < gDutyCycle[6] )
                LATBbits.LATB6 = 1;
            else
                LATBbits.LATB6 = 0;
        }
        if (gPinActivated & Bit(7))
        {
            if ( gCounter < gDutyCycle[7] )
                LATBbits.LATB7 = 1;
            else
                LATBbits.LATB7 = 0;
        }
        /*
        if (gPinActivated & Bit(0))
            LATBbits.LATB0 = ( gCounter < gDutyCycle[0] ) ? 1:0;
        if (gPinActivated & Bit(1))
            LATBbits.LATB1 = ( gCounter < gDutyCycle[1] ) ? 1:0;
        if (gPinActivated & Bit(2))
            LATBbits.LATB2 = ( gCounter < gDutyCycle[2] ) ? 1:0;
        if (gPinActivated & Bit(3))
            LATBbits.LATB3 = ( gCounter < gDutyCycle[3] ) ? 1:0;
        if (gPinActivated & Bit(4))
            LATBbits.LATB4 = ( gCounter < gDutyCycle[4] ) ? 1:0;
        if (gPinActivated & Bit(5))
            LATBbits.LATB5 = ( gCounter < gDutyCycle[5] ) ? 1:0;
        if (gPinActivated & Bit(6))
            LATBbits.LATB6 = ( gCounter < gDutyCycle[6] ) ? 1:0;
        if (gPinActivated & Bit(7))
            LATBbits.LATB7 = ( gCounter < gDutyCycle[7] ) ? 1:0;
        */
    }
}

#endif /* __SWPWM__ */
