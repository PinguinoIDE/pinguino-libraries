/*  --------------------------------------------------------------------
    FILE:           main.c
    PROJECT:        pinguino
    PURPOSE:        application main function
    PROGRAMER:      Jean-pierre Mandon - Régis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  19 Sep 2008
    LAST RELEASE:   06 Oct 2015
    --------------------------------------------------------------------
    CHANGELOG :
    Originally based on a file by (c)2006 Pierre Gaufillet <pierre.gaufillet@magic.fr>
    19 Sep. 2008 - Jean-pierre Mandon - adapted to Pinguino  
    21 Apr. 2012 - Régis Blanchot - added bootloader v4.x support
    20 Jun. 2012 - Régis Blanchot - added io.c support (remapping)
    05 Feb. 2013 - Régis Blanchot - added interrupt init
    11 Feb. 2013 - Régis Blanchot - removed call to crt0iPinguino.c
                                    added reset_isr() instead
    28 Feb. 2013 - Régis Blanchot - added stack pointer initialization
    09 Jul. 2015 - Régis Blanchot - replaced #include <pic18fregs.h> by #include <compiler.h>
                                    to enable compatibility between SDCC and XC8
    09 Sep. 2015 - Régis Blanchot - added PIC16F support
    06 Oct. 2015 - Régis Blanchot - added watchdog support
    12 Dec. 2015 - Régis Blanchot - added __DELAYMS__ flag
    27 Jan. 2016 - Régis Blanchot - added PIC16F1708 support
    12 Apr. 2016 - Régis Blanchot - removed __DELAYMS__ flag
    13 Oct. 2016 - Régis Blanchot - added PIC1xK50 support
    14 Oct. 2016 - Régis Blanchot - updated _cpu_clock_ value when ICSP is used
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
    ------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////
#include "define.h"
////////////////////////////////////////////////////////////////////////

//#ifdef CRYSTAL
//unsigned long _cpu_clock_ = CRYSTAL;
//#else
unsigned long _cpu_clock_ = 48000000;
//#endif

#include <compiler.h>       // SDCC / XC8 compatibility
#include <typedef.h>        // u8, u16, u32 and other types definition
#include <const.h>          // Pinguino main constants
#include <macro.h>          // Pinguino main macros
#include <pin.h>            // Pins definition, needs define.h to be included first
#include <io.c>             // needs define.h to be included first

////////////////////////////////////////////////////////////////////////
#include "user.c"           // user's .pde file translated to C
////////////////////////////////////////////////////////////////////////

#if defined(_PIC14E) && !defined(__XC8__)
    #error "********************************"
    #error "* PIC16F must use XC8 compiler *"
    #error "* Please change.               *"
    #error "********************************"
#endif

#if defined (noboot)
    #ifndef CRYSTAL
    #error "***********************************"
    #error "* Crystal frequency expected !    *"
    #error "* Ex: #define CRYSTAL 20000000    *"
    #error "***********************************"
    #endif
    #define SPEED   1
    // runtime start code with variable initialisation
    //#include "crt0.c"
    //#include "crt0i.c"
    #include "config.h"

    // Application entry point called from bootloader v4.x
    void main(void)

#elif defined(boot2)

    #if defined(__XC8__) || defined(_PIC14E)
        #error "********************************"
        #error "* Bootloader not compatible    *"
        #error "* with neither XC8 nor PIC16F  *"
        #error "* Please change.               *"
        #error "********************************"
    #endif

    #if !defined(__18f2455) && !defined(__18f4455) && \
        !defined(__18f2550) && !defined(__18f4550)
        #error "********************************"
        #error "* Bootloader not compatible    *"
        #error "* with your processor          *"
        #error "* Please change.               *"
        #error "********************************"
    #endif
    
    #warning   "********************************"
    #warning   "* Bootloader out of date       *"
    #warning   "* Please update.               *"
    #warning   "********************************"

    // 2013-07-31 - A. Gentric - fix usb.c
    //#include <common_types.h>
    //#include <boot_iface.h>
    
    // only for compatibility with application_iface.o
    #ifndef __USB__
        void epap_in()      { return; }
        void epap_out()     { return; }
        void epapin_init()  { return; }
        void epapout_init() { return; }
    #endif

    // Application entry point called from bootloader v2.12
    void pinguino_main(void)

#elif defined(boot4)

    #if defined(__16F1708)
        #error "********************************"
        #error "* No USB Module for this chip   *"
        #error "* Please change for ICSP mode. *"
        #error "********************************"
    #endif

    #if !defined(__XC8__)
        // runtime start code
        //#include "crt0.c"     // minimal  init.
        #include "crt0i.c"    // variables init. (CDC compatible)
        //#include "crt0iz.c"     // variables init. + clear
    #endif

    // Application entry point called from bootloader v4.x
    void main(void)

#endif

/// ----------------------------------------------------------------
/// Main
/// ----------------------------------------------------------------

{
    #if defined(__18f13k50) || defined(__18f14k50) || \
        defined(__18f25k50) || defined(__18f45k50) || \
        defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)

        u16 pll_startup_counter = 600;

    #endif

    /// ----------------------------------------------------------------
    /// If we start from a Power-on reset, clear reset bits
    /// ----------------------------------------------------------------

/**********************************************************************/
    #if defined(_PIC14E) //__16F1459
/**********************************************************************/

    if (PCONbits.nPOR == 0)
    {
        PCONbits.nPOR = 1;          // POR and BOR flags must be cleared by
        PCONbits.nBOR = 1;          // software to allow a new detection
    }

/**********************************************************************/
    #else
/**********************************************************************/

    RCONbits.IPEN = 1;              // Enables priority levels on
                                    // interrupts (cf. vectors.c/.h)
                                    // MUST BE SET OR INTERRUPT WON'T WORK !
                                    // NB: MCLR clears this bit

    if (RCONbits.NOT_POR == 0)
    {
        RCONbits.NOT_POR = 1;       // POR and BOR flags must be cleared by
        RCONbits.NOT_BOR = 1;       // software to allow a new detection
    }

/**********************************************************************/
    #endif
/**********************************************************************/

    /// ----------------------------------------------------------------
    /// Disables all interrupt
    /// ----------------------------------------------------------------

    /* RB : useless, interrupts are disabled per default after reset
    #if defined(__16F1459)
    INTCONbits.GIE  = 0;            // Disable global interrupt
    #else
    INTCONbits.GIEH = 0;            // Disables all HP interrupts
    INTCONbits.GIEL = 0;            // Disables all LP interrupts
    #endif
    */
    
    /// ----------------------------------------------------------------
    /// Perform a loop for some processors until their frequency is stable
    /// ----------------------------------------------------------------

/**********************************************************************/
    #if defined(__16F1708)
/**********************************************************************/

        // Whatever the configuration we start with INTOSC
        OSCCON = 0b11111010;        // SPLLEN   : 1 = 4x PLL is enabled (see config.h)
                                    // IRCF     : 1111 = HFINTOSC (16 MHz)
                                    // bit 2    : unimplemented
                                    // SCS      : 1x = use clock determined by IRCF

        // Wait HFINTOSC frequency is stable (HFIOFS=1) 
        while (!OSCSTATbits.HFIOFS);

        // Wait until the PLLRDY bit is set in the OSCSTAT register
        // before attempting to set the USBEN bit.
        while (!OSCSTATbits.PLLR);

/**********************************************************************/
    #elif defined(__16F1459)
/**********************************************************************/

        // Whatever the configuration we start with INTOSC
        OSCCON = 0b11111100;        // SPLLEN   : 1 = PLL is enabled (see config.h)
                                    // SPLLMULT : 1 = 3x PLL is enabled (16x3=48MHz)
                                    // IRCF     : 1111 = HFINTOSC (16 MHz)
                                    // SCS      : 00 = use clock determined by IRCF

        #if defined(__USB__) || defined(__USBCDC__) || defined(__USBBULK__)
        ACTCON = 0x90;              // Enable active clock tuning with USB
        #endif
        // Wait HFINTOSC frequency is stable (HFIOFS=1) 
        while (!OSCSTATbits.HFIOFS);

        // Wait until the PLLRDY bit is set in the OSCSTAT register
        // before attempting to set the USBEN bit.
        while (!OSCSTATbits.PLLRDY);

/**********************************************************************/
    #elif defined(__18f13k50) || defined(__18f14k50)
/**********************************************************************/

        OSCCONbits.SCS  = 0;        // 00 = Primary clock determined by CONFIG1H[FOSC<3:0>]
        //OSCCON2bits.PRI_SD = 1;     // 1 = Oscillator drive circuit on

        #if (CRYSTAL == 48)

        OSCTUNEbits.SPLLEN = 0;     // SPLLEN   : 0 = 4x PLL is disabled (see config.h)

        #else                       // 12 MHZ (4x12=48Mhz)

        OSCTUNEbits.SPLLEN = 1;     // SPLLEN   : 1 = 4x PLL is enabled (see config.h)
        while (pll_startup_counter--);

        #endif

/**********************************************************************/
    #elif defined(__18f2455) || defined(__18f4455) || \
          defined(__18f2550) || defined(__18f4550)
/**********************************************************************/

        // If Internal Oscillator is used
        if (OSCCONbits.SCS > 0x01)
            // wait INTOSC frequency is stable (IOFS=1) 
            while (!OSCCONbits.IOFS);

        // PLL is enabled by Config. Bits

/**********************************************************************/
    #elif defined(__18f25k50) || defined(__18f45k50)
/**********************************************************************/
    
        // If Internal Oscillator is used
        if (OSCCONbits.SCS > 0x01)
            // wait HFINTOSC frequency is stable (HFIOFS=1) 
            while (!OSCCONbits.HFIOFS);

        // Enable the PLL and wait 2+ms until the PLL locks
        OSCCON2bits.PLLEN = 1;
        OSCTUNEbits.SPLLMULT = 1;   // 1=3xPLL, 0=4xPLL
        while (pll_startup_counter--);

/**********************************************************************/
    #elif defined(__18f26j50) || defined(__18f46j50)
/**********************************************************************/
    
        // If Internal Oscillator is used
        // if (OSCCONbits.SCS > 0x02)
        // Seems there is no time to wait
        
        // Enable the PLL and wait 2+ms until the PLL locks
        OSCTUNEbits.PLLEN = 1;
        while (pll_startup_counter--);

/**********************************************************************/
    #elif defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)
/**********************************************************************/

        // If Internal Oscillator is used
        if (OSCCONbits.SCS > 0x02)
            // wait INTOSC frequency is stable (FLTS=1) 
            while(!OSCCONbits.FLTS);

        // Enable the PLL and wait 2+ms until the PLL locks
        OSCTUNEbits.PLLEN = 1;
        while (pll_startup_counter--);

    #endif

    /// ----------------------------------------------------------------
    /// I/O init 
    /// ----------------------------------------------------------------

    IO_init();
    IO_digital();
    
    #if defined(__16F1459)  || defined(__16F1708)  || \
        defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)

    IO_remap();

    #endif

    /// ----------------------------------------------------------------
    /// Various Init.
    /// ----------------------------------------------------------------

    #if defined(__MILLIS__) //|| defined(__DELAYMS__)
    millis_init();              // Use Timer 0 (16F use Timer 1)
    #endif

    #if defined(__PULSE__)
    pulse_init();              // Use Timer 1
    #endif

    #ifdef __USB__
    usb_init();
    #endif

    /* NB : will be up to users soon */
    #ifdef __USBCDC__
    //CDCbegin(9600);
    CDCbegin(115200);
    #endif    

    #ifdef __USBBULK__
    bulk_init();
    #endif

    #if defined(ANALOGREFERENCE) || defined(ANALOGREAD)
    analog_init();
    #endif

    #ifdef ANALOGWRITE
    analogwrite_init();
    #endif

    #ifdef __SPI__
    spi_init();
    #endif

    #ifdef __SERVO__        // Use Timer 1
    servo_init();
    #endif

    #ifdef __PS2KEYB__
    keyboard_init();
    #endif

    #ifdef __WATCHDOG__
    watchdog_init();
    #endif

////////////////////////////////////////////////////////////////////////
    setup();
////////////////////////////////////////////////////////////////////////

    #if defined(TMR0INT) || defined(TMR1INT) || \
        defined(TMR2INT) || defined(TMR3INT) || \
        defined(TMR4INT) || defined(TMR5INT) || \
        defined(TMR6INT) || defined(TMR8INT) 

    IntTimerStart();        // Enable all defined timers interrupts
                            // at the same time
    #endif

    #ifdef ON_EVENT         // defined if interrupt.c is used
    interrupts();           // starts interrupts
    #endif

    while (1)
    {
////////////////////////////////////////////////////////////////////////
        loop();
////////////////////////////////////////////////////////////////////////
    }

    // Returning from main will lock up.
    #if defined(__XC8__)
    #asm
    lockup:
    bra lockup
    #endasm
    #endif
}

/// ----------------------------------------------------------------
/// Interrupt 
/// ----------------------------------------------------------------

#if  defined(__USBCDC__)    || defined(__USBBULK__) || defined(__USB__)     || \
     defined(USERINT)       || defined(INT0INT)     || defined(I2CINT)      || \
     defined(__SERIAL__)    || defined(ON_EVENT)    || defined(__MILLIS__)  || \
     defined(__SERVO__)     || defined(__PS2KEYB__) || defined(__DCF77__)   || \
     defined(__IRREMOTE__)  || defined(__AUDIO__)   || defined(__STEPPER__) || \
     defined(__CTMU__)      || defined(__SWPWM__)   || defined(RTCCALARMINTENABLE)
     // || defined(__DELAYMS__)
     // || defined(__MICROSTEPPING__)

    #if defined(_PIC14E)

        /*  ------------------------------------------------------------
            Interrupt Vector
            ----------------------------------------------------------*/

        void interrupt PIC16F_isr(void)
        {
            #ifdef __USBCDC__
            CDC_interrupt();
            #endif
            
            #if defined(__USBBULK__)
            bulk_interrupt();
            #endif

            #ifdef __USB__
            usb_interrupt();
            #endif

            #ifdef __SERIAL__
            serial_interrupt();
            #endif

            #if defined(__MILLIS__) //|| defined(__DELAYMS__)
            millis_interrupt();
            #endif

            #ifdef I2CINT
            I2C_interrupt();
            #endif

            #ifdef __SERVO__
            servo_interrupt();
            #endif

            #ifdef INT0INT
            userhighinterrupt();
            #endif

            #ifdef __PS2KEYB__
            keyboard_interrupt();
            #endif

            #ifdef __DCF77__
            dcf77_interrupt();
            #endif

            #ifdef __IRREMOTE__
            irremote_interrupt();
            #endif

            //#ifdef __MICROSTEPPING__
            #ifdef __STEPPER__
            stepper_interrupt();
            #endif

            #ifdef RTCCALARMINTENABLE
            rtcc_interrupt();
            #endif

            #ifdef __AUDIO__
            pwm_interrupt();
            #endif

            #ifdef __SWPWM__
            swpwm_interrupt();
            #endif
            
            #ifdef __CTMU__
            //ctmu_interrupt();
            #endif
            
            #ifdef ON_EVENT
            userlowinterrupt();
            #endif

            #ifdef USERINT
            userinterrupt();
            #endif
        }

    #else // PIC18F

        /*  ------------------------------------------------------------
            High Interrupt Vector
            ----------------------------------------------------------*/

        #ifdef boot2
        #pragma code high_priority_isr 0x2020
        #endif

        // boot4 : ENTRY + 0x08
        // noboot: 0x08
        #ifdef __XC8__
        void interrupt high_priority high_priority_isr(void)
        #else
        void high_priority_isr(void) __interrupt 1
        #endif
        {
            #ifndef __XC8__
            __asm
                MOVFF   _TBLPTRL, POSTDEC1
                MOVFF   _TBLPTRH, POSTDEC1
                MOVFF   _TBLPTRU, POSTDEC1
                MOVFF   _TABLAT,  POSTDEC1
            __endasm;
            #endif

            #ifdef __USBCDC__
            CDC_interrupt();
            #endif
            
            #ifdef __USBBULK__
            bulk_interrupt();
            #endif

            #ifdef __USB__
            usb_interrupt();
            #endif

            #ifdef __SERIAL__
            serial_interrupt();
            #endif

            #if defined(__MILLIS__) //|| defined(__DELAYMS__)
            millis_interrupt();
            #endif

            #ifdef I2CINT
            I2C_interrupt();
            #endif

            #ifdef __SERVO__
            servo_interrupt();
            #endif

            #ifdef INT0INT
            userhighinterrupt();
            #endif

            #ifdef __PS2KEYB__
            keyboard_interrupt();
            #endif

            //#ifdef __KEYPAD__
            //keypad_interrupt();
            //#endif

            #ifdef __DCF77__
            dcf77_interrupt();
            #endif

            #ifdef __IRREMOTE__
            irremote_interrupt();
            #endif
            
            //#ifdef __MICROSTEPPING__
            #ifdef __STEPPER__
            stepper_interrupt();
            #endif

            #ifdef RTCCALARMINTENABLE
            rtcc_interrupt();
            #endif

            #ifdef __AUDIO__
            pwm_interrupt();
            #endif

            #ifdef __SWPWM__
            swpwm_interrupt();
            #endif

            #ifdef __CTMU__
            //ctmu_interrupt();
            #endif
            
            #ifndef __XC8__
            __asm
                MOVFF   PREINC1, _TABLAT
                MOVFF   PREINC1, _TBLPTRU
                MOVFF   PREINC1, _TBLPTRH
                MOVFF   PREINC1, _TBLPTRL
            __endasm;
            #endif
        }

        /*  ------------------------------------------------------------
            Low Interrupt Vector
            ----------------------------------------------------------*/

        #ifdef boot2
        #pragma code low_priority_isr 0x4000
        #endif

        // boot4 : ENTRY + 0x18
        // noboot: 0x18
        #ifdef __XC8__
        void interrupt low_priority low_priority_isr(void)
        #else
        void low_priority_isr(void) __interrupt 2
        #endif
        {
            #ifndef __XC8__
            __asm
                MOVFF   _TBLPTRL, POSTDEC1
                MOVFF   _TBLPTRH, POSTDEC1
                MOVFF   _TBLPTRU, POSTDEC1
                MOVFF   _TABLAT,  POSTDEC1
            __endasm;
            #endif
            
            #ifdef USERINT
            userinterrupt();
            #endif

            #ifdef ON_EVENT
            userlowinterrupt();
            #endif
            
            #ifndef __XC8__
            __asm
                MOVFF   PREINC1, _TABLAT
                MOVFF   PREINC1, _TBLPTRU
                MOVFF   PREINC1, _TBLPTRH
                MOVFF   PREINC1, _TBLPTRL
            __endasm;
            #endif
        }

    #endif /* PIC18F */
    
#endif /* all interrupt */

/*  --------------------------------------------------------------------
    Reset Interrupt Vector
    ------------------------------------------------------------------*/
/*
#if defined (noboot) || defined(boot4)

// boot4 : ENTRY + 0x00
// noboot: 0x00
void reset_isr(void) __naked __interrupt 0
{
    // Call the Pinguino main routine.
    main();
}

#endif
*/
