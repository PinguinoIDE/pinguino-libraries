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
#endif

#include <typedef.h>            // u8, u16, u32, ...
#include <macro.h>              // noInterrupts(), interrups(), ATOMIC

//#define SERIALUSEPORT1
//#define SERIALPRINTNUMBER
//#define SERIALPRINT
//#include <serial.c>             // SerialPrintNumber, ...

#define SERVOPERIOD             20000 // 1/50 Hz = 20 ms = 20 000 us
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
        extern volatile u16     CCPR4  @ 0xF13;
        extern volatile u16     CCPR5  @ 0xF10;
        extern volatile u16     CCPR6  @ 0xF0D;
        extern volatile u16     CCPR7  @ 0xF0A;
        extern volatile u16     CCPR8  @ 0xF07;
        extern volatile u16     CCPR9  @ 0xF04;
        extern volatile u16     CCPR10 @ 0xF01;
    #else
        //extern volatile u16     CCPR1  @ 0xFBE;
        //extern volatile u16     CCPR2  @ 0xF90;
        #define MAXNBSERVOS     2
    #endif
#endif

//----------------------------------------------------------------------
// Absolute Servos Spec.
//
// 500 (us)       1000            1500            2000             2500
//   |---------------|---------------|---------------|----------------|
//   0 (degrees)    45              90             135              180
//
//----------------------------------------------------------------------

#define ABSOLUTE_MIN_PULSEWIDTH 500     // 0   deg <=> 0.5 ms ( 500 us)
#define ABSOLUTE_MAX_PULSEWIDTH 2500    // 180 deg <=> 2.5 ms (2500 us)
#define ABSOLUTE_MID_PULSEWIDTH ((ABSOLUTE_MIN_PULSEWIDTH + ABSOLUTE_MAX_PULSEWIDTH) / 2)

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

u8 gPrescaler = 0;
u8 gFpb = 0;

#if defined(__PIC32MX__)
volatile u8 gActiveServo = 0;
#else
u16 gServoPeriod;
#endif

SERVO_t Servo[MAXNBSERVOS];

//----------------------------------------------------------------------
//  Calculation
//----------------------------------------------------------------------
// Signal period (us) = (PR2 + 1) * TMR2 prescaler / Fpb
// (PR2 + 1) * prescaler / Fpb = us/1000000
// (PR2 + 1) * prescaler = us * Fpb / 1000000
// PR2 = ((us * Fpb / 1000000) / prescaler) - 1

#define ServoPulseToCycle(us)   (((us * gFpb) >> gPrescaler) - 1)

//----------------------------------------------------------------------
//  Initialization
//----------------------------------------------------------------------

void servo_init()
{
    u8  n;

    #if defined(__PIC32MX__)
    gFpb = GetPeripheralClock() / 1000000;
    #else
    gFpb = _cpu_clock_ / 4000000;
    #endif

    //Serial_begin(UART1, 9600, NULL);
    //Serial_print(UART1, "TEST\r\n");

    // Filling up the servo values table 
    for (n = 0; n < MAXNBSERVOS; n++)
    {
        Servo[n].Attached = 0;   // detached
        Servo[n].MinPulseWidth = ABSOLUTE_MIN_PULSEWIDTH;
        Servo[n].MaxPulseWidth = ABSOLUTE_MAX_PULSEWIDTH;
        Servo[n].PulseWidth    = ABSOLUTE_MID_PULSEWIDTH;
        Servo[n].Range         = ABSOLUTE_MAX_PULSEWIDTH - ABSOLUTE_MIN_PULSEWIDTH;
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

        // Max nb of cycles value is with :
        // - Period = 20000 us
        // - Fpb = 80 MHz
        // 20000 * 80 / 32 = 0xC350 = 16-bit number
        gPrescaler = 5;         // 1:32  prescale value

        //gPrescaler = 6;         // 1:64  prescale value
        //gPrescaler = 7;         // 1:256 prescale value

        TMR2  = 0;              // clear timer register
        T2CON = gPrescaler << 4;// prescaler, internal peripheral clock
        PR2   = ServoPulseToCycle(TIMERPERIOD); 

        IntSetVectorPriority(INT_TIMER2_VECTOR, 7, 3);
        IntClearFlag(INT_TIMER2);
        IntEnable(INT_TIMER2);
        
        T2CONSET = Bit(15);     // start timer
    
        interrupts();

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
        #elif !defined(__16f1459)
        CCPTMRS  = 0;           // associate TMR1 with CCPx
        #endif
        
        //bit 5-4 TCKPS<1:0>: Timer Input Clock Prescale Select bits
        //gPrescaler = 0;         // 1:1   default prescale value
        //gPrescaler = 1;         // 1:2   default prescale value
        //gPrescaler = 2;         // 1:4  prescale value
        gPrescaler = 3;         // 1:8   prescale value

        // Timer1 must be running in Timer mode or Synchronized Counter
        // mode if the CCP module is using the compare feature.
        TMR1 = 0;
        T1CON = T1_SOURCE_FOSCDIV4 | (gPrescaler << 4) | T1_16BIT | T1_ON;
        
        gServoPeriod = ServoPulseToCycle(SERVOPERIOD);

    #endif
}

//----------------------------------------------------------------------
// Attach servo to a PWM pin
// PIC32MX : the I/O pin direction is controlled by the compare module
// PIC18F  : the I/O pin direction is controlled by the user
//----------------------------------------------------------------------

void ServoAttach(u8 num)
{
    if (num < MAXNBSERVOS)
    {
        Servo[num].Attached = 1; // Attached

        // Configure the Output/Compare module
        // * P32 OCx pin are automatically managed by the OC module
        // * P8 CCPx pin must be configured as an output by clearing
        // the appropriate TRIS bit.

        switch (num)
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

            #if defined(__16f1459)

            case 0:
                TRISCbits.TRISC5 = 0;   // RC5 = C1OUT pin
                CM1CON0bits.C1OE = 1;   // C1OUT signal on C1OUT pin
                CM1CON0bits.C1ON = 1;   // Comparator 1 enable
                PIR2bits.C1IF  = 0;     // Clear interrupt flag
                PIE2bits.C1IE  = 1;     // INT_ENABLE
                break;

            case 1:
                TRISCbits.TRISC6 = 0;   // RC6 = C2OUT pin
                CM2CON0bits.C2OE = 1;   // C2OUT signal on C2OUT pin
                CM2CON0bits.C2ON = 1;   // Comparator 2 enable
                PIR2bits.C2IF  = 0;     // Clear interrupt flag
                PIE2bits.C2IE  = 1;     // INT_ENABLE
                break;

            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)

            case 0:
                TRISBbits.TRISB4 = 0;   // RB4 = CCP4 pin
                CCPR4 = 0; 
                CCP4CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP4IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP4IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP4IE  = 1;   // INT_ENABLE
                break;

            case 1:
                TRISBbits.TRISB5 = 0;   // RB5 = CCP5 pin
                CCPR5 = 0;
                CCP5CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP5IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP5IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP5IE  = 1;   // INT_ENABLE
                break;

            case 2:
                TRISBbits.TRISB6 = 0;   // RB6 = CCP6 pin
                CCPR6 = 0;
                CCP6CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP6IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP6IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP6IE  = 1;   // INT_ENABLE
                break;

            case 3:
                TRISBbits.TRISB7 = 0;   // RB7 = CCP7 pin
                CCPR7 = 0;
                CCP7CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP7IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP7IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP7IE  = 1;   // INT_ENABLE
                break;

            case 4:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP8 pin
                CCPR8 = 0;
                CCP8CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP8IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP8IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP8IE  = 1;   // INT_ENABLE
                break;

            case 5:
                TRISCbits.TRISC6 = 0;   // RC6 = CCP9 pin
                CCPR9 = 0;
                CCP9CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR4bits.CCP9IP  = 1;   // INT_HIGH_PRIORITY
                PIR4bits.CCP9IF  = 0;   // Clear interrupt flag
                PIE4bits.CCP9IE  = 1;   // INT_ENABLE
                break;

            case 6:
                TRISCbits.TRISC7 = 0;   // RC7 = CCP10 pin
                CCPR10 = 0;
                CCP10CON = SETONMATCH;  // CCPx pin is set on compare match
                IPR4bits.CCP10IP  = 1;  // INT_HIGH_PRIORITY
                PIR4bits.CCP10IF  = 0;  // Clear interrupt flag
                PIE4bits.CCP10IE  = 1;  // INT_ENABLE
                break;

            #else

            case 0:
                TRISCbits.TRISC2 = 0;   // CCP1 pin = RC2 = Output
                CCPR1 = 0;
                CCP1CON = SETONMATCH;   // CCPx pin is set on compare match
                IPR1bits.CCP1IP  = 1;   // INT_HIGH_PRIORITY
                PIR1bits.CCP1IF  = 0;   // Clear interrupt flag
                PIE1bits.CCP1IE  = 1;   // INT_ENABLE
                break;

            case 1:
                TRISCbits.TRISC1 = 0;   // RC1 = CCP2 pin
                CCPR2 = 0;
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

void ServoDetach(u8 num)
{
    if (num < MAXNBSERVOS)
    {
        Servo[num].Attached = 0;   // Detached

        // Turn off the Output/Compare module
        switch (num)
        {
            #if defined(__PIC32MX__)

            case 0: OC1CONCLR = Bit(15); break;
            case 1: OC2CONCLR = Bit(15); break;
            case 2: OC3CONCLR = Bit(15); break;
            case 3: OC4CONCLR = Bit(15); break;
            case 4: OC5CONCLR = Bit(15); break;

            #elif defined(__16f1459)
            
            case 0: CM1CON0 = 0x000; break;
            case 1: CM2CON0 = 0x000; break;

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

u8 ServoAttached(u8 num)
{
    if (num < MAXNBSERVOS)
        return Servo[num].Attached;
    else
        return 0;
}

//----------------------------------------------------------------------
// Set the duration of the 0 degree PulseWidth in microseconds.
// Default MinPulseWidth value is ABSOLUTE_MIN_PULSEWIDTH microseconds.
//----------------------------------------------------------------------

void ServoSetMinimumPulse(u8 num, u16 pulse)
{
    // Check if number of servo is valid:
    if (num < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (pulse < ABSOLUTE_MIN_PULSEWIDTH)
            pulse = ABSOLUTE_MIN_PULSEWIDTH;
        if (pulse > ABSOLUTE_MID_PULSEWIDTH)
            pulse = ABSOLUTE_MID_PULSEWIDTH;

        Servo[num].MinPulseWidth = pulse;

        // update servo range
        Servo[num].Range = Servo[num].MaxPulseWidth - Servo[num].MinPulseWidth;
    }
}

//----------------------------------------------------------------------
// Get the MinPulseWidth value
// 0 = error;
//----------------------------------------------------------------------

u16 ServoGetMinimumPulse(u8 num)
{
    // Check if number of servo is valid:
    if (num < MAXNBSERVOS)
        return Servo[num].MinPulseWidth;
    else
        return 0;
}

//----------------------------------------------------------------------
// Set the duration of the 180 degree PulseWidth in microseconds.
// Default MaxPulseWidth value is ABSOLUTE_MAX_PULSEWIDTH microseconds.
//----------------------------------------------------------------------

void ServoSetMaximumPulse(u8 num, u16 pulse)
{
    // Check if number of servo is valid:
    if (num < MAXNBSERVOS)
    {
        // test if microseconds are within range:
        if (pulse < ABSOLUTE_MID_PULSEWIDTH)
            pulse = ABSOLUTE_MID_PULSEWIDTH;
        if (pulse > ABSOLUTE_MAX_PULSEWIDTH)
            pulse = ABSOLUTE_MAX_PULSEWIDTH;

        Servo[num].MaxPulseWidth = pulse;

        // update servo range
        Servo[num].Range = Servo[num].MaxPulseWidth - Servo[num].MinPulseWidth;
    }
}

//----------------------------------------------------------------------
// Get the MinPulseWidth value
// 0 = error;
//----------------------------------------------------------------------

u16 ServoGetMaximumPulse(u8 num)
{
    // Check if number of servo is valid:
    if (num < MAXNBSERVOS)
        return Servo[num].MaxPulseWidth;
    else
        return 0;
}

//----------------------------------------------------------------------
// Command servo to turn from 0 to 180 degrees
// Convert degree to PulseWidth width
//----------------------------------------------------------------------

void ServoWrite(u8 num, int degrees)
{
    u32 tmp;
    
    // Check if number of servo is valid
    if (num < MAXNBSERVOS)
    {
        // Normalize the angle to be in [0, 180[
        if (degrees < 0)
            degrees = 360 - ((-degrees) % 360);
        else
            degrees %= 360;

        if (degrees >= 180)
            degrees -= 180;

        // Convert degrees to pulse width
        tmp = (u32)degrees * (u32)Servo[num].Range / 180UL;
        Servo[num].PulseWidth = (u16)(tmp + Servo[num].MinPulseWidth);

        // Convert pulse width to nb of CPU cycles
        ATOMIC Servo[num].Phase1 = ServoPulseToCycle(Servo[num].PulseWidth);
        #if !defined(__PIC32MX__)
        ATOMIC Servo[num].Phase0 = gServoPeriod - Servo[num].Phase1;
        #endif
        
        #if defined(__PIC32MX__)
        Delayus(TIMERPERIOD);
        #else
        Delayus(SERVOPERIOD);
        #endif
    }
}

//----------------------------------------------------------------------
// Command servo to turn from MinPulseWidth to MaxPulseWidth
//----------------------------------------------------------------------

void ServoPulse(u8 num, u16 pulse)
{
    // Check if number of servo is valid
    if (num < MAXNBSERVOS)
    {
        // Converts degrees to pulse width
        if (pulse < Servo[num].MinPulseWidth)
            Servo[num].PulseWidth = Servo[num].MinPulseWidth;
        else if (pulse > Servo[num].MaxPulseWidth)
            Servo[num].PulseWidth = Servo[num].MaxPulseWidth;
        else
            Servo[num].PulseWidth = pulse;

        ATOMIC Servo[num].Phase1 = ServoPulseToCycle(Servo[num].PulseWidth);
        #if !defined(__PIC32MX__)
        ATOMIC Servo[num].Phase0 = gServoPeriod - Servo[num].Phase1;
        #endif
        
        #if defined(__PIC32MX__)
        Delayus(TIMERPERIOD);
        #else
        Delayus(SERVOPERIOD);
        #endif
    }
}

//----------------------------------------------------------------------
// Return servo position in degrees
// 255 : error
// 0 to 180 : valid
//----------------------------------------------------------------------

u8 ServoRead(u8 num)
{
    // Check if number of servo is valid:
    if (num < MAXNBSERVOS)
        return (180 * (Servo[num].PulseWidth - Servo[num].MinPulseWidth) / Servo[num].Range);
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
    
    if (PIR2bits.C1IF) // && Servo[0].Attached)
    {
        //CCPR1 = TMR1 + ((CCP1CONbits.CCP1M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        //CCP1CONbits.CCP1M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR2bits.C1IF = 0;            // Allow the CCP interrupt again
    }
    
    if (PIR2bits.C2IF) // && Servo[1].Attached)
    {
        //CCPR2 = TMR1 + ((CCP2CONbits.CCP2M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        //CCP2CONbits.CCP2M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR2bits.C2IF = 0;            // Allow the CCP interrupt again
    }
    
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    if (PIR4bits.CCP4IF) // && Servo[0].Attached)
    {
        CCPR4 = TMR1 + ((CCP4CONbits.CCP4M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP4CONbits.CCP4M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP4IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP5IF) // && Servo[0].Attached)
    {
        CCPR5 = TMR1 + ((CCP5CONbits.CCP5M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP5CONbits.CCP5M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP5IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP6IF) // && Servo[0].Attached)
    {
        CCPR6 = TMR1 + ((CCP6CONbits.CCP6M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP6CONbits.CCP6M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP6IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP7IF) // && Servo[0].Attached)
    {
        CCPR7 = TMR1 + ((CCP7CONbits.CCP7M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP7CONbits.CCP7M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP7IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP8IF) // && Servo[0].Attached)
    {
        CCPR8 = TMR1 + ((CCP8CONbits.CCP8M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP8CONbits.CCP8M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP8IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP9IF) // && Servo[0].Attached)
    {
        CCPR9 = TMR1 + ((CCP9CONbits.CCP9M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP9CONbits.CCP9M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP9IF = 0;            // Allow the CCP interrupt again
    }

    if (PIR4bits.CCP10IF) // && Servo[0].Attached)
    {
        CCPR10 = TMR1 + ((CCP10CONbits.CCP10M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP10CONbits.CCP10M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR4bits.CCP10IF = 0;            // Allow the CCP interrupt again
    }

    #else
    
    if (PIR1bits.CCP1IF) // && Servo[0].Attached)
    {
        CCPR1 = TMR1 + ((CCP1CONbits.CCP1M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP1CONbits.CCP1M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR1bits.CCP1IF = 0;            // Allow the CCP interrupt again
    }
    
    if (PIR2bits.CCP2IF) // && Servo[1].Attached)
    {
        CCPR2 = TMR1 + ((CCP2CONbits.CCP2M0) ? Servo[0].Phase0 : Servo[0].Phase1);
        CCP2CONbits.CCP2M0 ^= 1;        // Toggle SETONMATCH (0) / CLRONMATCH (1)
        PIR2bits.CCP2IF = 0;            // Allow the CCP interrupt again
    }
    
    #endif
}

#endif // __PIC32MX__

#endif // __SERVO__
