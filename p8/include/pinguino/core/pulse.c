/*
 * pulseIn() function
 * 
 * 2011-02-17 - Marcus Fazzi   - First release
 * 2017-12-05 - Regis Blanchot - Complete re-write for more accuracy
 * 
 */
 
#ifndef __PULSE__
#define __PULSE__

//Includes for functions used internally in this lib.
#include <typedef.h>
#include <macro.h>
#if defined(__PIC32MX__)
#include <digital.c>        // digitalread
#include <interrupt.c>      // Timers functions
#else
#include <compiler.h>
#include <interrupt.h>      // Timers definitions
#include <digitalr.c>       // digitalread
#endif

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------

#if !defined(__PIC23MX__)
extern unsigned long _cpu_clock_;
#endif

u8 gFpb = 0;
 
#define CAPTURE 0b00000100

#if defined(__PIC32MX__)
    #define PRESCALER   5       // 1:32  prescale value
#else
    #define PRESCALER   3       // 1:8   prescale value
#endif

#if defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)
    extern volatile u16     CCPR4  @ 0xF13;
    extern volatile u16     CCPR5  @ 0xF10;
    extern volatile u16     CCPR6  @ 0xF0D;
    extern volatile u16     CCPR7  @ 0xF0A;
    extern volatile u16     CCPR8  @ 0xF07;
    extern volatile u16     CCPR9  @ 0xF04;
    extern volatile u16     CCPR10 @ 0xF01;
#endif

//----------------------------------------------------------------------
//  Calculation
//----------------------------------------------------------------------
// Signal period (us) = (Period + 1) * Timer prescaler / Fpb
// (Period + 1) * prescaler / Fpb = us/1000000
// (Period + 1) * prescaler = us * Fpb / 1000000
// Period = ((us * Fpb / 1000000) / prescaler) - 1

#define UsToCycle(us)   ((us * gFpb) >> PRESCALER)
#define CycleToUs(cy)   ((cy << PRESCALER) / gFpb)

void pulse_init()
{
    #if defined(__PIC32MX__)

        // Get the peripheral clock frequency
        gFpb = GetPeripheralClock() / 1000000;
        TMR2  = 0;              // clear timer register
        T2CON = PRESCALER << 4; // prescaler, internal peripheral clock
        PR2   = UsToCycle(1);
        T2CONSET = Bit(15);     // start timer

    #else // PIC16F and PIC18F

        // Get the peripheral clock frequency
        gFpb = _cpu_clock_ / 4000000;
        TMR1 = 0;
        T1CON = T1_SOURCE_FOSCDIV4 | (PRESCALER << 4) | T1_16BIT | T1_ON;

    #endif
}

/**
* Measures the length (in microseconds) of a pulse
* pin : a digital pin number
* state : HIGH or LOW, the type of pulse to measure
* timeout : 
* Works on pulses from 2-3 microseconds to 3 minutes in length.
* It must be called at least a few dozen microseconds
* before the start of the pulse. 
*/
 
u16 pulseIn(u8 pin, u8 state, u16 timeout)
{
    u16 cy1, cy2, tout;
    u16 toutmax = timeout / gFpb;
    u16 width = 0;                  // keep initialization out of time critical area
    u16 numloops = 0;               // convert the timeout from microseconds to a number of times through
    u16 maxloops = timeout / 10;    // We have a microsecond by 10 loops (mean).

    #if defined(__PIC32MX__)



    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)

    if (pin == CCP4)
    {
        TRISBbits.TRISB4 = 1;       // RB4 = CCP4 pin = INPUT

        // Configure and wait 1st capture
        tout = 0;
        CCP4CON = CAPTURE | state;
        while(!PIR4bits.CCP4IF & tout++ < toutmax);
        cy1 = CCPR4;
        PIR4bits.CCP4IF = 0;        // clear capture flag bit

        // Configure and wait 2nd capture
        tout = 0;
        CCP4CON = CAPTURE | (!state);
        while(!PIR4bits.CCP4IF & tout++ < toutmax);
        cy2 = CCPR4;
        PIR4bits.CCP4IF = 0;        // clear capture flag bit
    }
    
    #else
    
    if (pin == CCP1)
    {
        TRISCbits.TRISC2 = 1;   // CCP1 pin = RC2 = Input

        // Configure and wait 1st capture
        tout = 0;
        CCP1CON = CAPTURE | state;
        while(!PIR1bits.CCP1IF & tout++ < toutmax);
        cy1 = CCPR1;
        PIR1bits.CCP1IF = 0;        // clear capture flag bit

        // Configure and wait 2nd capture
        tout = 0;
        CCP1CON = CAPTURE | (!state);
        while(!PIR1bits.CCP1IF & tout++ < toutmax);
        cy2 = CCPR1;
        PIR1bits.CCP1IF = 0;        // clear capture flag bit
    }
    else if (pin == CCP2)
    {
        TRISCbits.TRISC1 = 1;   // CCP2 pin = RC1 = Input

        // Configure and wait 1st capture
        tout = 0;
        CCP2CON = CAPTURE | state;
        while(!PIR2bits.CCP2IF & tout++ < toutmax);
        cy1 = CCPR2;
        PIR2bits.CCP2IF = 0;        // clear capture flag bit

        // Configure and wait 2nd capture
        tout = 0;
        CCP2CON = CAPTURE | (!state);
        while(!PIR2bits.CCP2IF & tout++ < toutmax);
        cy2 = CCPR2;
        PIR2bits.CCP2IF = 0;        // clear capture flag bit
    }
    else
    {
        // wait for any previous pulse to end
        while (digitalread(pin) == state)
            if (numloops++ == maxloops)
                return 0;
        
        // wait for the pulse to start
        while (digitalread(pin) != state)
            if (numloops++ == maxloops)
                return 0;
        
        // wait for the pulse to stop
        while (digitalread(pin) == state)
            width++;

        // There will be some error introduced by the interrupt handlers.
        // At last loop, each interaction have 12us + 60us from digitalRead()
        // instructions
        return width * 12 + 60; 
    }

    #endif
    
    return CycleToUs(cy2-cy1);
}
 
#endif
