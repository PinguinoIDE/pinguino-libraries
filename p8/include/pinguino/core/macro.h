/*	--------------------------------------------------------------------
    ---------- macro.h
    ---------- rblanchot@gmail.com
    ---------- Part of Pinguino - http://www.pinguino.cc/
    -----------------------------------------------------------------**/

#ifndef __MACRO_H
    #define __MACRO_H

    #include <compiler.h>           // PIC Registers definitions
    #include <typedef.h>            // u8, u16, u32, ...
    #include <const.h>              // DEG_TO_RAD, RAD_TO_DEG, ...

    extern unsigned long _cpu_clock_;

    /// ASM

    #define nop()                   do { __asm__("NOP");    } while (0)
    #define clrwdt()                do { __asm__("CLRWDT"); } while (0)
    #define reset()                 do { __asm__("RESET");  } while (0)
    #define sleep()                 do { __asm__("SLEEP");  } while (0)
    
    #if defined(__16F1459)
    #define interrupts()            do { INTCONbits.GIE = 1; INTCONbits.PEIE = 1; } while (0)
    #define noInterrupts()          do { INTCONbits.GIE = 0; INTCONbits.PEIE = 0; } while (0)
    #define isInterrupts()          (INTCONbits.GIE + INTCONbits.PEIE)
    #else
    #define interrupts()            do { INTCONbits.GIEH = 1; INTCONbits.GIEL = 1; } while (0)
    #define noInterrupts()          do { INTCONbits.GIEH = 0; INTCONbits.GIEL = 0; } while (0)
    #define isInterrupts()          (INTCONbits.GIEH + INTCONbits.GIEL)
    #endif
    #define ATOMIC                  for(INTCONbits.GIE=0;!INTCONbits.GIE;INTCONbits.GIE=1)
    
    /// C

    #define noEndLoop()             while(1)

    /// BYTES

    #define highByte(x)             ((u8) ((x) >> 8))
    #define high8(x)                ((u8) ((x) >> 8))
    #define lowByte(x)              ((u8) ((x) & 0xFF))
    #define low8(x)                 ((u8) ((x) & 0xFF))
    #define make16(low,high)        ((u16)low | (u16)(high << 8))
    #define make32(low, high)       ((u32)low | (u32)(high << 16))
    #define msb(x)                  (x & 0xF0)
    #define lsb(x)                  (x & 0x0F)

    /// MATH

    #if !defined(__XC8__) && !defined(_STDLIB_H_)
        //#define min(a,b)            ((a)<(b)?(a):(b))
        #define min(x, y)           (y ^ ((x ^ y) & -(x < y)))
        //#define max(a,b)            ((a)>(b)?(a):(b))
        #define max(x, y)           (x ^ ((x ^ y) & -(x < y)))
    #endif
    
    //already defined in stdlib.h / mathlib.c
    //#define abs(x)                    ((x)>0?(x):-(x))
    #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
    #define round(x)                ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
    #define radians(deg)            ((deg)*DEG_TO_RAD)
    #define degrees(rad)            ((rad)*RAD_TO_DEG)
    #define sq(x)                   ((x)*(x))
    #define swap(a, b)              ((&(a) == &(b)) || (((a) -= (b)), ((b) += (a)), ((a) = (b) - (a))))

    /// CLOCK

    #define UsToTcy(x)              ((u16)((x)*(_cpu_clock_/4000000)))
    #define TcyToUs(x)              ((u16)((x)*4000000/_cpu_clock_))
    #define MsToTcy(x)              ((u16)((x)*(_cpu_clock_/4000)))
    //#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
    //#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
    //#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

    /// BITWISE OPERATION

    /* Outdated, SDCC supports binary numbers
    #define BIN_BIT(value, bit, dec) \
        (((((u32)(value##.0))/dec)&1 == 1)? (1<<bit) : 0)

    #define Bin(value) \
    (	BIN_BIT(value,  0, 1) | \
        BIN_BIT(value,  1, 10) | \
        BIN_BIT(value,  2, 100) | \
        BIN_BIT(value,  3, 1000) | \
        BIN_BIT(value,  4, 10000) | \
        BIN_BIT(value,  5, 100000) | \
        BIN_BIT(value,  6, 1000000) | \
        BIN_BIT(value,  7, 10000000))
    */
    
    #define Bit(n)                  (1 << (n))
    #define BitTest(b, n)           (((b) & (1 << (n)))!=0)
    #define BitRead(val, n)         (((val) >> (n)) & 1)
    #define BitSet(val, n)          ((val) |= (1 << (n)))
    #define BitClear(val, n)        ((val) &= ~(1 << (n)))
    #define BitInv(val, n)          ((val) ^= (1 << (n)))
    #define BitWrite(val, n, b)     ((b) ? BitSet(val, n) : BitClear(val, n))
    #define Not(n)                  (255 - (n))

#endif
