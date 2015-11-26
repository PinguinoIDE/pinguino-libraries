/*	--------------------------------------------------------------------
	FILE:			CTMU.c
	PROJECT:		pinguino
	PURPOSE:		Driving the Charge Time Measurement Unit (CTMU)
	PROGRAMER:		regis blanchot <rblanchot@gmail.com>
	FIRST RELEASE:	26 Sep. 2012
	LAST RELEASE:	26 Sep. 2012
	----------------------------------------------------------------------------
	TODO : http://forum.allaboutcircuits.com/showthread.php?t=63487
	----------------------------------------------------------------------------
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
	------------------------------------------------------------------*/

#ifndef __CTMU_C
#define __CTMU_C

#define __CTMU__

#if !defined(__18f25k50) && !defined(__18f45k50) && \
    !defined(__18f26j50) && !defined(__18f46j50) && \
    !defined(__18f26j53) && !defined(__18f46j53) && \
    !defined(__18f27j53) && !defined(__18f47j53)

    #error "Your proc. has no built-in CTMU module"

#endif

#include <compiler.h>
#include <typedef.h>			    // Pinguino's types definitions
#include <const.h>				    // Pinguino's constants definitions
#include <macro.h>				    // Pinguino's macros definitions
#include <delayus.c>                // Pinguino's delays routines (us)
//#include <digitalw.c>               // Pinguino's Digital IO's definition
#include <analog.c>                 // Pinguino's Analog IO's definition

/*******************************************************************************
 * THEORY OF OPERATION
 *******************************************************************************
 * I = C . dV / dt
 * t = (C . V) / I
 * C = (I . t) / V
 ******************************************************************************/

#if defined(__18f25k50) || defined(__18f45k50)
#define ADREF           5.0                 // Vdd connected to A/D Vr+
#else
#define ADREF           3.3                 // Vdd connected to A/D Vr+
#endif

#define NOMINALCURRENT  (70.0*ADREF/100.0)  // 70% of full scale
#define ADSCALE         1023                // for unsigned conversion 10 sig bits
#define ETIME           125                 // time in uS
#define OPENSW          1000                // Un-pressed switch value
#define TRIP            300                 // Difference between pressed and un-pressed switch
#define HYST            65                  // amount to change from pressed to un-pressed
#define PRESSED         1
#define UNPRESSED       0

/*
To calculate the optimal value for RCAL, the nominal current must be chosen.
If the range of the CTMU current source is selected to be 0.55 uA,
the resistor value needed is calculated as
    RCAL = 2.31V/0.55 uA for a value of 4.2 MΩ.
*/

#define RCAL (NOMINALCURRENT/).027          // R value is 4200000 (4.2M)
                                            // scaled so that result is in 1/100th of uA

u8 TIME_CHARGE;
u8 CTMU_WORKING;

/// PROTOTYPES

void  CTMU_init();
float CTMU_getVoltage(u8);
float CTMU_getCurrent(u8);
float CTMU_getCapacitance(u8);
float CTMU_getTime(u8);
u8    CTMU_isPressed(u8);

/*******************************************************************************
 * CTMU setup
 ******************************************************************************* 
 * param : channel = analog pin number (0 for A0, 1 for A1, ...)
 ******************************************************************************/

void CTMU_init()
{
    /// Setup CTMU module

    //CTMUCON - CTMU Control register
    CTMUCONH = 0x00;                // make sure CTMU is disabled
    CTMUCONL = 0b10010000;          // CTMU enable, Enables edge delay generation

    //CTMUICON - CTMU Current Control Register
    CTMUICON = 0x01;                // Nominal current output = Base current level

    /// Setup AD converter
    
    analog_init();
}
 
/*******************************************************************************
 * Voltage measurement
 ******************************************************************************* 
 * Assumes AD and CTMU are enable
 * Assumes AD channel is set
 * Assumes Edge status bits are to zero
 ******************************************************************************/

#if defined(CTMUGETVOLTAGE)     || defined(CTMUGETCURRENT) || \
    defined(CTMUGETCAPACITANCE) || defined(CTMUGETTIME)

float CTMU_getVoltage(u8 channel)
{
    u8  j;
    u16 VTot = 0, Vread = 0;
    float Vavg, Vcal;

    CTMU_init();

    for(j=0;j<10;j++)
    {
        CTMUCONHbits.IDISSEN = 1;   // drain charge on the circuit
        Delayus(ETIME);             // Wait 125us
        CTMUCONHbits.IDISSEN = 0;   // end drain of circuit
        
        CTMUCONLbits.EDG1STAT = 1;  // Begin charging the circuit
        Delayus(ETIME);             // Wait 125us
        CTMUCONLbits.EDG1STAT = 0;  // Stop charging circuit using CTMU current source
        
        Vread = analogread(channel);
        /*
        PIR1bits.ADIF = 0;          // make sure A/D Int not set
        ADCON0bits.GO=1;            // and begin A/D conv.
        while(!PIR1bits.ADIF);      // Wait for A/D convert complete
        Vread = ADRES;              // Get the value from the A/D
        PIR1bits.ADIF = 0;          // Clear A/D Interrupt Flag
        */
        
        VTot += Vread;              // Add the reading to the total
    }
    
    Vavg = (float)VTot / 10.000;    // Average of 10 readings
    Vcal = (float)(Vavg / ADSCALE * ADREF);
    return Vcal;
}

#endif

/// Inductance measurement

#if defined(CTMUGETCURRENT) || defined(CTMUGETTIME)

float CTMU_getCurrent(u8 channel)
{
    float Vcal=0;
    float CTMUISrc = 0;             // current value

    CTMU_init();

    CTMUCONHbits.CTMUEN = 1;        // Enable the CTMU
    CTMUCONLbits.EDG1STAT = 0;      // Set Edge status bits to zero
    CTMUCONLbits.EDG2STAT = 0;

    Vcal = CTMU_getVoltage(channel);
    CTMUISrc = Vcal / RCAL;         // CTMUISrc is in 1/100ths of uA
    return CTMUISrc;
}

#endif

/// Capacitance measurement

#if defined(CTMUGETCAPACITANCE) || defined(CTMUGETTIME)

float CTMU_getCapacitance(u8 channel)
{
    float Vcal = 0;
    float CTMUISrc = 0;             // current value
    float CTMUCap = 0;              // capacitance value

    CTMU_init();

    CTMUCONHbits.CTMUEN = 1;        // Enable the CTMU
    CTMUCONLbits.EDG1STAT = 0;      // Set Edge status bits to zero
    CTMUCONLbits.EDG2STAT = 0;

    Vcal = CTMU_getVoltage(channel);
    CTMUISrc = Vcal / RCAL;         // CTMUISrc is in 1/100ths of uA
    CTMUCap = (CTMUISrc*ETIME/Vcal)/100;
    return CTMUCap;
}

#endif

/// Resistance measurement

// TODO

/// High-resolution time measurement

#ifdef CTMUGETTIME

float CTMU_getTime(u8 channel)
{
    float Vcal;
    float CTMUISrc;
    float CTMUCap;
    float CTMUTime;
    
    CTMU_init();

    CTMUCONHbits.CTMUEN = 1;        // Enable the CTMU
    CTMUCONLbits.EDG1STAT = 0;      // Set Edge status bits to zero
    CTMUCONLbits.EDG2STAT = 0;

    Vcal = CTMU_getVoltage(channel);
    CTMUISrc = CTMU_getCurrent(channel);
    CTMUCap = CTMU_getCapacitance(channel);
    CTMUTime = ( CTMUCap * Vcal ) / CTMUISrc;

    return CTMUTime;
}

#endif

/// CAPACITIVE TOUCH SWITCH

#ifdef CTMUISPRESSED

u8 CTMU_isPressed(u8 channel)
{
    u16 Vread;                      //storage for reading
    
    CTMU_init();
    
    CTMUCONHbits.CTMUEN = 1;        // Enable the CTMU
    CTMUCONLbits.EDG1STAT = 0;      // Set Edge status bits to zero
    CTMUCONLbits.EDG2STAT = 0;
    CTMUCONHbits.IDISSEN = 1;       // Drain charge on the circuit
    Delayus(ETIME);                 // Wait 125us
    CTMUCONHbits.IDISSEN = 0;       // End drain of circuit
    CTMUCONLbits.EDG1STAT = 1;      // Begin charging the circuit using CTMU current source
    Delayus(ETIME);                 // Wait 125us
    CTMUCONLbits.EDG1STAT = 0;      // Stop charging circuit
    
    Vread = analogread(channel);
    /*
    PIR1bits.ADIF = 0;          // make sure A/D Int not set
    ADCON0bits.GO=1;            // and begin A/D conv.
    while(!PIR1bits.ADIF);      // Wait for A/D convert complete
    Vread = ADRES;              // Get the value from the A/D
    PIR1bits.ADIF = 0;          // Clear A/D Interrupt Flag
    */

    if (Vread < OPENSW - TRIP)
    {
        return PRESSED;
    }
    else if (Vread > OPENSW - TRIP + HYST)
    {
        return UNPRESSED;
    }
}

#endif

/// Interrupt routine

/*
void ctmu_interrupt(void)
{
    // check timer0 irq 
    if (INTCONbits.TMR0IF)
    {
        // charge cycle timer0 int, because not shorting the CTMU voltage.
        if (!CTMUCONHbits.IDISSEN)
        {
            CTMUCONLbits.EDG1STAT = 0;       // Stop charging touch circuit
            TIME_CHARGE=FALSE;        // clear charging flag
            CTMU_WORKING=TRUE;        // set working flag, doing touch ADC conversion
            // configure ADC for next reading
            ADCON0bits.CHS=0;         // Select ADC channel
            ADCON0bits.ADON=1;         // Turn on ADC
            ADCON0bits.GO=1;         // and begin A/D conv, will set adc int flag when done.
            //LATCbits.LATC7=!LATCbits.LATC7;     // blink led
        }
        
        // discharge cycle timer0 int, because CTMU voltage is shorted 
        else
        {
            //LATCbits.LATC6=!LATCbits.LATC6;     // blink led
            CTMUCONHbits.IDISSEN = 0;       // end drain of touch circuit
            TIME_CHARGE=TRUE;        // set charging flag
            CTMU_WORKING=TRUE;        // set working flag, doing 
            WriteTimer0 ( charge_time );     // set timer to charge rate time
            CTMUCONLbits.EDG1STAT = 1;       // Begin charging the touch circuit
        }

        // clr  TMR0 int flag
        INTCONbits.TMR0IF = 0;         //clear interrupt flag
    }

    // check ADC irq
    if (PIR1bits.ADIF)
    {
        PIR1bits.ADIF = 0;          // clear ADC int flag
        //   LATCbits.LATC5=!LATCbits.LATC5;      // blink led
        Vread = ADRES;           // Get the value from the A/D
        Vread= (Vread >>3)&0x007f;       // toss lower bit noise and mask

        // see if we have a pressed button
        if (Vread < (touch_base - TRIP))
        {
            switchState = PRESSED;
            LATCbits.LATC4=OFF;
        }

        else if (Vread > (touch_base - TRIP + HYST))
        {
            switchState = UNPRESSED;
            LATCbits.LATC4=ON;
        }

        CTMU_ADC_UPDATED=TRUE;        // New data is in Vread, set to FALSE in main program flow
        CTMU_WORKING=FALSE;         // clear working flag, ok to read Vread.
        // config CTMU for next reading
        CTMUCONHbits.CTMUEN = 1;        // Enable the CTMU
        CTMUCONLbits.EDG1STAT = 0;        // Set Edge status bits to zero
        CTMUCONLbits.EDG2STAT = 0;
        CTMUCONHbits.IDISSEN = 1;        // drain charge on the circuit
        WriteTimer0 ( TIMERDISCHARGE );      // set timer to discharge rate
    }

    // clear TMR2 int flag
    if (PIR1bits.TMR2IF)
        PIR1bits.TMR2IF = 0;
}
*/

#endif /* __CTMU_C */
