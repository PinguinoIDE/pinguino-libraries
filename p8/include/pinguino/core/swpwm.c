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

#include <compiler.h>               // sfr's
#include <typedef.h>                // u8, u16, u32, ...
#include <macro.h>                  // BitSet, ...

// Uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
#define SWPWMRES                255 // PWM resolution
#define SWPWMNBPIN              8   // Number of SW PWM pin

volatile u8 gDutyCycle[SWPWMNBPIN];
volatile u8 gCounter=0;
volatile u8 gPinNum=0;
volatile u8 gPinActivated=0;
volatile t16 _swpwm_period;

/*  --------------------------------------------------------------------
    SWPWM_setFrequency
    --------------------------------------------------------------------
    @descr:     calculate Timer prescaler and period to get the frequency
    @param:     frequency in hertz (range 2929Hz .. 12MHz)
    @return:    period
    ------------------------------------------------------------------*/

u16 SWPWM_setFrequency(u32 freq)
{
    _swpwm_period.w = 0xFFFF - ((_cpu_clock_ / 4) / (freq * SWPWMRES));

    noInterrupts();                 // Disable global interrupts

    #if defined(__16F1459)
    
    #error "Not yet implemented."
    #error "Please, contact rblanchot@pinguino.cc"
    
    #else
    
    // Configure Timer0
    TMR0H = _swpwm_period.h8;
    TMR0L = _swpwm_period.l8;
    INTCON2bits.TMR0IP = 1;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    T0CON = 0b10001000;         // TMR0 = T0_ON | T0_16BIT | T0_PS_OFF;

    #endif

    interrupts();                   // Enable global interrupts

    return _swpwm_period.w;
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
    BitClear(TRISB, pin);           // Configure pins as output
    BitSet(gPinActivated, pin);     // Declare pin as activated
    gDutyCycle[pin] = duty;         // Set duty cycle
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

/*  --------------------------------------------------------------------
    Interrupt routine
    --------------------------------------------------------------------
    Compute output level by comparing each register with the gCounter
    The associated pin is made LOW or HIGH according to the result of comparison
    For speed reason we used LAT registers rather than digitalWrite function
    ------------------------------------------------------------------*/

void swpwm_interrupt()
{
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
        //
        if (gPinActivated & 1)
            LATBbits.LATB0 = (gCounter < gDutyCycle[0]);
        if (gPinActivated & 2)
            LATBbits.LATB1 = (gCounter < gDutyCycle[1]);
        if (gPinActivated & 4)
            LATBbits.LATB2 = (gCounter < gDutyCycle[2]);
        if (gPinActivated & 8)
            LATBbits.LATB3 = (gCounter < gDutyCycle[3]);
        if (gPinActivated & 16)
            LATBbits.LATB4 = (gCounter < gDutyCycle[4]);
        if (gPinActivated & 32)
            LATBbits.LATB5 = (gCounter < gDutyCycle[5]);
        if (gPinActivated & 64)
            LATBbits.LATB6 = (gCounter < gDutyCycle[6]);
        if (gPinActivated & 128)
            LATBbits.LATB7 = (gCounter < gDutyCycle[7]);
    }
}

#endif /* __SWPWM__ */
