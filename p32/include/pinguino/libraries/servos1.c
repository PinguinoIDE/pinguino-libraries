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
    //#include <delayms.c>        // Delayms
    #include <interrupt.h>      // Timers definitions
#endif

#include <typedef.h>            // u8, u16, u32, ...
#include <macro.h>              // noInterrupts() and interrups()

#ifdef __PIC32MX__
#define SERVOPERIOD             20000 // 20 ms
#define MAXNBSERVOS             5
#else
    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
#define SERVOPERIOD             20000 // 21 ms
#define MAXNBSERVOS             5     // 7 
    #else
#define SERVOPERIOD             20000 // 20 ms
#define MAXNBSERVOS             2
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
    u16 PulseWidth;             // current duty cycle
    u16 MinPulseWidth;          // minimal duty cycle
    u16 MaxPulseWidth;          // maximal duty cycle
    u16 Range;                  // MaxPulseWidth - MinPulseWidth
} SERVO_t;

//----------------------------------------------------------------------
// Global
//----------------------------------------------------------------------

extern unsigned long _cpu_clock_;

volatile u8 gCount = 0;
volatile u8 gPrescaler;
volatile u8 gFpb;
#ifndef __PIC23MX__
volatile u8 gTMR1H; 
volatile u8 gTMR1L; 
#endif
volatile SERVO_t Servo[MAXNBSERVOS];

#ifdef __PIC32MX__
//----------------------------------------------------------------------
// TMR2 Period (us) = (PR2 + 1) * TMR prescaler / Fpb
// (PR2 + 1) * prescaler / Fpb = us/1000000
// (PR2 + 1) * prescaler = us * Fpb / 1000000
// PR2 = ((us * Fpb / 1000000) / prescaler) - 1
//----------------------------------------------------------------------
//#define DutyCycle(us)  ((us * gFpb) / (1 << gPrescaler) - 1)
#define DutyCycle(us)  (((us * gFpb) >> gPrescaler) - 1)
#else
//----------------------------------------------------------------------
// The TMR1 register pair (TMR1H:TMR1L) increments from 0000h to FFFFh
// and rolls over to 0000h. The Timer1 interrupt is generated on overflow.
//----------------------------------------------------------------------
//#define DutyCycle(us)  (0x10000 - (us * gFpb) / (1 << gPrescaler))
#define DutyCycle(us)  (0x10000 - ((us * gFpb) >> gPrescaler))
#endif

//----------------------------------------------------------------------
//  Initialization
//----------------------------------------------------------------------

void servo_init()
{
    u8  n;

    #ifdef __PIC32MX__
    gFpb = GetPeripheralClock() / 1000000;
    #else
    u16 duty;
    gFpb = (_cpu_clock_ / 4) / 1000000;
    #endif
    
    // Filling up the servo values table 
    for (n=0; n < MAXNBSERVOS; n++)
    {
        Servo[n].Attached = 0;   // detached
        Servo[n].MinPulseWidth = ABSOLUTE_MIN_DUTY;
        Servo[n].MaxPulseWidth = ABSOLUTE_MAX_DUTY;
        Servo[n].PulseWidth    = ABSOLUTE_MID_DUTY;
        Servo[n].Range         = ABSOLUTE_MAX_DUTY - ABSOLUTE_MIN_DUTY;
    }
    
    // Configure the Timer (interrupts every 4 ms)

    noInterrupts();
      
    #ifdef __PIC32MX__

        // Reset Output Compare module
        //OC1R = 0; OC2R = 0; OC3R = 0; OC4R = 0; OC5R = 0;

        IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
        //bit 6-4 TCKPS<2:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   prescale value
        //gPrescaler = 2;         // 1:4   prescale value
        gPrescaler = 3;         // 1:8   prescale value
        //gPrescaler = 4;         // 1:16  prescale value
        //gPrescaler = 5;         // 1:32  prescale value
        //gPrescaler = 6;         // 1:64  prescale value
        //gPrescaler = 7;         // 1:256 prescale value

        T2CON = gPrescaler << 4;// prescaler, internal peripheral clock
        TMR2  = 0;              // clear timer register
        PR2   = DutyCycle(TIMERPERIOD); 

        IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
        IntClearFlag(INT_TIMER2);
        IntEnable(INT_TIMER2);
        
        T2CONSET = Bit(15);     // start timer

    #else // PIC16F and PIC18F

        //bit 5-4 TCKPS<1:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   default prescale value
        //gPrescaler = 2;         // 1:4  prescale value
        gPrescaler = 3;         // 1:8   prescale value

        // Timer1 must be running in Timer mode or Synchronized Counter
        // mode if the CCP module is using the compare feature.
        T1CON = T1_SOURCE_FOSCDIV4 | (gPrescaler << 4) | T1_16BIT | T1_ON;
        duty   = (u16)DutyCycle(TIMERPERIOD);
        gTMR1H = (u8)high8(duty); 
        gTMR1L = (u8)low8(duty); 
        TMR1H  = gTMR1H; 
        TMR1L  = gTMR1L; 

        #if defined(__18f25k50) || defined(__18f45k50)
        // Timer1 counts regardless of the Timer1 gate function
        T1GCON = 0;
        #elif defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53) 
        // Timer1 counts regardless of the Timer1 gate function
        T1GCON = 0;
        //CCPTMRS0 = 0;
        //CCPTMRS1 = 0;
        //CCPTMRS2 = 0;
        #else
        //CCPTMRS  = 0;
        #endif
        
        #ifndef __16F1459
        IPR1bits.TMR1IP = 1;    // INT_HIGH_PRIORITY
        #endif
        PIR1bits.TMR1IF = 0;    // Setting flag to 0
        PIE1bits.TMR1IE = 1;    // INT_ENABLE

        //T1CON |= T1_ON;         // start timer
        //T1CON = 0b00110011;       // FOSC/4, 1:8, 0, 0, 16-bit, Enable
        
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

        #ifdef __PIC32MX__

        // Configure the Output/Compare module
        switch (pwmpin)
        {
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
        }
        
        #else

        // Configure the Output/Compare module
        // Users must configure the CCPx pin as an output by clearing
        // the appropriate TRIS bit.
        switch (pwmpin)
        {
            #if defined(__16F1459)

            case 0:
                TRISCbits.TRISC5 = 0;   // RC5 = CCP1 pin
                CCP1CON = 0x0000;       // Turn the CCP module off
                CCP1CONbits.CCP1M=0b0010;// CCPx pin is set on compare match
                break;

            case 1:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP2 pin
                CCP2CON = 0x0000;       // Turn the CCP module off
                CCP2CONbits.CCP2M=0b0010;// CCPx pin is set on compare match
                break;

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            case 0:
                TRISBbits.TRISB4 = 0;   // RB4 = CCP4 pin
                //CCP4CON = 0x0000;       // Turn the CCP module off
                //CCP4CON = 0b00001001;   // CCPx pin is set on compare match
                break;

            case 1:
                TRISBbits.TRISB5 = 0;   // RB5 = CCP5 pin
                CCP5CON = 0x0000;       // Turn the CCP module off
                CCP5CONbits.CCP5M=0b0010;// CCPx pin is set on compare match
                break;

            case 2:
                TRISBbits.TRISB6 = 0;   // RB6 = CCP6 pin
                CCP6CON = 0x0000;       // Turn the CCP module off
                CCP6CONbits.CCP6M=0b0010;// CCPx pin is set on compare match
                break;

            case 3:
                TRISBbits.TRISB7 = 0;   // RB7 = CCP7 pin
                CCP7CON = 0x0000;       // Turn the CCP module off
                CCP7CONbits.CCP7M=0b0010;// CCPx pin is set on compare match
                break;

            case 4:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP8 pin
                CCP8CON = 0x0000;       // Turn the CCP module off
                CCP8CONbits.CCP8M=0b0010;// CCPx pin is set on compare match
                break;

            case 5:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP9 pin
                CCP9CON = 0x0000;       // Turn the CCP module off
                CCP9CONbits.CCP9M=0b0010;// CCPx pin is set on compare match
                break;

            case 6:
                TRISCbits.TRISC7 = 0;   // RC7 = CCP10 pin
                CCP10CON = 0x0000;       // Turn the CCP module off
                CCP10CONbits.CCP10M=0b0010;// CCPx pin is set on compare match
                break;

            #else

            case 0:
                TRISCbits.TRISC2 = 0;   // RC2 = CCP1 pin
                CCP1CON = 0x0000;       // Turn the CCP module off
                CCP1CONbits.CCP1M=0b0010;// CCPx pin is set on compare match
                break;

            case 1:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP2 pin
                CCP2CON = 0x0000;       // Turn the CCP module off
                CCP2CONbits.CCP2M=0b0010;// CCPx pin is set on compare match
                break;

            #endif
        }
        #endif
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

            #else
            
                #if defined(__16F1459)

                #error

                #elif defined(__18f26j50) || defined(__18f46j50) || \
                      defined(__18f26j53) || defined(__18f46j53) || \
                      defined(__18f27j53) || defined(__18f47j53)

                case 0: CCP4CON  = 0x0000; break;
                case 1: CCP5CON  = 0x0000; break;
                case 2: CCP6CON  = 0x0000; break;
                case 3: CCP7CON  = 0x0000; break;
                case 4: CCP8CON  = 0x0000; break;
                case 5: CCP9CON  = 0x0000; break;
                case 6: CCP10CON = 0x0000; break;

                #else

                case 0: CCP1CON  = 0x0000; break;
                case 1: CCP2CON  = 0x0000; break;

                #endif
                
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
    //SERVO_t * S = Servo[pwmpin];

    // Check if number of servo is valid
    if (pwmpin < MAXNBSERVOS)
    {
        // Converts degrees to pulse width
        if (degrees == 0)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MinPulseWidth;
        else if (degrees > 180)
            Servo[pwmpin].PulseWidth = Servo[pwmpin].MaxPulseWidth;
        else
            Servo[pwmpin].PulseWidth = (degrees*Servo[pwmpin].Range) / 180 + Servo[pwmpin].MinPulseWidth;
        Delayus(TIMERPERIOD);
    }
}

//----------------------------------------------------------------------
// Command servo to turn from MinPulseWidth to MaxPulseWidth
//----------------------------------------------------------------------

void ServoPulse(u8 pwmpin, u16 pulse)
{
    //SERVO_t * S = Servo[pwmpin];

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
void Timer2Interrupt(void)
#else
void servo_interrupt(void)
#endif
{
    #ifndef __PIC32MX__
    u16 duty;
    #endif

    #ifdef __PIC32MX__
    if (IntGetFlag(INT_TIMER2))
    #else
    if (PIR1bits.TMR1IF)
    #endif
    {
        if (gCount++ >= MAXNBSERVOS)
            gCount = 0;

        // set all OC pins low

        #ifdef __PIC32MX__
        
            OC1RS = 0; OC2RS = 0; OC3RS = 0; OC4RS = 0; OC5RS = 0;
        
        #else
        
            #if defined(__16F1459)

            #error

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            // initialize CCPx pin low
            CCP4CON  = 0b00001000;
            CCP5CON  = 0b00001000;
            CCP6CON  = 0b00001000;
            CCP7CON  = 0b00001000;
            CCP8CON  = 0b00001000;
            CCP9CON  = 0b00001000;
            CCP10CON = 0b00001000;
            /*
            CCPR4H  = 0; CCPR4L  = 0;
            CCPR5H  = 0; CCPR5L  = 0;
            CCPR6H  = 0; CCPR6L  = 0;
            CCPR7H  = 0; CCPR7L  = 0;
            CCPR8H  = 0; CCPR8L  = 0;
            CCPR9H  = 0; CCPR9L  = 0;
            CCPR10H = 0; CCPR10L = 0;
            */
            #else
            
            CCPR1H  = 0; CCPR1L  = 0;
            CCPR2H  = 0; CCPR2L  = 0;
            
            #endif

        #endif
        
        // set pin high when OC value match timer value
        
        #ifdef __PIC32MX__

            if (gCount == 0 && Servo[0].Attached)
                OC1RS = DutyCycle(Servo[0].PulseWidth);
            if (gCount == 1 && Servo[1].Attached)
                OC2RS = DutyCycle(Servo[1].PulseWidth);
            if (gCount == 2 && Servo[2].Attached)
                OC3RS = DutyCycle(Servo[2].PulseWidth);
            if (gCount == 3 && Servo[3].Attached)
                OC4RS = DutyCycle(Servo[3].PulseWidth);
            if (gCount == 4 && Servo[4].Attached)
                OC5RS = DutyCycle(Servo[4].PulseWidth);

        #else
        
            #if defined(__16F1459)
            
            if (gCount == 0 && Servo[0].Attached)
            {
                duty = DutyCycle(Servo[0].PulseWidth);
                PWM1DCH  = high8(duty);
                PWM1DCL |=  low8(duty);
            }
            if (gCount == 1 && Servo[1].Attached)
            {
                duty = DutyCycle(Servo[1].PulseWidth);
                PWM2DCH  = high8(duty);
                PWM2DCL |=  low8(duty);
            }
            
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            if (gCount == 0 && Servo[0].Attached)
            {
                duty = DutyCycle(Servo[0].PulseWidth);
                CCPR4H = high8(duty);
                CCPR4L =  low8(duty);
            }
            if (gCount == 1 && Servo[1].Attached)
            {
                duty = DutyCycle(Servo[1].PulseWidth);
                CCPR5H = high8(duty);
                CCPR5L =  low8(duty);
            }
            if (gCount == 2 && Servo[2].Attached)
            {
                duty = DutyCycle(Servo[2].PulseWidth);
                CCPR6H = high8(duty);
                CCPR6L =  low8(duty);
            }
            if (gCount == 3 && Servo[3].Attached)
            {
                duty = DutyCycle(Servo[3].PulseWidth);
                CCPR7H = high8(duty);
                CCPR7L =  low8(duty);
            }
            if (gCount == 4 && Servo[4].Attached)
            {
                duty = DutyCycle(Servo[4].PulseWidth);
                CCPR8H = high8(duty);
                CCPR8L =  low8(duty);
            }
            if (gCount == 5 && Servo[5].Attached)
            {
                duty = DutyCycle(Servo[5].PulseWidth);
                CCPR9H = high8(duty);
                CCPR9L =  low8(duty);
            }
            if (gCount == 6 && Servo[6].Attached)
            {
                duty = DutyCycle(Servo[6].PulseWidth);
                CCPR10H = high8(duty);
                CCPR10L =  low8(duty);
            }
            
            #else
            
            if (gCount == 0 && Servo[0].Attached)
            {
                duty = DutyCycle(Servo[0].PulseWidth);
                CCPR1H = high8(duty);
                CCPR1L =  low8(duty);
            }
            if (gCount == 1 && Servo[1].Attached)
            {
                duty = DutyCycle(Servo[1].PulseWidth);
                CCPR2H = high8(duty);
                CCPR2L =  low8(duty);
            }
            
            #endif

        #endif
        
        // enable interrupt again
        #ifdef __PIC32MX__
        IntClearFlag(INT_TIMER2);
        #else
        // re-load TMR1 period
        TMR1H = gTMR1H;
        TMR1L = gTMR1L;
        PIR1bits.TMR1IF = 0;
        #endif
    }
}

#endif // __SERVO__
