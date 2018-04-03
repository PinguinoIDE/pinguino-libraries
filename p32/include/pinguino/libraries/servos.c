/*----------------------------------------------------------------------
    FILE        : servo.c
    Version     : 5.0
    Descr.      : Servo control on all Pinguino pins
    Project     : Pinguino
    Author      : Régis Blanchot
    --------------------------------------------------------------------
    CHANGELOG:
    05 Apr. 2012 - Expansion to versions 4550/PICUNO_EQUO using #defines in user program.
    02 Sep. 2012 - Changes on ServoMinPulse and ServoMaxPulse functions to assemble Arduino ones in order to expand from 500us to 2500us PulseWidths.
    28 Sep. 2013 - Corrections on maths at servowrite funtion. 
    01 Oct. 2013 - Tested and calibrated with oscilloscope for 18F4550, 18F2550 and EQUO_UNO for X.3 IDE.
    20 Oct. 2013 - Fixed interrupt handling for working TMR1 with new x.4 enviroment.
    12 Nov. 2013 - Error on ServosPulseWidthUp function corrected. Code cleaned and compacted. Expanded to all 8 bit PICs available. 
    15 Nov. 2013 - Several bugs removed. Improved ServoAttach function.
    04 Feb. 2016 - v4.0 - Régis Blanchot - Added all 8-bit (included 16F) support
    10 Oct. 2016 - v4.1 - Régis Blanchot - Changed PORTx to LATx
    12 Oct. 2016 - v4.2 - Régis Blanchot - Added ServoAttached() function
    13 Oct. 2016 - v4.3 - Régis Blanchot - ServoRead() now returns angle in degrees, not a PulseWidth width
    20 Apr. 2017 - v4.4 - Régis Blanchot - Fixed the lib. to support XC8 and SDCC
    16 Oct. 2017 - v5.0 - Régis Blanchot - Use of Output Compare module
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

#ifndef __SERVO__
#define __SERVO__

//Includes for functions used internally in this lib.
#if defined(__PIC32MX__)
    #include <delay.c>          // Delayus and Delayms
    #include <interrupt.c>      // Timers functions
    #include <system.c>         // GetPeripheralClock
#else
    #include <compiler.h>
    #include <delayus.c>        // Delayus
    #include <interrupt.h>      // Timers definitions
    #include <const.h>          // ATOMIC
#endif

#include <typedef.h>            // u8, u16, u32, ...
#include <macro.h>              // noInterrupts() and interrups()

#define SERVOPERIOD             20000 // 20 ms = 20 000 us
#if defined(__PIC32MX__)
    #define MAXNBSERVOS         5
    #define TIMERPERIOD         (SERVOPERIOD / MAXNBSERVOS)
#else
    #define TOGONMATCH          0x02  // CCPx pin toggle on compare match
    #define SETONMATCH          0x08  // CCPx pin set on compare match
    #define CLRONMATCH          0x09  // CCPx pin cleared on compare match
    #define CCPMATCH            SETONMATCH
    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #define MAXNBSERVOS     7 
    #else
        #define MAXNBSERVOS     2
    #endif
    #define TIMERPERIOD         SERVOPERIOD
#endif

//----------------------------------------------------------------------
// Absolute Servos Spec.
//
// 500 (us)       1000            1500            2000             2500
//   |---------------|---------------|---------------|----------------|
//   0 (degrees)    45              90             135              180
//
//----------------------------------------------------------------------

#define ABSOLUTE_MIN_DUTY       500     // 0   deg <=> 0.5 ms ( 500 us)
#define ABSOLUTE_MAX_DUTY       2500    // 180 deg <=> 2.5 ms (2500 us)
#define ABSOLUTE_MID_DUTY       ((ABSOLUTE_MIN_DUTY + ABSOLUTE_MAX_DUTY) / 2)

//----------------------------------------------------------------------
// Servo type
//----------------------------------------------------------------------

typedef struct
{
    u8  Attached;               // attached or not
    u16 PulseWidth;             // current pulse width
    u16 MinPulseWidth;          // minimal pulse width
    u16 MaxPulseWidth;          // maximal pulse width
    u16 Range;                  // MaxPulseWidth - MinPulseWidth
    u16 Phase1;                 // nb of cycles when signal is high
    #if !defined(__PIC23MX__)
    u16 Phase0;                 // nb of cycles when signal is low
    #endif
} SERVO_t;

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------

#if !defined(__PIC23MX__)
extern unsigned long _cpu_clock_;
#endif

volatile u8 gPrescaler = 0;
volatile u8 gFpb = 0;

#if defined(__PIC32MX__)
volatile u8 gActiveServo = 0;
#else
volatile u16 gTimerPeriod;
#endif

volatile SERVO_t Servo[MAXNBSERVOS];

//----------------------------------------------------------------------
//  Duty cycle calculation
//----------------------------------------------------------------------
// TMR2 Period (us) = (PR2 + 1) * TMR2 prescaler / Fpb
// (PR2 + 1) * prescaler / Fpb = us/1000000
// (PR2 + 1) * prescaler = us * Fpb / 1000000
// PR2 = ((us * Fpb / 1000000) / prescaler) - 1
//----------------------------------------------------------------------
// The TMR1 register pair (TMR1H:TMR1L) increments from 0000h to FFFFh
// and rolls over to 0000h. The Timer1 interrupt is generated on overflow.
//----------------------------------------------------------------------

#if 1

#define ServoPulseToCycle(us)  ((us * gFpb >> gPrescaler) - 1)

#else

u32 ServoPulseToCycle(u16 us)
{
    u32 tmp = us * gFpb;
    tmp >>= gPrescaler;
    return (tmp - 1);
}

#endif

//----------------------------------------------------------------------
//  Initialization
//----------------------------------------------------------------------

void servo_init()
{
    u8  n;

    #if defined(__PIC32MX__)
    gFpb = GetPeripheralClock() / 1000000;
    #else
    gFpb = (_cpu_clock_ / 4) / 1000000;
    #endif

    // Filling up the servo values table 
    for (n = 0; n < MAXNBSERVOS; n++)
    {
        Servo[n].Attached = 0;   // detached
        Servo[n].MinPulseWidth = ABSOLUTE_MIN_DUTY;
        Servo[n].MaxPulseWidth = ABSOLUTE_MAX_DUTY;
        Servo[n].PulseWidth    = ABSOLUTE_MID_DUTY;
        Servo[n].Range         = ABSOLUTE_MAX_DUTY - ABSOLUTE_MIN_DUTY;
        Servo[n].Phase1        = 0;
        #if !defined(__PIC32MX__)
        Servo[n].Phase0        = 0;
        #endif
    }
    
    // Configure the Timer

    #if defined(__PIC32MX__)

        noInterrupts();
      
        // Reset Output Compare module
        //OC1R = 0; OC2R = 0; OC3R = 0; OC4R = 0; OC5R = 0;

        IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
        //bit 6-4 TCKPS<2:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   prescale value
        //gPrescaler = 2;         // 1:4   prescale value
        //gPrescaler = 3;         // 1:8   prescale value
        //gPrescaler = 4;         // 1:16  prescale value

        // Max duty cycle value is with :
        // - Period = 20000 us
        // - Fpb = 80 MHz
        // 20000 * 80 / 32 = 0xC350 = 16-bit number
        gPrescaler = 5;         // 1:32  prescale value

        //gPrescaler = 6;         // 1:64  prescale value
        //gPrescaler = 7;         // 1:256 prescale value

        T2CON = gPrescaler << 4;// prescaler, internal peripheral clock
        TMR2  = 0;              // clear timer register
        PR2   = ServoPulseToCycle(TIMERPERIOD); 

        IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
        IntClearFlag(INT_TIMER2);
        IntEnable(INT_TIMER2);
        
        T2CONSET = Bit(15);     // start timer
    
        interrupts();

    #else // PIC16F and PIC18F

        // Capture mode makes use of the 16-bit Timer1 resources
        /*
        #if defined(__18f25k50) || defined(__18f45k50)
        T1GCON = 0;             // Timer1 counts regardless of the Timer1 gate function
        CCPTMRS  = 0;           // associate TMR1 with CCPx
        #elif defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53) 
        T1GCON = 0;             // Timer1 counts regardless of the Timer1 gate function
        CCPTMRS0 = 0;           // associate TMR1 with CCPx
        CCPTMRS1 = 0;
        CCPTMRS2 = 0;
        #else
        CCPTMRS  = 0;           // associate TMR1 with CCPx
        #endif
        */
        
        //bit 5-4 TCKPS<1:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   default prescale value
        //gPrescaler = 2;         // 1:4  prescale value
        gPrescaler = 3;         // 1:8   prescale value

        // Timer1 must be running in Timer mode or Synchronized Counter
        // mode if the CCP module is using the compare feature.
        TMR1 = 0;
        T1CON = T1_SOURCE_FOSCDIV4 | (gPrescaler << 4) | T1_16BIT | T1_ON;

        // Timer1 triggers an interrupt when it rolls over 0xFFFF
        gTimerPeriod = ServoPulseToCycle(TIMERPERIOD);

    #endif
}

//----------------------------------------------------------------------
// Attach servo to a PWM pin
// PIC32MX : the I/O pin direction is controlled by the compare module
// PIC18F  : the I/O pin direction is controlled by the user
//----------------------------------------------------------------------

void ServoAttach(u8 pwmpin)
{
    if (pwmpin < MAXNBSERVOS)
    {
        Servo[pwmpin].Attached = 1; // Attached

        // Configure the Output/Compare module
        // * P32 OCx pin are automatically managed by the OC module
        // * P8 CCPx pin must be configured as an output by clearing
        // the appropriate TRIS bit.

        switch (pwmpin)
        {
            #if defined(__PIC32MX__)

            case 0:
                OC1CON = 0x0000;        // Turn off OC1 while doing setup
                OC1CONbits.OCTSEL = 0;  // Timer2 is the clock source for this OC
                OC1CONbits.OCM = 0b110; // PWM mode on this OC; Fault pin disabled
                OC1CONSET = Bit(15);    // Output Compare peripheral is enabled
                break;
            case 1:
                OC2CON = 0x0000;        // Turn off OC1 while doing setup
                OC2CONbits.OCTSEL = 0;  // Timer2 is the clock source for this OC
                OC2CONbits.OCM = 0b110; // PWM mode on this OC; Fault pin disabled
                OC2CONSET = Bit(15);    // Output Compare peripheral is enabled
                break;
            case 2:
                OC3CON = 0x0000;        // Turn off OC1 while doing setup
                OC3CONbits.OCTSEL = 0;  // Timer2 is the clock source for this OC
                OC3CONbits.OCM = 0b110; // PWM mode on this OC; Fault pin disabled
                OC3CONSET = Bit(15);    // Output Compare peripheral is enabled
                break;
            case 3:
                OC4CON = 0x0000;        // Turn off OC1 while doing setup
                OC4CONbits.OCTSEL = 0;  // Timer2 is the clock source for this OC
                OC4CONbits.OCM = 0b110; // PWM mode on this OC; Fault pin disabled
                OC4CONSET = Bit(15);    // Output Compare peripheral is enabled
                break;
            case 4:
                OC5CON = 0x0000;        // Turn off OC1 while doing setup
                OC5CONbits.OCTSEL = 0;  // Timer2 is the clock source for this OC
                OC5CONbits.OCM = 0b110; // PWM mode on this OC; Fault pin disabled
                OC5CONSET = Bit(15);    // Output Compare peripheral is enabled
                break;
        
            #else // 8-bit PIC
            
            noInterrupts();

            #if defined(__16F1459)

            case 0:
                TRISCbits.TRISC5 = 0;   // RC5 = CCP1 pin
                CCP1CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 1:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP2 pin
                CCP2CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            case 0:
                TRISBbits.TRISB4 = 0;   // RB4 = CCP4 pin
                CCP4CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 1:
                TRISBbits.TRISB5 = 0;   // RB5 = CCP5 pin
                CCP5CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 2:
                TRISBbits.TRISB6 = 0;   // RB6 = CCP6 pin
                CCP6CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 3:
                TRISBbits.TRISB7 = 0;   // RB7 = CCP7 pin
                CCP7CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 4:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP8 pin
                CCP8CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 5:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP9 pin
                CCP9CON = SETONMATCH;   // CCPx pin is set on compare match
                break;

            case 6:
                TRISCbits.TRISC7 = 0;   // RC7 = CCP10 pin
                CCP10CON = SETONMATCH;  // CCPx pin is set on compare match
                break;

            #else

            case 0:
                TRISCbits.TRISC2 = 0;   // CCP1 pin = RC2 = Output
                CCPR1 = Servo[0].Phase0;
                CCP1CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR1bits.CCP1IP  = 1;   // INT_HIGH_PRIORITY
                PIR1bits.CCP1IF  = 0;   // Clear interrupt flag
                PIE1bits.CCP1IE  = 1;   // INT_ENABLE
                break;

            case 1:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP2 pin
                CCPR2 = Servo[1].Phase0;
                CCP2CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR2bits.CCP2IP  = 1;   // INT_HIGH_PRIORITY
                PIR2bits.CCP2IF  = 0;   // Clear interrupt flag
                PIE2bits.CCP2IE  = 1;   // INT_ENABLE
                break;

            #endif
            
            interrupts();
            
            #endif
        }
    }
}

//----------------------------------------------------------------------
// Detach servo from its pin
//----------------------------------------------------------------------

void ServoDetach(u8 pwmpin)
{
    if (pwmpin < MAXNBSERVOS)
    {
        Servo[pwmpin].Attached = 0;   // Detached

        // Turn off the Output/Compare module
        switch (pwmpin)
        {
            #if defined(__PIC32MX__)

            case 0: OC1CONCLR = Bit(15); break;
            case 1: OC2CONCLR = Bit(15); break;
            case 2: OC3CONCLR = Bit(15); break;
            case 3: OC4CONCLR = Bit(15); break;
            case 4: OC5CONCLR = Bit(15); break;

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            case 0: CCP4CON  = 0x00; break;
            case 1: CCP5CON  = 0x00; break;
            case 2: CCP6CON  = 0x00; break;
            case 3: CCP7CON  = 0x00; break;
            case 4: CCP8CON  = 0x00; break;
            case 5: CCP9CON  = 0x00; break;
            case 6: CCP10CON = 0x00; break;

            #else

            case 0: CCP1CON  = 0x00; break;
            case 1: CCP2CON  = 0x00; break;

            #endif
        }
    }
}

//----------------------------------------------------------------------
// Return 1 if the servo is currently attached.
//----------------------------------------------------------------------

u8 ServoAttached(u8 pwmpin)
{
    if (pwmpin < MAXNBSERVOS)
        return Servo[pwmpin].Attached;
    else
        return 0;
}

//----------------------------------------------------------------------
// Set the duration of the 0 degree PulseWidth in microseconds.
// Default MinPulseWidth value is ABSOLUTE_MIN_DUTY microseconds.
//----------------------------------------------------------------------

void ServoSetMinimumPulse(u8 pwmpin, u16 duty)
{
    // Check if number of servo is valid:
    if (pwmpin < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (duty < ABSOLUTE_MIN_DUTY)
            duty = ABSOLUTE_MIN_DUTY;
        if (duty > ABSOLUTE_MID_DUTY)
            duty = ABSOLUTE_MID_DUTY;

        Servo[pwmpin].MinPulseWidth = duty;

        // update servo range
        Servo[pwmpin].Range = Servo[pwmpin].MaxPulseWidth - Servo[pwmpin].MinPulseWidth;
    }
}

//----------------------------------------------------------------------
// Get the MinPulseWidth value
// 0 = error;
//----------------------------------------------------------------------

u16 ServoGetMinimumPulse(u8 pwmpin)
{
    // Check if number of servo is valid:
    if (pwmpin < MAXNBSERVOS)
        return Servo[pwmpin].MinPulseWidth;
    else
        return 0;
}

//----------------------------------------------------------------------
// Set the duration of the 180 degree PulseWidth in microseconds.
// Default MaxPulseWidth value is ABSOLUTE_MAX_DUTY microseconds.
//----------------------------------------------------------------------

void ServoSetMaximumPulse(u8 pwmpin, u16 duty)
{
    // Check if number of servo is valid:
    if (pwmpin < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (duty < ABSOLUTE_MID_DUTY)
            duty = ABSOLUTE_MID_DUTY;
        if (duty > ABSOLUTE_MAX_DUTY)
            duty = ABSOLUTE_MAX_DUTY;

        Servo[pwmpin].MaxPulseWidth = duty;

        // update servo range
        Servo[pwmpin].Range = Servo[pwmpin].MaxPulseWidth - Servo[pwmpin].MinPulseWidth;
    }
}

//----------------------------------------------------------------------
// Get the MinPulseWidth value
// 0 = error;
//----------------------------------------------------------------------

u16 ServoGetMaximumPulse(u8 pwmpin)
{
    // Check if number of servo is valid:
    if (pwmpin < MAXNBSERVOS)
        return Servo[pwmpin].MaxPulseWidth;
    else
        return 0;
}

//----------------------------------------------------------------------
// Command servo to turn from 0 to 180 degrees
// Convert degree to PulseWidth width
//----------------------------------------------------------------------

void ServoWrite(u8 pwmpin, u16 degrees)
{
    //SERVO_t * this;
    //this = &Servo[pwmpin];

    // Check if number of servo is valid
    if (pwmpin < MAXNBSERVOS)
    {
        // Converts degrees to pulse width
        if (degrees == 0)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MinPulseWidth;
        else if (degrees > 180)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MaxPulseWidth;
        else
            Servo[pwmpin].PulseWidth = (degrees * Servo[pwmpin].Range) / 180 + Servo[pwmpin].MinPulseWidth;
        
        ATOMIC Servo[pwmpin].Phase1 = ServoPulseToCycle(Servo[pwmpin].PulseWidth);
        #if !defined(__PIC32MX__)
        ATOMIC Servo[pwmpin].Phase0 = gTimerPeriod - Servo[pwmpin].Phase1;
        #endif
        
        Delayus(TIMERPERIOD);
    }
}

//----------------------------------------------------------------------
// Command servo to turn from MinPulseWidth to MaxPulseWidth
//----------------------------------------------------------------------

void ServoPulse(u8 pwmpin, u16 pulse)
{
    //SERVO_t * this;
    //this = &Servo[pwmpin];

    // Check if number of servo is valid
    if (pwmpin < MAXNBSERVOS)
    {
        // Converts degrees to pulse width
        if (pulse < Servo[pwmpin].MinPulseWidth)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MinPulseWidth;
        else if (pulse > Servo[pwmpin].MaxPulseWidth)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MaxPulseWidth;
        else
            Servo[pwmpin].PulseWidth = pulse;

        ATOMIC Servo[pwmpin].Phase1 = ServoPulseToCycle(Servo[pwmpin].PulseWidth);
        #if !defined(__PIC32MX__)
        ATOMIC Servo[pwmpin].Phase0 = gTimerPeriod - Servo[pwmpin].Phase1;
        #endif
        
        Delayus(TIMERPERIOD);
    }
}

//----------------------------------------------------------------------
// Return servo position in degrees
// 255 : error
// 0 to 180 : valid
//----------------------------------------------------------------------

u8 ServoRead(u8 pwmpin)
{
    // Check if number of servo is valid:
    if (pwmpin < MAXNBSERVOS)
        return (180 * (Servo[pwmpin].PulseWidth - Servo[pwmpin].MinPulseWidth) / Servo[pwmpin].Range);
    else
        return 255;
}

//----------------------------------------------------------------------
// Interrupt handler
//----------------------------------------------------------------------

#if defined(__PIC32MX__)

// Timer2 resets to zero when it equals PR2
void Timer2Interrupt(void)
{
    if (gActiveServo++ >= MAXNBSERVOS)
        gActiveServo = 0;

    // set all OCx pins low

    OC1RS = 0; OC2RS = 0; OC3RS = 0; OC4RS = 0; OC5RS = 0;

    // set pin high when OC value match timer value

    if (gActiveServo == 0 && Servo[0].Attached)
        OC1RS = Servo[0].Phase1;
    if (gActiveServo == 1 && Servo[1].Attached)
        OC2RS = Servo[1].Phase1;
    if (gActiveServo == 2 && Servo[2].Attached)
        OC3RS = Servo[2].Phase1;
    if (gActiveServo == 3 && Servo[3].Attached)
        OC4RS = Servo[3].Phase1;
    if (gActiveServo == 4 && Servo[4].Attached)
        OC5RS = Servo[4].Phase1;

    // enable interrupt again

    IntClearFlag(INT_TIMER2);
}

#else // PIC16F and PIC18F

void servo_interrupt(void)
{
    #if defined(__16F1459)
    
    #error
    
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    #error

    #else
    
    if (PIR1bits.CCP1IF && Servo[0].Attached)
    {
        PIR1bits.CCP1IF = 0;            // Allow the CCP interrupt again
        TMR1 = 0;                       // Reset the timer
        //CCP1CONbits.CCP1M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        //CCPR1 = (CCP1CONbits.CCP1M0) ? Servo[0].Phase1 : Servo[0].Phase0;
        if (CCP1CON == SETONMATCH)  // 0x08 = 0b1000
        {
            CCPR1 = Servo[0].Phase1;
            CCP1CON = CLRONMATCH;   // 0x09 = 0b1001
        }
        else                        // 0x09 = 0b1001
        {
            CCPR1 = Servo[0].Phase0;
            CCP1CON = SETONMATCH;   // 0x08 = 0b1000
        }
    }
    
    if (PIR2bits.CCP2IF && Servo[1].Attached)
    {
        PIR2bits.CCP2IF = 0;            // Allow the CCP interrupt again
        TMR1 = 0;                       // Reset the timer
        CCPR2 = (CCP2CONbits.CCP2M0) ? Servo[0].Phase0 : Servo[0].Phase1;
        CCP2CONbits.CCP2M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
    }
    
    #endif
}

#endif // __PIC32MX__

#endif // __SERVO__
