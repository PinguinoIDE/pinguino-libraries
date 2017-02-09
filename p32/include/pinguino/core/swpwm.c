/*	--------------------------------------------------------------------
    FILE:			swpwm.c
    PROJECT:		pinguino
    PURPOSE:		software PWM control functions
    PROGRAMER:		Régis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    Changelog :
    21 Sep. 2016 - Régis Blanchot - first PIC32 release
    --------------------------------------------------------------------
    To do :
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

#include <p32xxxx.h>                // sfr's
#include <typedef.h>                // u8, u16, u32, ...
#include <macro.h>                  // BitSet(), ...
#include <interrupt.h>
#include <interrupt.c>              // IntConfigureSystem(), ...
#include <digitalw.c>               // high(), low(), ...
#include <system.c>                 // GetPeripheralClock()

// Uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
#define NBSWPWMPIN              8
#define SWPWMRES                255 // PWM resolution

volatile u8 gDutyCycle[NBSWPWMPIN];
volatile u8 gCounter=0;
volatile u8 gPinNum=0;
volatile u8 gPinActivated=0;
u32 gPrescaler[] = {1, 8, 64, 256};

/*  --------------------------------------------------------------------
    SWPWM_setFrequency
    --------------------------------------------------------------------
    @descr:     calculate Timer1 prescaler and period to get the frequency
    @param:     frequency in hertz
    @return:    value of PR1
    ------------------------------------------------------------------*/

u16 SWPWM_setFrequency(u32 freq)
{
    u32 tckps = 2;                  // prescaler select bits
    u32 Fpb=GetPeripheralClock();   // TMR1 increments on every PBCLK clock cycle
    u32 period = (freq * 0x10000 * gPrescaler[tckps] ) / Fpb;
    
    noInterrupts();                 // Disable global interrupts
    
    // Configure interrupt
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    IntSetVectorPriority(INT_TIMER1_VECTOR, 7, 3);
    IntClearFlag(INT_TIMER1);
    IntEnable(INT_TIMER1);

    // Configure Timer1
    T1CON = tckps << 4;             // set prescaler (bit 5-4)
    TMR1 = 0;                       // clear timer register
    PR1 = period/NBSWPWMPIN;        // load period register (divided by nb of pins)
    T1CONSET = 0x8000;              // start timer 1

    interrupts();                   // Enable global interrupts

    return PR1;
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
    output(pin);
    
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

//void swpwm_interrupt()
void Timer1Interrupt()
{
    u8 pin;
    
    if (IntGetFlag(INT_TIMER1))
    {
        IntClearFlag(INT_TIMER1);
        gCounter++;
        gCounter &= SWPWMRES;
        //
        for (pin=0; pin<NBSWPWMPIN; pin++)
            if (gPinActivated & (1<<pin))
                ( gCounter < gDutyCycle[pin] ) ? high(pin) : low(pin);
    }
}

#endif /* __SWPWM__ */
