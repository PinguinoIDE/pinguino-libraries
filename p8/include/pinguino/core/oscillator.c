/*  --------------------------------------------------------------------
    FILE:           oscillator.c
    PROJECT:        pinguino
    PURPOSE:        pinguino system clock switching functions
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    The Clock Switching mode is controlled by the SCS bits in the OSCCON
    register. These bits can be changed via software during run time to
    change the clock source.
    There are three choices that can be selected for the system clock
    via the clock switching feature. They include:
    * System Clock determined by the Fosc settings in the Configuration Word
    * INTOSC – Internal Oscillator
    * Timer1 External 32.768 Khz clock crystal
    --------------------------------------------------------------------
    CHANGELOG:
    05 Jan. 2011 - Régis Blanchot - first release
    21 Nov. 2012 - Régis Blanchot - added PINGUINO1220,1320,14k22 support
    07 Dec. 2012 - Régis Blanchot - added PINGUINO25K50 and 45K50 support
                                    added low power functions
    06 Jan. 2015 - Régis Blanchot - fixed some bugs
    08 Sep. 2015 - Régis Blanchot - added PINGUINO1459 support
    20 Sep. 2015 - Régis Blanchot - added EUSART Receiver Idle Status check before
                                    changing System Clock
    27 Jan. 2016 - Régis Blanchot - added PIC16F1708 support
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

#ifndef __OSCILLATOR_C
#define __OSCILLATOR_C

#include <compiler.h>
#include <typedef.h>
#include <const.h>
#include <macro.h>

#if defined(_PIC14E) || (!defined(_PIC14E) && !defined(__XC8__))
    #ifndef FLASHREAD
    #define FLASHREAD
    #endif
    #include <flash.c>
#endif

#if defined(_MILLIS_C_)
extern volatile t16 _period;
#endif

volatile u32 _cpu_clock_ = 48000000;

// The indices are valid values for PLLDIV
//static const u8 plldiv[] = { 12, 10, 6, 5, 4, 3, 2, 1 };

#if defined(__16F1708)

    // INTOSC Frequency
    #define FINTOSC     8000000
    // The indices are valid values for CPDIV
    static const u8 _cpudiv[] = { 1, 2, 3, 6 };
    // The indices are valid values for IRCF
    static const u32 ircf[] = { 31000, 31250, 62500, 125000, 250000, 500000, 1000000, 2000000, 4000000, 8000000, 16000000 };

#elif defined(__16F1459)

    // INTOSC Frequency
    #define FINTOSC     16000000
    // The indices are valid values for CPDIV
    static const u8 _cpudiv[] = { 1, 2, 3, 6 };
    // The indices are valid values for IRCF
    static const u32 ircf[] = { 31000, 31000, 31250, 31250, 62500, 125000, 250000, 500000, 125000, 250000, 500000, 1000000, 2000000, 4000000, 8000000, 16000000 };

#elif   defined(__18f14k22) || \
        defined(__18f25k50) || defined(__18f45k50)

    // INTOSC Frequency
    #define FINTOSC     16000000
    // The indices are valid values for CPDIV
    static const u8 _cpudiv[] = { 1, 2, 3, 6 };
    // The indices are valid values for IRCF
    static const u32 ircf[] = { 31250, 250000, 500000, 1000000, 2000000, 4000000, 8000000, 16000000 };

#elif   defined(__18f2455)  || defined(__18f4455)  || \
        defined(__18f2550)  || defined(__18f4550)   

    // INTOSC Frequency
    #define FINTOSC     8000000
    // The indices are valid values for CPDIV
    static const u8 _cpudiv[] = { 2, 3, 4, 6 };
    static const u8 _cpudiv_nopll[] = { 1, 2, 3, 4 };
    // The indices are valid values for IRCF
    static const u32 ircf[] = { 31250, 125000, 250000, 500000, 1000000, 2000000, 4000000, 8000000 };

#elif   defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f27j53) || defined(__18f47j53)

    // INTOSC Frequency
    #define FINTOSC     8000000
    // The indices are valid values for CPDIV
    static const u8 _cpudiv[] = { 6, 3, 2, 1 };
    // The indices are valid values for IRCF
    static const u32 ircf[] = { 31250, 125000, 250000, 500000, 1000000, 2000000, 4000000, 8000000 };

#else

    #error "*** Processor not supported ***"
    
#endif

// Crystal default value
// Can be defined by user in his .pde file
// for ex. #define CRYSTAL = 8000000L
#ifndef CRYSTAL
    #if   defined(__16F1459)  || defined(__16F1708)  || \
          defined(__18f25k50) || defined(__18f45k50)
        #define CRYSTAL FINTOSC
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)
        #define CRYSTAL 8000000L
    #else
        #define CRYSTAL 20000000L
    #endif
#endif

// Different frequencies available
#define _64MHZ_     64000000
#define _48MHZ_     48000000
#define _32MHZ_     32000000
#define _16MHZ_     16000000
#define _8MHZ_       8000000
#define _4MHZ_       4000000
#define _2MHZ_       2000000
#define _1MHZ_       1000000
#define _500KHZ_      500000
#define _250KHZ_      250000
#define _125KHZ_      125000
#define _62K500HZ_     62500
#define _32K768HZ_     32768
#define _31K25HZ_      31250
#define _31KHZ_        31000

/*  --------------------------------------------------------------------
    CONFIG words definition workaround
    2014-09-05 - RB - PIC18F
    2015-09-08 - RB - PIC16F
    ------------------------------------------------------------------*/

#if defined(_PIC14E) //__16F1459 || __16F1708

    // CONFIG1: CONFIGURATION REGISTER 1 (BYTE ADDRESS 8007h)    
    #if !defined(__CONFIG1)
    #define __CONFIG1 0x8007
    #endif

    // CONFIG2: CONFIGURATION REGISTER 2 (BYTE ADDRESS 8008h)    
    #if !defined(__CONFIG2)
    #define __CONFIG2 0x8008
    #endif

/*
    const u16 config1 @ 0x8007;
    const u16 config2 @ 0x8008;
*/

#else

    #ifdef __XC8__
    
    extern const u16 config1l @ 0x300000;
    extern const u16 config1h @ 0x300001;
    extern const u16 config2l @ 0x300002;

    #else
    /*
    __code u16 __at (__CONFIG1L) config1l;
    __code u16 __at (__CONFIG1H) config1h;
    __code u16 __at (__CONFIG2L) config2l ;
    */

    // CONFIG1L: CONFIGURATION REGISTER 1 LOW (BYTE ADDRESS 300000h)
    #if !defined(__CONFIG1L)
    #define __CONFIG1L 0x300000
    #endif

    // CONFIG1H: CONFIGURATION REGISTER 1 HIGH (BYTE ADDRESS 300001h)    
    #if !defined(__CONFIG1H)
    #define __CONFIG1H 0x300001
    #endif

    // CONFIG2L: CONFIGURATION REGISTER 2 LOW (BYTE ADDRESS 300002h)    
    #if !defined(__CONFIG2L)
    #define __CONFIG2L 0x300002
    #endif

    #endif // __XC8__
    
#endif

/*  --------------------------------------------------------------------
    Return Primary Oscillator Source 
    It can be internal or external w/o PLL and divider

    // 1459 and 1708
    // 111 = ECH: External clock, High-Power mode: on CLKIN pin
    // 110 = ECM: External clock, Medium-Power mode: on CLKIN pin
    // 101 = ECL: External clock, Low-Power mode: on CLKIN pin
    // 100 = INTOSC oscillator: I/O function on OSC1 pin
    // 011 = EXTRC oscillator: RC function connected to CLKIN pin
    // 010 = HS oscillator: High-speed crystal/resonator on OSC1 and OSC2 pins
    // 001 = XT oscillator: Crystal/resonator on OSC1 and OSC2 pins
    // 000 = LP oscillator: Low-power crystal on OSC1 and OSC2 pins

    // x5k50
    // 1101 = EC oscillator (low power, <4 MHz)
    // 1100 = EC oscillator, CLKO function on OSC2 (low power, <4 MHz)
    // 1011 = EC oscillator (medium power, 4 MHz - 16 MHz)
    // 1010 = EC oscillator, CLKO function on OSC2 (medium power, 4 MHz - 16 MHz)
    // 1001 = Internal oscillator block, CLKO function on OSC2
    // 1000 = Internal oscillator block
    // 0111 = External RC oscillator
    // 0110 = External RC oscillator, CLKO function on OSC2
    // 0101 = EC oscillator (high power, 16 MHz - 48 MHz)
    // 0100 = EC oscillator, CLKO function on OSC2 (high power, 16 MHz - 48 MHz)
    // 0011= HS oscillator (medium power, 4 MHz - 16 MHz)
    // 0010= HS oscillator (high power, 16 MHz - 25 MHz)
    // 0001= XT oscillator
    // 0000= LP oscillator

    // x550 and x455
7    // 111x = HS oscillator, PLL enabled (HSPLL)
6    // 110x = HS oscillator (HS)
5    // 1011 = Internal oscillator, HS oscillator used by USB (INTHS)
5    // 1010 = Internal oscillator, XT used by USB (INTXT)
4    // 1001 = Internal oscillator, CLKO function on RA6, EC used by USB (INTCKO)
4    // 1000 = Internal oscillator, port function on RA6, EC used by USB (INTIO)
3    // 0111 = EC oscillator, PLL enabled, CLKO function on RA6 (ECPLL)
3    // 0110 = EC oscillator, PLL enabled, port function on RA6 (ECPIO)
2    // 0101 = EC oscillator, CLKO function on RA6 (EC)
2    // 0100 = EC oscillator, port function on RA6 (ECIO)
1    // 001x = XT oscillator, PLL enabled (XTPLL)
0    // 000x = XT oscillator (XT)

    // xxJ5x
    // 111 = ECPLL oscillator with PLL software controlled, CLKO on RA6
    // 110 = EC oscillator with CLKO on RA6
    // 101 = HSPLL oscillator with PLL software controlled
    // 100 = HS oscillator
    // 011 = INTOSCPLLO, internal oscillator with PLL software controlled, CLKO on RA6, port function on RA7
    // 010 = INTOSCPLL, internal oscillator with PLL software controlled, port function on RA6 and RA7
    // 001 = INTOSCO internal oscillator block (INTRC/INTOSC) with CLKO on RA6, port function on RA7
    // 000 = INTOSC internal oscillator block (INTRC/INTOSC), port function on RA6 and RA7

    ------------------------------------------------------------------*/

u8 System_getSource()
{
    #if defined(__16F1459) || defined(__16F1708)

    return Flash_read(__CONFIG1) & 0b00000111;
    //return config1 & 0b00000111;

    #elif defined(__18f2455)  || defined(__18f4455)  || \
          defined(__18f2550)  || defined(__18f4550)  || \
          defined(__18f25k50) || defined(__18f45k50)

    #ifdef __XC8__
    return config1h & 0b00001111;
    #else
    return Flash_read(__CONFIG1H) & 0b00001111;
    #endif
    
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    #ifdef __XC8__
    return config2l & 0b00001111;
    #else
    return Flash_read(__CONFIG2L) & 0b00000111;
    #endif

    #else

        #error "This library doesn't support your processor."
        #error "Please contact a developper."

    #endif
}

/*  --------------------------------------------------------------------
    Return PLL value, 0 = no PLL 
    ------------------------------------------------------------------*/

u8 System_getPLL()
{
    #if defined(__16F1708)

    return 4*OSCCONbits.SPLLEN;

    #elif defined(__16F1459)

    return OSCCONbits.SPLLMULT ? 3*OSCCONbits.SPLLEN : 4*OSCCONbits.SPLLEN;

    #elif defined(__18f2455)  || defined(__18f4455)  || \
          defined(__18f2550)  || defined(__18f4550)

    return 1; // Need to know the source to know if PLL is enabled
    
    #elif defined(__18f25k50) || defined(__18f45k50)

    return OSCTUNEbits.SPLLMULT ? 3*OSCCON2bits.PLLEN : 4*OSCCON2bits.PLLEN;

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    return OSCTUNEbits.PLLEN;

    #else

        #error "This library doesn't support your processor."
        #error "Please contact a developper."

    #endif
}

/*  --------------------------------------------------------------------
    Return PLL value, 0 = no PLL 
    ------------------------------------------------------------------*/

u8 System_getPLLDIV()
{
    #if defined(__16F1708)  || defined(__16F1459)  || \
        defined(__18f25k50) || defined(__18f45k50)

    return 1; // No PLLDIV

    #elif defined(__18f2455)  || defined(__18f4455)  || \
          defined(__18f2550)  || defined(__18f4550)

    #ifdef __XC8__
    return 8 - (config1l & 0b00000111 );
    #else
    return 8 - (Flash_read(__CONFIG1L) & 0b00000111 );
    #endif
    
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

    #ifdef __XC8__
    return 8 - ((config1l & 0b00001110 ) >> 1);
    #else
    return 8 - ((Flash_read(__CONFIG1L) & 0b00001110 ) >> 1);
    #endif

    #else

        #error "This library doesn't support your processor."
        #error "Please contact a developper."

    #endif
}

/*  --------------------------------------------------------------------
    Return CPU divider 
    ------------------------------------------------------------------*/

static u8 System_getCPUDIV()
{
    /**---------------------------------------------------------------*/
    #if defined(__16F1708)
    /**---------------------------------------------------------------*/

    return 1; // No CPU Div

    /**---------------------------------------------------------------*/
    #elif defined(__16F1459)
    /**---------------------------------------------------------------*/

    return _cpudiv[(Flash_read(__CONFIG2) & 0x0030) >> 4];
    //return _cpudiv[(config2 & 0x0030) >> 4];

    /**---------------------------------------------------------------*/
    #elif defined(__18f2455)  || defined(__18f4455)  || \
          defined(__18f2550)  || defined(__18f4550)
    /**---------------------------------------------------------------*/

    u8 pll = System_getPLLDIV();

    #ifdef __XC8__
    if (pll)
        return _cpudiv[(config1l & 0b00011000 ) >> 3];
    else
        return _cpudiv_nopll[(config1l & 0b00011000 ) >> 3];
    #else
    if (pll)
        return _cpudiv[(Flash_read(__CONFIG1L) & 0b00011000 ) >> 3];
    else
        return _cpudiv_nopll[(Flash_read(__CONFIG1L) & 0b00011000 ) >> 3];
    #endif
    
    /**---------------------------------------------------------------*/
    #elif defined(__18f25k50) || defined(__18f45k50)
    /**---------------------------------------------------------------*/

    #ifdef __XC8__
    return _cpudiv[(config1h & 0b00011000) >> 3];
    #else
    return _cpudiv[(Flash_read(__CONFIG1H) & 0b00011000) >> 3];
    #endif
    
    /**---------------------------------------------------------------*/
    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)
    /**---------------------------------------------------------------*/

    #ifdef __XC8__
    return _cpudiv[config1h & 0b00000011];
    #else
    return _cpudiv[Flash_read(__CONFIG1H) & 0b00000011];
    #endif

    /**---------------------------------------------------------------*/
    #else

        #error "This library doesn't support your processor."
        #error "Please contact a developper."

    #endif
}

/*  --------------------------------------------------------------------
    Calculates the CPU frequency.

    - if PLL is enabled
        * CPU Freq. = 48MHz / divider
        * Incoming Freq. = 4 * PLLDIV (unused) (x3 or x4 on 25k50)
    - if PLL is disabled
        * if OSCCONbits.SCS == 0, Incoming Freq. = External Oscillator
                CPU Freq. = Incoming Freq. / divider
                Inc. Freq. is unknown, must be defined by user (for ex. #define CRYSTAL = 8000000L)
        * if OSCCONbits.SCS == 1, Incoming Freq. = Timer1
                CPU Freq. = Incoming Freq.
                how to get freq. from Timer 1 ?
        * if OSCCONbits.SCS >= 2, Incoming Freq. = Internal Oscillator
                CPU Freq. = Incoming Freq.
                Inc. Freq. = IRCF bits

    TODO : 18f14k22
    ------------------------------------------------------------------*/

u32 System_getCpuFrequency() 
{
    u8 source, pll, plldiv, cpudiv;

    switch (OSCCONbits.SCS)
    {
        case 0:
            // Clock is primary osc. determined by FOSC<2:0> or FOSC<3:0>

            pll = System_getPLL();
            plldiv = System_getPLLDIV();
            cpudiv = System_getCPUDIV();
            source = System_getSource();

            /**-------------------------------------------------------*/
            #if defined(__18f2455) || defined(__18f4455) || \
                defined(__18f2550) || defined(__18f4550)
            /**-------------------------------------------------------*/

            source = source >> 1;
            if( (source == 1) || (source == 3) || (source == 7) )
                _cpu_clock_ = 96000000UL / cpudiv; // pll is enabled
            else
                _cpu_clock_ = CRYSTAL / cpudiv;
            
            /**-------------------------------------------------------*/
            #elif defined(__16F1459)  || defined(__16F1708)
            /**-------------------------------------------------------*/

            if (source == 8)    // INTOSC
                _cpu_clock_ = pll * ircf[OSCCONbits.IRCF] / cpudiv;
            else
                _cpu_clock_ = pll * CRYSTAL / cpudiv;

            /**-------------------------------------------------------*/
            #elif defined(__18f25k50) || defined(__18f45k50) || \
                  defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
            /**-------------------------------------------------------*/

            if (pll)
                _cpu_clock_ = 48000000UL / cpudiv; // pll is enabled
            else
                _cpu_clock_ = CRYSTAL / cpudiv;
            
            /**-------------------------------------------------------*/
            #else

                #error "This library doesn't support your processor."
                #error "Please contact a developper."

            #endif
            break;

        case 1:
            // Clock is secondary osc. = Timer 1 oscillator (32768 Hz)
            _cpu_clock_ = 32768;
            break;

        case 2:
        case 3:
        default:
            // Clock is a postscaled internal clock (IRCF)
            // When an output frequency of 31 kHz is selected (IRCF = 0),
            // users may choose which internal oscillator acts as the
            // source. This is done with the Internal Oscillator
            // Low-Frequency Source Select bit : OSCTUNEbits.INTSRC
            // 1 = 31.25 kHz device clock derived from 8 MHz INTOSC source (divide-by-256 enabled)
            // 0 = 31 kHz device clock derived directly from INTRC internal oscillator

            if (OSCCONbits.IRCF)
            {
                _cpu_clock_ = ircf[OSCCONbits.IRCF];
            }
            #if !defined(_PIC14E)
            else
            {
                #if defined(__18f25k50) || defined(__18f45k50)
                if (OSCCON2bits.INTSRC)
                #else
                if (OSCTUNEbits.INTSRC)
                #endif
                    _cpu_clock_ = 31250;
                else
                    _cpu_clock_ = 31000;
            }
            #endif
            break;
    }
    return _cpu_clock_;
}

/*  --------------------------------------------------------------------
    Calculates the Peripheral frequency.
    On PIC18F, Peripheral Freq. = CPU. Freq. / 4
    09-09-2015 - RB - function replaced by a macro
    ------------------------------------------------------------------*/

#define System_getInstructionClock()    (System_getCpuFrequency() >> 2)
#define System_getPeripheralFrequency() (System_getCpuFrequency() >> 2)

/*  --------------------------------------------------------------------
    Functions to change clock source and frequency
    --------------------------------------------------------------------

    OSCCON: OSCILLATOR CONTROL REGISTER
    --------------------------------------------------------------------
    OSTS: Oscillator Start-up Time-out Status bit
    1 = Device is running from the clock defined by FOSC<3:0> of the CONFIG1H register
    0 = Device is running from the internal oscillator (HFINTOSC or INTRC)

    SCS<1:0>: System Clock Select bit
    1x = Internal oscillator block
    01 = Secondary (SOSC) oscillator
    00 = Primary clock (determined by FOSC<3:0> in CONFIG1H).

    OSCCON2: OSCILLATOR CONTROL REGISTER 2
    --------------------------------------------------------------------
    SOSCRUN: SOSC Run Status bit
    1 = System clock comes from secondary SOSC
    0 = System clock comes from an oscillator, other than SOSC

    SOSCDRV: SOSC Drive Control bit
    1 = T1OSC/SOSC oscillator drive circuit is selected by Configuration bits, CONFIG2L <4:3>
    0 = Low-power T1OSC/SOSC circuit is selected
 
    SOSCGO: Secondary Oscillator Start Control bit
    1 = Secondary oscillator is enabled.
    0 = Secondary oscillator is shut off if no other sources are requesting it.

    PRISD: Primary Oscillator Drive Circuit Shutdown bit
    1 = Oscillator drive circuit on
    0 = Oscillator drive circuit off (zero power)

    INTSRC: HFINTOSC Divided by 512 Enable bit
    1 = HFINTOSC used as the 31.25 kHz system clock reference – high accuracy
    0 = INTRC used as the 31.25 kHz system clock reference – low power.

    OSCTUNE: OSCILLATOR TUNING REGISTER (PICxxJxx only)
    --------------------------------------------------------------------
    INTSRC: Internal Oscillator Low-Frequency Source Select bit
    1 = 31.25 kHz device clock derived from 8 MHz INTOSC source (divide-by-256 enabled)
    0 = 31 kHz device clock derived directly from INTRC internal oscillator
    ------------------------------------------------------------------*/

#if defined(__16F1459)  || defined(__16F1708)  || \
    defined(__18f14k22) || \
    defined(__18f2455)  || defined(__18f4455)  || \
    defined(__18f2550)  || defined(__18f4550)  || \
    defined(__18f25k50) || defined(__18f45k50) || \
    defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

/*  --------------------------------------------------------------------
    Switch to System Clock determined by the Fosc settings
    in the Configuration Word.
    When the SCS bits = 00, this can be the Internal Oscillator,
    External Crystal/Resonator or External Clock.
    Pinguino's Configuration Word are set to External Crystal.
    ------------------------------------------------------------------*/

#if defined(SYSTEMSETEXTOSC) || defined(SYSTEMSETCPUFREQUENCY) || defined(SYSTEMSETPERIPHERALFREQUENCY)
void System_setExtOsc()
{
    OSCCONbits.SCS  = 0b00;
    // Wait while Oscillator Start-up Timer time-out is running
    // which means the primary oscillator is not yet ready
    //while (!OSCCONbits.OSTS);

    //#if defined(__18f25k50) || defined(__18f45k50)
    //OSCCON2bits.INTSRC = ;
    //#else
    //OSCTUNEbits.INTSRC = ;
    //#endif
}
#endif /* defined(SYSTEMSETEXTOSC) || defined(SYSTEMSETPERIPHERALFREQUENCY) */

/*  --------------------------------------------------------------------
    Switch on Timer1 External 32768Hz clock crystal
    With help from Mike Hall
    --------------------------------------------------------------------
    When the SCS bits = 01, the system clock is switched to the Secondary
    Oscillator which is an external 32768Hz crystal that controls
    the Timer1 Peripheral. The external clock crystal is an optional
    clock source that must be part of the Timer1 design circuit.
    ------------------------------------------------------------------*/

#if defined(SYSTEMSETTIMER1OSC) || defined(SYSTEMSETCPUFREQUENCY) || defined(SYSTEMSETPERIPHERALFREQUENCY)
void System_setTimer1Osc()
{
    TMR1H = 0x80;
    TMR1L = 0x00;

    #if defined(__16F1459)  || defined(__16F1708)  || \
        defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53) || \
        defined(__18f25k50) || defined(__18f45k50)
        
    T1GCONbits.TMR1GE = 0;      // Timer1 is not gated, set to 1 if timer1 doesnt work
    
    #endif
    
    // Set the timer1 to increment asynchronously to the internal
    // phase clocks. Running asynchronously allows the external clock
    // source to continue incrementing the timer during Sleep and can
    // generate an interrupt on overflow.

    T1CONbits.NOT_T1SYNC = 1;   // Make the external clock un-synchronized.   
    T1CONbits.TMR1CS = 0b10;    // Timer1 Source is the External Oscillator
    T1CONbits.T1OSCEN = 1;      // Enable Timer1 as a Source Clock
    T1CONbits.TMR1ON = 1;       // Starts Timer1

    #if defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        
    OSCCON2bits.SOSCDRV = 1;    // Osc. high drive level
    
    #endif
    
    #if defined(__16F1459)  || defined(__16F1708)  || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53) || \
        defined(__18f25k50) || defined(__18f45k50)
        
    OSCCON2bits.SOSCRUN = 0;    // System clock not from ext. Osc
    OSCCON2bits.SOSCGO = 1;     // turn on Osc.
    OSCCON2bits.PRISD = 1;      // turn on Osc.
    
    #endif
    
    OSCCONbits.SCS = 0b01;      // Switch System Clock to the Timer1
}
#endif /* defined(SYSTEMSETTIMER1OSC) || defined(SYSTEMSETPERIPHERALFREQUENCY) */

/*  --------------------------------------------------------------------
    Switch to Internal Oscillator (INTOSC)
    When the SCS bits = 0b10 or 0b11, then the system clock is switched
    to Internal Oscillator independent of the Fosc configuration bit
    settings. The IRCF bits of the OSCCON register will select the
    internal oscillator frequency.
    ------------------------------------------------------------------*/

#if defined(SYSTEMSETINTOSC) || defined(SYSTEMSETCPUFREQUENCY) || defined(SYSTEMSETPERIPHERALFREQUENCY)
void System_setIntOsc(u32 freq)
{
    u8 status = INTCONbits.GIE;
    u8 sel=0;
    u8 i;
    
    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        
    u16 pll_startup_counter = 600;

    #endif

    /// -----------------------------------------------------------------
    /// 1- Save status and Disable Interrupt  
    /// -----------------------------------------------------------------

    if (status)
        noInterrupts();

    /// -----------------------------------------------------------------
    /// 2- if freq > FINTOSC
    /// -----------------------------------------------------------------

    if (freq > FINTOSC)
    {
        #if defined(__18f14k22) || \
            defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f27j53) || defined(__18f47j53)
            
            OSCTUNEbits.PLLEN = 1;
            // Enable the PLL and wait 2+ms until the PLL locks
            while (pll_startup_counter--);
            //OSCCONbits.SCS  = 0b00; // Select PLL.

            if (freq = 32000000)
            {
                _cpu_clock_ = 32000000; 
                OSCCONbits.IRCF = 0b110;
            }
       
        #elif defined(__18f2455)  || defined(__18f4455)  || \
              defined(__18f2550)  || defined(__18f4550)  || \
              defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f27j53) || defined(__18f47j53)

            if (freq == 48000000)
            {
                _cpu_clock_ = 48000000;
                OSCCONbits.IRCF = 0b111;
            }
            
        #elif defined(__18f14k22)
        
            if (freq == 64000000)
            {
                _cpu_clock_ = 64000000;
                OSCCONbits.IRCF = 0b111;
            }

        #else // invalid freq

            _cpu_clock_ = FINTOSC;

        #endif
    }

    /// -----------------------------------------------------------------
    /// 3- if freq <= FINTOSC
    /// -----------------------------------------------------------------

    if (freq <= FINTOSC)
    {
        // *** LFINTOSC ***
        //INTSRC: Internal Oscillator Low-Frequency Source Select bit
        //0 = 31 kHz device clock derived directly from INTRC internal oscillator
        if (freq == 31000)
        {
            _cpu_clock_ = 31000;
            
            #if defined(__16F1459) || defined(__16F1708)
            
            //OSCCONbits.SPLLEN = 0;      // PLL disabled
            OSCCONbits.IRCF = 0;        // 31KHz
            OSCCONbits.SCS = 0b10;      // Internal Oscillator

            // The Low-Frequency Internal Oscillator Ready bit
            // (LFIOFR) of the OSCSTAT register indicates when the
            // LFINTOSC is running.
            while (!OSCSTATbits.LFIOFR);
            
            #elif defined(__18f25k50) || defined(__18f45k50)

            OSCCON2bits.PLLEN = 0;      // PLL disabled
            OSCCON2bits.INTSRC = 0;     // select INTOSC as a 31 KHz clock source
            OSCCONbits.IRCF = 0;        // 31KHz

            #elif defined(__18f2455) || defined(__18f4455) || \
                  defined(__18f2550) || defined(__18f4550)

            OSCTUNEbits.INTSRC = 0;     // select INTOSC as a 31 KHz clock source
            OSCCONbits.IRCF = 0;        // 31KHz

            #else

            OSCTUNEbits.PLLEN = 0;      // PLL disabled
            OSCTUNEbits.INTSRC = 0;     // select INTOSC as a 31 KHz clock source
            OSCCONbits.IRCF = 0;        // 31KHz

            #endif
        }

        // *** LFINTOSC ***
        //INTSRC: Internal Oscillator Low-Frequency Source Select bit
        //1 = 31.25 kHz device clock derived from INTOSC source
        else if (freq == 31250)
        {
            _cpu_clock_ = 31250;
            
            #if defined(__16F1459)  || defined(__16F1708)
            
            OSCCONbits.SPLLEN = 0;      // PLL disabled
            OSCCONbits.IRCF = 0b0010;   // 31.25 KHz

            #elif defined(__18f25k50) || defined(__18f45k50)

            OSCCON2bits.PLLEN = 0;      // PLL disabled
            OSCCON2bits.INTSRC = 1;     // select INTOSC as a 31.25 KHz clock source
            OSCCONbits.IRCF = 0;        // 

            #elif defined(__18f2455) || defined(__18f4455) || \
                  defined(__18f2550) || defined(__18f4550)

            OSCTUNEbits.INTSRC = 1;     // select INTOSC as a 31.25 KHz clock source
            OSCCONbits.IRCF = 0;        // 

            #else

            OSCTUNEbits.PLLEN = 0;      // PLL disabled
            OSCTUNEbits.INTSRC = 1;     // select INTOSC as a 31.25 KHz clock source
            OSCCONbits.IRCF = 0;        // 

            #endif
        }

        // *** HFINTOSC ***
        else
        {
            // calculate a valid freq (must be a multiple of FINTOSC)
            // and get the corresponding bits (IRCF) to select
            while ( ( (FINTOSC / freq) << sel++ ) < 0x80 );
            _cpu_clock_ = ircf[sel];

            OSCCONbits.IRCF = sel;
        }

    }
    
    /// -----------------------------------------------------------------
    /// 4- Switch to Internal Oscillator (INTOSC)  
    /// -----------------------------------------------------------------

    OSCCONbits.SCS  = 0b11;

    /// -----------------------------------------------------------------
    /// 5- wait INTOSC frequency is stable (HFIOFS=1)
    /// -----------------------------------------------------------------

    #if   defined(__16F1459) || defined(__16F1708)

    while (!OSCSTATbits.HFIOFS); 

    #elif defined(__18f25k50) || defined(__18f45k50)

    while (!OSCCONbits.HFIOFS); 

    #endif

    /// -----------------------------------------------------------------
    /// 6- update millis if used
    /// -----------------------------------------------------------------

    // RB : Can not work because this function call System_getPeripheralFrequency()
    //updateMillisReloadValue();

    #if defined(_MILLIS_C_)
    
        #if defined(__16F1459) || defined(__16F1708)
        
        PIE1bits.TMR1IE = 0;
        _period.w = 0xFFFF - (_cpu_clock_ / 4 / 1000) ;
        PIE1bits.TMR1IE = 1;

        #else

        INTCONbits.TMR0IE = 0;
        _period.w = 0xFFFF - (_cpu_clock_ / 4 / 1000) ;
        INTCONbits.TMR0IE = 1;

        #endif
    
    #endif
    
    /// -----------------------------------------------------------------
    /// 6- Get back to Interrupt status
    /// -----------------------------------------------------------------

    if (status)
        interrupts();
}
#endif /* defined(SYSTEMSETINTOSC) || defined(SYSTEMSETPERIPHERALFREQUENCY) */

/*  --------------------------------------------------------------------
    Set the CPU frequency
    --------------------------------------------------------------------
    20-09-2015 - RB - If the system clock is changed during an active EUSART
    receive operation, a receive error or data loss may result. To avoid this
    problem, check the status of the RCIDL bit to make sure that the receive
    operation is idle before changing the system clock.
    ------------------------------------------------------------------*/

#if defined(SYSTEMSETCPUFREQUENCY) 
void System_setCpuFrequency(u32 freq) 
{
    #if defined(__SERIAL__)
    while(!BAUDCONbits.RCIDL);  // Wait the receiver is Idle
    #endif
    
    if (freq == 32768)
        System_setTimer1Osc();
    else if (freq == 48000000)
        System_setExtOsc();
    else
        System_setIntOsc(freq);
}
#endif

/*  --------------------------------------------------------------------
    Set the Peripheral frequency (Fosc/4)
    --------------------------------------------------------------------
    20-09-2015 - RB - If the system clock is changed during an active EUSART
    receive operation, a receive error or data loss may result. To avoid this
    problem, check the status of the RCIDL bit to make sure that the receive
    operation is idle before changing the system clock.
    ------------------------------------------------------------------*/

#if defined(SYSTEMSETPERIPHERALFREQUENCY)
void System_setPeripheralFrequency(u32 freq)
{
    #if defined(__SERIAL__)
    while(!BAUDCONbits.RCIDL);  // Wait the receiver is Idle
    #endif

    if (freq > 12000000UL)
    {
        freq = 12000000UL;
    }

    if (OSCCONbits.SCS == 0b00)
    {
        // TODO
    }

    else if (OSCCONbits.SCS == 0b11)
    {
        System_setIntOsc(freq*4);
    }

    else
    {
        System_setExtOsc(freq*4);
    }
}
#endif /* defined(SYSTEMSETPERIPHERALFREQUENCY) */

#else

    #error "This library doesn't support your processor."
    #error "Please contact a developper."

#endif /* defined(__18f14k22) ... */

#endif /* __OSCILLATOR_C */

