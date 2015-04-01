/*  --------------------------------------------------------------------
    FILE:               isrwrapper.c
    PROJECT:            pinguino 32
    PURPOSE:            weak definition of isr routines
    PROGRAMERS:         Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:      05 Feb. 2015
    LAST RELEASE:       05 Feb. 2015
    --------------------------------------------------------------------
    CHANGELOG:
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

#ifndef ISRWRAPPER_C
#define ISRWRAPPER_C

#if 0

    void __attribute__((weak)) Serial1Interrupt() { Nop(); }
    void __attribute__((weak)) Serial2Interrupt() {}
    #if defined(__32MX795F512L__) || \
        defined(__32MX795F512H__)
    void __attribute__((weak)) Serial3Interrupt() {}
    void __attribute__((weak)) Serial4Interrupt() {}
    void __attribute__((weak)) Serial5Interrupt() {}
    void __attribute__((weak)) Serial6Interrupt() {}
    #endif
    void __attribute__((weak)) Timer1Interrupt()  {}
    void __attribute__((weak)) Timer2Interrupt()  {}
    void __attribute__((weak)) Timer3Interrupt()  {}
    void __attribute__((weak)) Timer4Interrupt()  {}
    void __attribute__((weak)) Timer5Interrupt()  {}
    void __attribute__((weak)) SPI1Interrupt()    {}
    void __attribute__((weak)) SPI2Interrupt()    {}
    void __attribute__((weak)) RTCCInterrupt()    {}
    void __attribute__((weak)) USBInterrupt()     {}

#else

    #ifndef __SERIAL__
        void Serial1Interrupt(void) { Nop(); }
        void Serial2Interrupt(void) { Nop(); }

    #if defined(__32MX795F512L__) || \
        defined(__32MX795F512H__)

        #ifndef ENABLE_UART3
        void Serial3Interrupt(void) { Nop(); }
        #endif

        #ifndef ENABLE_UART4
        void Serial4Interrupt(void) { Nop(); }
        #endif

        #ifndef ENABLE_UART5
        void Serial5Interrupt(void) { Nop(); }
        #endif

        #ifndef ENABLE_UART6
        void Serial6Interrupt(void) { Nop(); }
        #endif

    #endif

    #endif // __SERIAL__


    #if !defined(TMR1INT) && !defined(__MILLIS__) && !defined(__DCF77__) // DCF77 TO MOVE
    void Timer1Interrupt(void) { Nop(); }
    #endif

    #if !defined(TMR2INT) && !defined(__SERVOS__) && !defined(__AUDIO__)
    void Timer2Interrupt(void) { Nop(); }
    #endif

    #if !defined(TMR3INT) && !defined(__IRREMOTE__) //&& !defined(__PWM__)
    void Timer3Interrupt(void) { Nop(); }
    #endif

    #if !defined(TMR4INT) && !defined(__STEPPER__)
    void Timer4Interrupt(void) { Nop(); }
    #endif

    #if !defined(TMR5INT) //&& !defined(__DCF77__) TODO
    void Timer5Interrupt(void) { Nop(); }
    #endif

    #ifndef __SPI__
        #if (SPIx != 1)
        void SPI1Interrupt(void) { Nop(); }
        #endif
        
        #if !defined(__32MX795F512L__) && \
            !defined(__32MX795F512H__)
            #if (SPIx != 2)
            void SPI2Interrupt(void) { Nop(); }
            #endif
        #endif
    #endif // __SPI__

    #ifndef __RTCC__
    void RTCCInterrupt(void) { Nop(); }
    #endif // __RTCC__

    #if !defined(__USBCDCINTERRUPT__) // !defined(__USBCDC__) || 
    void USBInterrupt(void) { Nop(); }
    #endif

#endif

#endif // ISRWRAPPER_C
