// millis library for pinguino
// Jean-Pierre MANDON 2009
// added interrupt.c functions (regis blanchot 2011)
// [14-05-12][jp.mandon changed long to u32 and Millis to millis / thanks mark harper]
// [31-01-13][r.blanchot use of System_getPeripheralFrequency()]

#ifndef _MILLIS_C_
#define _MILLIS_C_

#include <compiler.h>
#include <typedef.h>
//#include <interrupt.h>
//#include <interrupt.c>
#if !defined(__16F1459)
#include <oscillator.c>         // System_getPeripheralFrequency()
#endif

volatile u32 _millis;
#if defined(__16F1459)
volatile u8 _PR0_;
#else
volatile u16 _PR0_;
#endif

void updateMillisReloadValue(void )   /* Call from System_setIntOsc() */
{
    /* Atomic operation */
    INTCONbits.TMR0IE = 0;      //INT_DISABLE;
    #if defined(__16F1459)
    //_PR0_ = (0xFFFF - System_getPeripheralFrequency() / 1000) >> 8;
    _PR0_ = 69;
    #else
    _PR0_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    #endif
    INTCONbits.TMR0IE = 1;      //INT_ENABLE;
}

void millis_init(void)
{
//    intUsed[INT_TMR0] = INT_USED;

    /*
    T0CON = 0x80;               // TMR0 on, 16 bits counter, prescaler=2
    INTCON |= 0xA0;             // set GIE and TMR0IE
    */
    
    // if Fosc = 48 MHz then Fosc/4 = 12MHz
    // which means 12.E-06 cycles/sec = 12.000 cycles/ms
    // if TMR0 is loaded with 65536 - 12000
    // overload will occur after 12.000 cycles = 1ms
    
    #if defined(__16F1459)
    INTCONbits.GIE     = 0;     // Disable global interrupts
    #else
    INTCONbits.GIEH    = 0;     // Disable global HP interrupts
    INTCONbits.GIEL    = 0;     // Disable global LP interrupts
    #endif
    
    #if defined(__16F1459)
    OPTION_REG = 0b00000111;    // Clock source FOSC/4, prescaler 1:256
    //_PR0_ = (0xFFFF - System_getPeripheralFrequency() / 1000) >> 8;
    _PR0_ = 69;
    TMR0 = _PR0_;
    #else
    T0CON = 0b00001000;         //T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
    _PR0_ = 0xFFFF - (System_getPeripheralFrequency() / 1000);
    TMR0H = high8(_PR0_);
    TMR0L =  low8(_PR0_);
    #endif
    
    #if !defined(__16F1459)
    INTCON2bits.TMR0IP = 1;     //INT_HIGH_PRIORITY;
    #endif
    INTCONbits.TMR0IF  = 0;
    INTCONbits.TMR0IE  = 1;     //INT_ENABLE;

    #if defined(__16F1459)
    INTCONbits.GIE     = 1;     // Enable global interrupts
    #else
    T0CONbits.TMR0ON   = 1;
    INTCONbits.GIEH    = 1;     // Enable global HP interrupts
    INTCONbits.GIEL    = 1;     // Enable global LP interrupts
    #endif

    _millis = 0;
}

u32 millis()
{
    u32 temp;
    /* Atomic operation for multibyte value */
    INTCONbits.TMR0IE = 0;      //INT_DISABLE;
    temp = _millis;
    INTCONbits.TMR0IE = 1;      //INT_ENABLE;
    return (temp);
}

// called by interruption service routine in main.c    if (INTCONbits.TMR0IF)
void millis_interrupt(void)
{
    if (INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        #if defined(__16F1459)
        TMR0 = _PR0_;
        #else
        TMR0H = high8(_PR0_);
        TMR0L =  low8(_PR0_);
        #endif
        _millis++;
    }
}

#endif /* _MILLIS_C_ */
