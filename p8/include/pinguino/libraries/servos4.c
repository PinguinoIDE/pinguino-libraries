/*----------------------------------------------------------------------
    FILE        : servo.c
    Version     : 4.3
    Descr.      : Servo control on all Pinguino pins
    Project     : Pinguino
    Author      : Jesús Carmona Esteban
                  Régis Blanchot
    --------------------------------------------------------------------
    CHANGELOG:
    05 Apr. 2012 - Expansion to versions 4550/PICUNO_EQUO using #defines in user program.
    02 Sep. 2012 - Changes on ServoMinPulse and ServoMaxPulse functions to assemble Arduino ones in order to expand from 500us to 2500us PulseWidths.
    28 Sep. 2013 - Corrections on maths at servowrite funtion. 
    01 Oct. 2013 - Tested and calibrated with oscilloscope for 18F4550, 18F2550 and EQUO_UNO for X.3 IDE.
    20 Oct. 2013 - Fixed interrupt handling for working TMR1 with new x.4 enviroment.
    12 Nov. 2013 - Error on ServosPulseWidthUp function corrected. Code cleaned and compacted. Expanded to all 8 bit PICs available. 
    15 Nov. 2013 - Several bugs removed. Improved ServoAttach function.
    04 Feb. 2016 - Régis Blanchot - Added all 8-bit (included 16F) support
    10 Oct. 2016 - Régis Blanchot - Changed PORTx to LATx
    12 Oct. 2016 - Régis Blanchot - Added ServoAttached() function
    13 Oct. 2016 - Régis Blanchot - ServoRead() now returns angle in degrees, not a PulseWidth width
    20 Apr. 2017 - Régis Blanchot - Fixed the lib. to support XC8 and SDCC
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
----------------------------------------------------------------------*/


// NOTES:
// - Fosc 48Mhz => 12 MIPS (Fosc/4).
//   Lesser clock frequencies are not compatible with this library.
// - This library allows 250 positions for a servo.
//   Those 1-250 values are mapped from 0-180 degrees,
//   which is the input value by user at servo.write function.
// - There is a correspondence table where is stored MaxPulseWidthimum and MinPulseWidthimum
//   values that any servo could reach in microseconds.
//   But the value stored is from 1 to 250.
// - All servos are automatically refreshed by PIC in a parallel way.
//
// Values mapping between position and microseconds:
//
// 500 (microseconds)  1000                1500                2000                 2500
//   |------------------- |-------------------|-------------------|--------------------|
//  50 (timeslot)       100                 150                 200                  250
//   |--------------------|-------------------|-------------------|--------------------|
//     (degrees)          0                  90                 180
//
// ---------------------------------------------------------------------

#ifndef __SERVO__
#define __SERVO__

//Includes for functions used internally in this lib.
#include <compiler.h>
#include <typedef.h>                // u8, u16, u32, ...
#include <macro.h>                  // noInterrupts() and interrups()
#include <interrupt.h>              // Timers definitions
//#include <digital.h>                // Ports and mask definitions.
//#include <digitalw.c>               // digitalwrite
//#include <digitalp.c>               // pinmode
#include <delayus.c>                // Delayus
//#include <delayms.c>                // Delayms

//----------------------------------------------------------------------
// Absolute Servos Spec.
//----------------------------------------------------------------------

#define ABSOLUTE_MIN_US         500     // 0   deg <=> 1ms
#define ABSOLUTE_MAX_US         2500    // 180 deg <=> 2ms
#define ABSOLUTE_MID_US         ((ABSOLUTE_MIN_US + ABSOLUTE_MAX_US) / 2)

#define SERVO_PERIOD_US         20000   // 20ms +/- 2ms or 50Hz
#define SERVO_RESOLUTION        250     // How many steps from 0 to 180 deg.
#define SERVO_RESOLUTION_STEP   (ABSOLUTE_MAX_US / SERVO_RESOLUTION)
//----------------------------------------------------------------------
// Timer Configuration
// The TMR Count register increments on every FOSC/4 cycle
// FOSC/4 = 48/4 = 12 MHz => TMR inc. every PRESCALER/12 us
// NB Cycles = TIME(us) / PRESCALER * 12
//----------------------------------------------------------------------

#define TMR500US_PRESCALER      T1_PS_1_1
#define TMR500US_PRELOAD        (0xFFFF - ABSOLUTE_MIN_US * 12)
#define TMR500US_PRELOADH       high8(TMR500US_PRELOAD)
#define TMR500US_PRELOADL       low8(TMR500US_PRELOAD)

#define TMR2000US_PRESCALER     T1_PS_1_1
#define TMR2000US_PRELOAD       (0xFFFF - (ABSOLUTE_MAX_US - ABSOLUTE_MIN_US) * 12)
#define TMR2000US_PRELOADH      high8(TMR2000US_PRELOAD)
#define TMR2000US_PRELOADL      low8(TMR2000US_PRELOAD)
#define TMR2000US_DELAY         SERVO_RESOLUTION_STEP

#define TMR17500US_PRESCALER    T1_PS_1_8
#define TMR17500US_PRELOAD      (0xFFFF - (SERVO_PERIOD_US - ABSOLUTE_MAX_US) / 8 * 12)
#define TMR17500US_PRELOADH     high8(TMR17500US_PRELOAD)
#define TMR17500US_PRELOADL     low8(TMR17500US_PRELOAD)

//----------------------------------------------------------------------
// Timeslots
//----------------------------------------------------------------------

#define ABSOLUTE_MIN_TS         (ABSOLUTE_MIN_US / SERVO_RESOLUTION_STEP)
#define ABSOLUTE_MAX_TS         (ABSOLUTE_MAX_US / SERVO_RESOLUTION_STEP)
        
//----------------------------------------------------------------------
// Variable definition that depends on PIC type:
//----------------------------------------------------------------------

#define MAXNBSERVOS             8

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------

volatile u16 gTimeSlot = ABSOLUTE_MIN_TS;
volatile u8  gActivatedServos = 0;
volatile u8  gPhase = 0;
volatile u8  gSorted;

//----------------------------------------------------------------------
// Servo type
//----------------------------------------------------------------------

typedef struct
{
    u8 Activated;       // attached or not
    u8 PulseWidth;      // target  timeslot
    u8 MinPulseWidth;   // MinPulseWidthimal timeslot
    u8 MaxPulseWidth;   // MaxPulseWidthimal timeslot
} SERVO_t;

volatile SERVO_t Servo[MAXNBSERVOS];
volatile u8 Sorted[MAXNBSERVOS];

//----------------------------------------------------------------------
//  Initialisation
//----------------------------------------------------------------------

void servo_init()
{
    u8 n;

    // Filling up the servovalues table to 255. 
    for (n=0; n < MAXNBSERVOS; n++)
    {
        Servo[n].Activated = 0;   // detached
        Servo[n].MinPulseWidth = ABSOLUTE_MIN_TS;
        Servo[n].MaxPulseWidth = ABSOLUTE_MAX_TS;
        Sorted[n] = n;
    }
    
    // Configure the Timer
    noInterrupts();
      
    TMR1H = TMR500US_PRELOADH;
    TMR1L = TMR500US_PRELOADL;
    T1CON = T1_ON | TMR500US_PRESCALER;

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
    T1GCON = 0;
    #endif

    #ifndef __16F1459
    IPR1bits.TMR1IP = 1;    // INT_HIGH_PRIORITY
    #endif

    PIR1bits.TMR1IF = 0;    // Setting flag to 0
    PIE1bits.TMR1IE = 1;    // INT_ENABLE
    
    interrupts();
}

//----------------------------------------------------------------------
// Attach servo to a pin
// Set this pin as an output
//----------------------------------------------------------------------

void ServoAttach(u8 n)
{
    if (n < MAXNBSERVOS)
    {
        /*
        pinmode(n, OUTPUT);
        digitalwrite(n, 0);
        */
        BitClear(LATB,  n);     // Set pin low
        BitClear(TRISB, n);     // Set pin output
        Servo[n].Activated = 1; // Attached
        BitSet(gActivatedServos,  n); // Set pin activated
    }
}

//----------------------------------------------------------------------
// Detach servo from its pin
//----------------------------------------------------------------------

void ServoDetach(u8 n)
{
    if (n < MAXNBSERVOS)
        Servo[n].Activated = 0;   // Detached
        BitClear(gActivatedServos,  n); // Set pin activated
}

//----------------------------------------------------------------------
// Return 1 if the servo is currently attached.
//----------------------------------------------------------------------

u8 ServoAttached(u8 n)
{
    if (n < MAXNBSERVOS)
        return Servo[n].Activated;
        //return BitTest(gActivatedServos, n);
    return 0;
}

//----------------------------------------------------------------------
// Set the duration of the 0 degree PulseWidth in microseconds.
// Default MinPulseWidth value is ABSOLUTE_MIN_US microseconds.
//----------------------------------------------------------------------

void ServoMinimumPulse(u8 n, u16 ms)
{
    // Check if number of servo is valid:
    if (n < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (ms < ABSOLUTE_MIN_US) ms = ABSOLUTE_MIN_US;
        if (ms > ABSOLUTE_MID_US) ms = ABSOLUTE_MID_US;

        // The following formula converts MinPulseWidth. microseconds to MinPulseWidth. timeslot
        Servo[n].MinPulseWidth = ms / SERVO_RESOLUTION_STEP;
    }
}

//----------------------------------------------------------------------
// Set the duration of the 180 degree PulseWidth in microseconds.
// Default MaxPulseWidth value is ABSOLUTE_MAX_US microseconds.
//----------------------------------------------------------------------

void ServoMaximumPulse(u8 n, u16 ms)
{
    // Check if number of servo is valid:
    if (n < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (ms < ABSOLUTE_MID_US) ms = ABSOLUTE_MID_US;
        if (ms > ABSOLUTE_MAX_US) ms = ABSOLUTE_MAX_US;

        // The following formula converts MaxPulseWidth. microseconds to MaxPulseWidth. timeslot
        Servo[n].MaxPulseWidth = ms / SERVO_RESOLUTION_STEP;
    }
}

//----------------------------------------------------------------------
// Convert degree to PulseWidth width
//----------------------------------------------------------------------

void ServoWrite(u8 n, u8 degrees)
{
    u8 range;

    // Check if number of servo is valid
    if (n < MAXNBSERVOS)
    {
        // Converts degrees to timeslot
        if (degrees > 180)
        {
            Servo[n].PulseWidth = Servo[n].MaxPulseWidth;
        }
        else
        {
            range = Servo[n].MaxPulseWidth  - Servo[n].MinPulseWidth;
            Servo[n].PulseWidth = (degrees*range) / 180 + Servo[n].MinPulseWidth;
        }
    }
    Delayus(22000);
    // Servo timings must be reordered.
    gSorted = 1;
}

//----------------------------------------------------------------------
// Return servo position in degrees
//----------------------------------------------------------------------

u8 ServoRead(u8 n)
{
    u8 range;

    // Check if number of servo is valid:
    if (n < MAXNBSERVOS)
    {
        range = Servo[n].MaxPulseWidth - Servo[n].MinPulseWidth;
        return (180 * (Servo[n].PulseWidth - Servo[n].MinPulseWidth) / range);
    }
    return 255;
}

//----------------------------------------------------------------------
// Sort Servo
//----------------------------------------------------------------------

#if 0
void ServoSort()
{
    volatile u8 i, j;

    for (i = 0; i < MAXNBSERVOS; i++)
        for (j = i + 1; j < MAXNBSERVOS; j++)
            if (Servo[Sorted[i]].PulseWidth > Servo[Sorted[j]].PulseWidth)
                swap(Sorted[i], Sorted[j]);
    // Servo timings are sorted.
    gSorted = 0;
}
#endif

//----------------------------------------------------------------------
// Interrupt handler
//----------------------------------------------------------------------

void servo_interrupt(void)
{
    u8 n, t, num;
    
    if (PIR1bits.TMR1IF)
    {
        if (gPhase)
        {
            // Set signal high
            LATB = LATB | gActivatedServos;
            // 0.5 ms
            TMR1H = TMR500US_PRELOADH;
            TMR1L = TMR500US_PRELOADL;
            T1CON = T1_ON | TMR500US_PRESCALER;
            gPhase = 0;
        }

        else
        {
            // 2 ms
            // Set signal low if PulseWidth has been reached
            // Each loop must be 2 ms / (ABSOLUTE_MAX_TS - ABSOLUTE_MIN_TS)
            for (t = ABSOLUTE_MIN_TS; t < ABSOLUTE_MAX_TS; t++)
            {
                for (n = 0; n < MAXNBSERVOS; n++)
                {
                    //num = Sorted[n];
                    //num = n;
                    //if (Servo[num].Activated)
                        if (Servo[n].PulseWidth == t)
                            BitClear(LATB, n);
                }
                //Delayus(SERVO_RESOLUTION_STEP);
            }

            //if (gSorted)
            //    ServoSort();
            
            // 17.5 ms
            TMR1H = TMR17500US_PRELOADH;
            TMR1L = TMR17500US_PRELOADL;
            T1CON = T1_ON | TMR17500US_PRESCALER;
            gPhase = 1;
        }
        
        // Enable interrupt again
        PIR1bits.TMR1IF = 0;
    }
}

#endif // __SERVO__
