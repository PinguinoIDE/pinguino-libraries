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
#ifdef __PIC32MX__
    #include <delay.c>          // Delayus and Delayms
    #include <interrupt.c>      // Timers functions
    #include <system.c>         // GetPeripheralClock
#else
    #include <compiler.h>
    #include <delayus.c>        // Delayus
    #include <interrupt.h>      // Timers definitions
#endif

#include <typedef.h>            // u8, u16, u32, ...
#include <macro.h>              // noInterrupts() and interrups()

#ifdef __PIC32MX__
    #define SERVOPERIOD         20000 // 20 ms = 20 000 us
    #define MAXNBSERVOS         5
#else
    #define TOGONMATCH          0x02  // CCPx pin toggle on compare match
    #define SETONMATCH          0x08  // CCPx pin set on compare match
    #define CLRONMATCH          0x09  // CCPx pin cleared on compare match
    #define CCPMATCH            CLRONMATCH
    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #define SERVOPERIOD     21000 // 21 ms = 21 000 us
        #define MAXNBSERVOS     7 
    #else
        #define SERVOPERIOD     20000 // 20 ms = 20 000 us
        #define MAXNBSERVOS     2
    #endif
#endif
#define TIMERPERIOD             (SERVOPERIOD / MAXNBSERVOS)

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
    u16 DutyCycle;              // duty cycle
} SERVO_t;

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------

#ifndef __PIC23MX__
extern unsigned long _cpu_clock_;
#endif

volatile u8 gActiveServo = 0;
volatile u8 gPrescaler = 0;
volatile u8 gFpb = 0;
#ifndef __PIC23MX__
volatile t16 gPeriod;
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

u16 ServoDutyCycle(u16 us)
{
    u32 tmp = us * gFpb;
    tmp >>= gPrescaler;
    #ifdef __PIC23MX__
    return (u16)(tmp - 1);
    #else
    return (u16)(0x10000 - tmp);
    #endif
}

//----------------------------------------------------------------------
//  Initialization
//----------------------------------------------------------------------

void servo_init()
{
    u8  n;

    #ifdef __PIC32MX__
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
        Servo[n].DutyCycle     = ServoDutyCycle(ABSOLUTE_MID_DUTY);
    }
    
    // Configure the Timer

    noInterrupts();
      
    #ifdef __PIC32MX__

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
        PR2   = ServoDutyCycle(TIMERPERIOD); 

        IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
        IntClearFlag(INT_TIMER2);
        IntEnable(INT_TIMER2);
        
        T2CONSET = Bit(15);     // start timer

    #else // PIC16F and PIC18F

        // Capture mode makes use of the 16-bit Timer1 resources
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

        //bit 5-4 TCKPS<1:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   default prescale value
        //gPrescaler = 2;         // 1:4  prescale value
        gPrescaler = 3;         // 1:8   prescale value

        // Timer1 must be running in Timer mode or Synchronized Counter
        // mode if the CCP module is using the compare feature.
        T1CON = T1_SOURCE_FOSCDIV4 | (gPrescaler << 4) | T1_16BIT | T1_ON;

        // Timer1 triggers an interrupt when it rolls over 0xFFFF
        gPeriod.w = ServoDutyCycle(TIMERPERIOD);

        // First interrupt will occur very soon
        TMR1H  = 0xFF; 
        TMR1L  = 0x00; 

        #ifndef __16F1459
        IPR1bits.TMR1IP = 1;    // INT_HIGH_PRIORITY
        #endif
        PIR1bits.TMR1IF = 0;    // Setting flag to 0
        PIE1bits.TMR1IE = 1;    // INT_ENABLE

    #endif
    
    interrupts();
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

        //noInterrupts();

        switch (pwmpin)
        {
            #ifdef __PIC32MX__

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
        
            #elif defined(__16F1459)

            case 0:
                TRISCbits.TRISC5 = 0;   // RC5 = CCP1 pin
                CCP1CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 1:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP2 pin
                CCP2CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            case 0:
                TRISBbits.TRISB4 = 0;   // RB4 = CCP4 pin
                CCP4CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 1:
                TRISBbits.TRISB5 = 0;   // RB5 = CCP5 pin
                CCP5CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 2:
                TRISBbits.TRISB6 = 0;   // RB6 = CCP6 pin
                CCP6CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 3:
                TRISBbits.TRISB7 = 0;   // RB7 = CCP7 pin
                CCP7CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 4:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP8 pin
                CCP8CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 5:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP9 pin
                CCP9CON = CCPMATCH;  // CCPx pin is set on compare match
                break;

            case 6:
                TRISCbits.TRISC7 = 0;   // RC7 = CCP10 pin
                CCP10CON = CCPMATCH; // CCPx pin is set on compare match
                break;

            #elif defined(__16F1459)
            
                #error
                
            #else

            case 0:
                TRISCbits.TRISC2 = 0;   // CCP1 pin = RC2 = Output
                /*
                IPR1bits.CCP1IP  = 1;   // INT_HIGH_PRIORITY
                PIR1bits.CCP1IF  = 0;   // Clear interrupt flag
                PIE1bits.CCP1IE  = 1;   // INT_ENABLE
                CCP1CON = SETONMATCH;   // CCPx pin is set on compare match
                CCPR1H = TMR1H + high8(ServoDutyCycle(ABSOLUTE_MID_DUTY));
                CCPR1L = TMR1L +  low8(ServoDutyCycle(ABSOLUTE_MID_DUTY));
                */
                break;

            case 1:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP2 pin
                CCP2CON = CCPMATCH;     // CCPx pin is set on compare match
                break;

            #endif
        }

        //interrupts();
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
            #ifdef __PIC32MX__

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
        
        Servo[pwmpin].DutyCycle = ServoDutyCycle(Servo[pwmpin].PulseWidth);

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

        Servo[pwmpin].DutyCycle = ServoDutyCycle(Servo[pwmpin].PulseWidth);

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

#ifdef __PIC32MX__

// Timer2 resets to zero when it equals PR2
void Timer2Interrupt(void)
{
    if (gActiveServo++ >= MAXNBSERVOS)
        gActiveServo = 0;

    // set all OCx pins low

    OC1RS = 0; OC2RS = 0; OC3RS = 0; OC4RS = 0; OC5RS = 0;

    // set pin high when OC value match timer value

    if (gActiveServo == 0 && Servo[0].Attached)
        OC1RS = Servo[0].DutyCycle;
    if (gActiveServo == 1 && Servo[1].Attached)
        OC2RS = Servo[1].DutyCycle;
    if (gActiveServo == 2 && Servo[2].Attached)
        OC3RS = Servo[2].DutyCycle;
    if (gActiveServo == 3 && Servo[3].Attached)
        OC4RS = Servo[3].DutyCycle;
    if (gActiveServo == 4 && Servo[4].Attached)
        OC5RS = Servo[4].DutyCycle;

    // enable interrupt again

    IntClearFlag(INT_TIMER2);
}

#else // PIC16F and PIC18F

void servo_interrupt(void)
{
    t16 duty;

    if (PIR1bits.TMR1IF)
    {
        if (gActiveServo++ >= MAXNBSERVOS)
            gActiveServo = 0;

        #if defined(__16F1459)
        
        #error
        
        #elif defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53)

        #error

        #else
        
        // Set all CCPx pin low
        // Selecting the compare output mode, forces the state of the
        // CCP pin to the state that is opposite of the match state. 
        CCP1CON = SETONMATCH;
            CCPR1H = gPeriod.h8;
            CCPR1L = gPeriod.l8;

        // Set CCPRx ahead of TMR1
        if (gActiveServo == 0 && Servo[0].Attached)
        {
            CCPR1 = Servo[0].DutyCycle;
            /*
            duty.w = Servo[0].DutyCycle;
            CCPR1H = duty.h8;
            CCPR1L = duty.l8;
            */
        }
            
        #endif // __16F1459

        // Re-init the Timer
        TMR1H = gPeriod.h8;
        TMR1L = gPeriod.l8;

        // Allow the interrupt again
        PIR1bits.TMR1IF = 0;
    }
}

#endif // __PIC32MX__

#endif // __SERVO__

/*
    if (PIR1bits.CCP1IF && Servo[0].Attached && gActiveServo == 0)
    {
        duty.w = ServoDutyCycle(Servo[0].PulseWidth);
        if (CCP1CON == SETONMATCH)
        {
            CCP1CON = CLRONMATCH;       // CCPx pin will be low
            CCPR1H = TMR1H + duty.h8;   // next time
            CCPR1L = TMR1L + duty.l8;   // next time
        }
        else
        {
            CCP1CON = SETONMATCH;       // CCPx pin will be high
            CCPR1H = TMR1H + gPeriod.h8 - duty.h8;
            CCPR1L = TMR1L + gPeriod.l8 - duty.l8;
        }
        PIR1bits.CCP1IF = 0;            // enable interrupt again
    }
*/
