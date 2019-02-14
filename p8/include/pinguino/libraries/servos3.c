/*----------------------------------------------------------------------
    FILE        : servo.c
    Version     : 4.2
    Descr.      : Servo control on all Pinguino pins
    Project     : Pinguino
    Author      : Jesús Carmona Esteban
    --------------------------------------------------------------------
    CHANGELOG:
    05 Apr. 2012 - Expansion to versions 4550/PICUNO_EQUO using #defines in user program.
    02 Sep. 2012 - Changes on ServoMinPulse and ServoMaxPulse functions to assemble Arduino ones in order to expand from 500us to 2500us pulses.
    28 Sep. 2013 - Corrections on maths at servowrite funtion. 
    01 Oct. 2013 - Tested and calibrated with oscilloscope for 18F4550, 18F2550 and EQUO_UNO for X.3 IDE.
    20 Oct. 2013 - Fixed interrupt handling for working TMR1 with new x.4 enviroment.
    12 Nov. 2013 - Error on ServospulseUp function corrected. Code cleaned and compacted. Expanded to all 8 bit PICs available. 
    15 Nov. 2013 - Several bugs removed. Improved ServoAttach function.
    16 May. 2014 - Régis Blanchot - Changed Timer1 interrupt (used by DCF77 library) to Timer2 interrupt
    24 May. 2014 - Régis Blanchot - Ported to 32-bit Pinguino
    04 Feb. 2016 - Régis Blanchot - Added all 8-bit (included 16F) support
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
// - This library allows 250 positions for a servo.
//   Those 1-250 values are mapped from 0-180 degrees,
//   which is the input value by user at servo.write function.
// - There is a correspondence table where is stored maximum and minimum
//   values that any servo could reach in microseconds. But the value stored is from 1 to 250.
// - All servos are automatically refreshed by PIC in a parallel way.

// Values mapping between position and microseconds:
//
// TIMESLOT(byte value):
// 1                  62                  125                  192                  250
// |-------------------|-------------------|--------------------|--------------------|
// 500               1000                 1500                2000                 2500
// Time (microseconds)
//
// Defaul values now for SERVOMAX and SERVOMIN should be 64 and 192, 1000usec and 2000usec respectively.
// User can change 0 degrees up to 500 us pulse as absolute minumum, 
// and 180 degrees up to 2500 usec pulse as absolute maximum using the following functions:
// 
// - ServoMinimumPulse(u8 servo, int min_microseconds)
// - ServoMaximumPulse(u8 servo, int max_microseconds)
//
// -------------------------------------------------------------------------------------------------------

#ifndef __SERVO__
#define __SERVO__

//Includes for functions used internally in this lib.
//#include <stdlib.h>
#include <typedef.h>  // u8, u16, u32, ...
#include <digital.h>  // Ports and mask definitions.
#include <macro.h>    // noInterrupts() and interrups()

// Max and Min values that correspond to 2000 usec and 1000 usec. 
#define DefaultSERVOMAX 192
#define DefaultSERVOMIN  64

#define MINUS 500
#define MAXUS 2500
#define MIDUS ((MINUS+MAXUS)/2)

//library internal variables:
volatile u8 phase=0;
volatile u8 needreordering=0;

//-----------------------------------------------------------------------------------------------------------------------------
// Variable definition that depends on PIC type:
//-----------------------------------------------------------------------------------------------------------------------------

#define MAXNBSERVOS   8

//-----------------------------------------------------------------------------------------------------------------------------
// Variables and Matrix definitions for Any PIC
//-----------------------------------------------------------------------------------------------------------------------------
u8 timingindex;
u8 loopvar;
u8 mascaratotal;

u8 timevalue[MAXNBSERVOS];              // This keeps values ordered for all pins.
u8 timings[MAXNBSERVOS]; // This keeps ports and pins activated for a specific timevalue (both matrix share index to make access easy).
u8 activatedservos;       // This keeps masks for every port with the activated pins to be servos.
u8 servovalues[MAXNBSERVOS];            // Entry table for values sets for every pin-servo.
u8 maxminpos[2][MAXNBSERVOS];           // This table keeps minimum(0 degrees) and maximum(180 degrees) values(in ticks) that the servo can reach.

//-----------------------------------------------------------------------------------------------------------------------------
//  Functions for SERVO library
//-----------------------------------------------------------------------------------------------------------------------------

void servo_init()
{
    u8 a;

    // Filling up the servovalues table to 255. 
    for(a=0;a<MAXNBSERVOS;a++)
    {
        servovalues[a]=255;               // Filling up the servovalues table to 255.
        maxminpos[0][a]= DefaultSERVOMIN; // Setting min servo position to 1000 usec.
        maxminpos[1][a]= DefaultSERVOMAX; // Setting max servo position to 2000 usec.
    }
    
    // Filling up the activated servos matrix.
    activatedservos=0x00;  // Setting all pins as deactivated as servo.

    #ifndef __PIC32MX__
    noInterrupts();
    T1CON=0x01;             //timer 1 prescaler 1 source is internal oscillator
    TMR1H=0xFF;             // First value on timer to start up...
    TMR1L=0x00;             // ...now the first interrupt will be generated by timer after 9 ms.
    #ifndef __16F1459
    IPR1bits.TMR1IP = 1;    // INT_HIGH_PRIORITY
    #endif
    PIR1bits.TMR1IF = 0;    // Setting flag to 0
    PIE1bits.TMR1IE = 1;    // INT_ENABLE
    T1CONbits.TMR1ON   = 1; // Starting TMR1
    interrupts();
    #else
    // Timer2 Configuration
    // The Timer2 clock prescale (TCKPS) is 1:64
    // TMR2 count register increments on every Peripheral clock cycle
    // TMR2 increments every 64 * 1/Fpb
    //  500 us =>  500 / (64 / Fpb) = (  500 * Fpb ) / 64 cycles
    // 2500 us => 2500 / (64 / Fpb) = ( 2500 * Fpb ) / 64 cycles

    fpb    = GetPeripheralClock() / 1000 / 1000;
    f500us = (   500 * fpb ) / 64; 
    f20ms  = ( 20000 * fpb ) / 64;
    
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    // bit 6-4 TCKPS<2:0>: Timer Input Clock Prescale Select bits
    // 0 = 1:1   default prescale value
    // 1 = 1:2   prescale value
    // 2 = 1:4   prescale value
    // 3 = 1:8   prescale value
    // 4 = 1:16  prescale value
    // 5 = 1:32  prescale value
    // 6 = 1:64  prescale value
    // 7 = 1:256 prescale value

    T2CON    = 6 << 4;  				// prescaler 1:64, internal peripheral clock
    TMR2     = 0;					// clear timer register
    PR2      = f500us;					// load period register
    IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
    IntClearFlag(INT_TIMER2);
    IntEnable(INT_TIMER2);
    T2CONSET = 0x8000;					// start timer 1
    #endif
}

static void ServosPulseDown()
{
    u8 timingindex, timedivision;

    for(timedivision = 0; timedivision < 251; timedivision++)
    {
        if (timevalue[timingindex] == timedivision)
        {
            LATB = LATB ^ timings[timingindex];
            timingindex++;
            //if (timingindex++ >= MAXNBSERVOS)
            //    timingindex=0;
        }
        
        #ifndef __PIC32MX__
        // NEW: Every round on this "for" loop must last 8 microseconds.
        // So the following asm code is to adjust to that exact time.
        #ifdef __XC8__
            #asm
            MOVLW 7
            MOVWF _loopvar
        bucle:
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            #ifdef __16F1459
            DECFSZ _loopvar
            #else
            DECFSZ _loopvar,B
            #endif
            GOTO bucle
            #endasm
        #else
            __asm
            MOVLW 6
            MOVWF _loopvar
        bucle:
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            DECFSZ _loopvar,1
            GOTO bucle
            __endasm;
        #endif
        #else
        Delayus(8);
        #endif
    }
}

// This function starts up pulses for all activated servos.
static void ServosPulseUp()
{
    LATB = LATB | activatedservos;
}

// This funtion analyses servovalues table and creates and ordered table(timings)
// from smaller to bigger of all the values, asociating to each
// position of the table the servos that matches that timing.
static void SortServoTimings()
{
    volatile u8 s,t,totalservos,numservos;

    // table initialization:
    for(t=0;t<MAXNBSERVOS;t++)
    {
        timevalue[t]=255; 
        timings[t]=0x00;
    }
    
    // mascaratotal table initialization:
    mascaratotal=0x00;

    totalservos=0; // Total servos revised. This helps to keep within "while"
    t=0;           // Index to go through timevalue and timings tables.

    while(totalservos<MAXNBSERVOS)
    {
        numservos=1;

        for(s=0;s<MAXNBSERVOS;s++)
        {
            if (mask[s] & mascaratotal & activatedservos)
                break;

            else if (servovalues[s] < timevalue[t])
            {
                timevalue[t]=servovalues[s];
                timings[t]=mask[s];
                numservos=1;
            }
            else if (servovalues[s] == timevalue[t])
            {
                timings[t] |= mask[s];
                numservos++;
            }
        }
        mascaratotal |= timings[t];
        totalservos += numservos;
        t++;
    }
    needreordering=0;  // This indicates that servo timings are sorted.
}

void ServoAttach(u8 pin)
{
    if(pin>=MAXNBSERVOS) return;

    activatedservos = activatedservos | mask[pin];  // list pin as servo driver.
    TRISB = TRISB & (~mask[pin]);                   // set as output pin
}

void ServoDetach(u8 pin)
{
    if(pin>=MAXNBSERVOS) return;

    activatedservos = activatedservos ^ mask[pin];
}

void ServoWrite(u8 servo,u8 degrees)
{
    u8 range;
    u8 value;

    // Check if number of servo is valid
    if(servo>=MAXNBSERVOS)
        return;

    // limitting degrees:
    if(degrees>180) degrees=180;

    // Converts degrees to timeslots
    range = (maxminpos[1][servo]  - maxminpos[0][servo]);
    value = (degrees*range) / 180 + maxminpos[0][servo];

    // Storage of that new position to servovalues positions table:
    // it should be added the min value for that servo
    servovalues[servo]= value;

    needreordering=1;  // This indicates servo timings must be reordered.
}

u8 ServoRead(u8 servo)
{
    if(servo>=MAXNBSERVOS)        // test if numservo is valid
        return 0;
        
    return servovalues[servo];
}

void ServoMinimumPulse(u8 servo,int min_microseconds)
{
    // Check if number of servo is valid:
    if(servo>=MAXNBSERVOS)
        return;

    // test if microseconds are within range:
    if (min_microseconds < MINUS) min_microseconds = MINUS;
    if (min_microseconds > MIDUS) min_microseconds = MIDUS;

    // The following formula converts min. microseconds to min. timeslot
    maxminpos[0][servo]=(min_microseconds - MINUS)>>3;   // 0 < final_min < 125
}

void ServoMaximumPulse(u8 servo,int max_microseconds)
{
    // Check if number of servo is valid:
    if(servo>=MAXNBSERVOS)
        return;
        
    // test if microseconds are within range:
    if (max_microseconds < MIDUS) max_microseconds = MIDUS;
    if (max_microseconds > MAXUS) max_microseconds = MAXUS;

    // The following formula converts max. microseconds to max. timeslot
    maxminpos[1][servo]=(max_microseconds - MINUS)>>3;   // 125 < final_max < 250
}

//interrupt handler that handles servos
void servo_interrupt(void)
{
    if (PIR1bits.TMR1IF)
    {
        //T1CON=0x00;

        //case before 500 microseconds:

        if (phase)
        {
            ServosPulseUp();
            // Load at TMR1 (65535d - 6000d (-54usec for adjusments ) = 59481d = 0xE859 (after some calibration 0xe959)
            TMR1H= 0xe9;//0xe9;
            TMR1L= 0x59;
            // timer 1 prescaler 1 source is internal oscillator Fosc/4 (CPU clock or Fosc=48Mhz).
            T1CON=1;
            phase = 0;
        }

        //case before 2500 microseconds:

        else
        {
            //The following call takes 2 ms aprox.:
            ServosPulseDown();
            // After fisrt 2,5 ms we need a delay of 17,5 ms to complete a 20 ms cycle.
            // Loading at TMR1 65535d - (4,375 x 12000(=1ms)=) 52500d = 13035d = 0x32EB => 4,375 ms
            // This is 4,375ms x 4 (prescaler) = 17,5 ms
            TMR1H= 0x32;
            TMR1L= 0xeb;
            // timer 1 prescaler 1 source is internal oscillator Fosc/4 (recordemos que Fosc=48Mhz).
            if (needreordering)
                SortServoTimings(); // This takes more than 1 ms, but it's call only if needed.
            T1CON= ( 1 | 2 << 4 ) ; // activate timer1 and prescaler = 1:4
            phase = 1;              //This indicates that after next interrupt it will start the servos cycle.
        }

        // enable interrupt again
        PIR1bits.TMR1IF=0;
    }
    return;
}

#endif
