/*	----------------------------------------------------------------------------
    macro.h
    Régis Blanchot
    
    TODO :
    * move Math macros in math.c
    --------------------------------------------------------------------------*/

#ifndef __MACRO_H
#define __MACRO_H

    // Tell the compiler the next function must be MIPS32.
    // Typically needed by all functions such as interrupt handlers
    // which cannot be MIPS16 functions when -mips16 option is enabled.
    // Usage : void MIPS32 myfunction(...)
    #define MIPS32          __attribute__((noinline,nomips16))

    // Make next statement or block Atomic
    #define ATOMIC //u32 status; for(asm volatile("di %0" : "r="(status)); !status; asm volatile("ei %0" : "r="(status)))

    /// ASM
    #include <mips.h>
    #define interrupts()        EnableInterrupt()
    #define noInterrupts()      DisableInterrupt()
    #define isInterrupts()      (true)
    //Already defined ???
    #define nop()               asm volatile("nop")

    /// C
    #define noEndLoop()             while(1)

    /// BYTES

    #define low8(x)             ((unsigned char) ((x) & 0xFF))
    #define high8(x)            ((unsigned char) ((x) >> 8))
    // already defined in pinguino.pdl32
    //#define lowByte(x)        ((unsigned char) ((x) & 0xFF))
    //#define highByte(x)       ((unsigned char) ((x) >> 8))
    #define make16(low, high)   (low | (high << 8))
    #define make32(low, high)   (low | (high << 16))

    /// BITWISE OPERATION

    #define Bit(b)              (1 << (b))
    #define BitTest(b, n)       (((b) & (1 << (n)))!=0)
    #define BitRead(b, n)       (((b) >> (n)) & 1 )
    #define BitSet(b, n)        ((b) |= (1 << (n)))
    #define BitClear(b, n)      ((b) &= ~(1 << (n)))	//(b &= !(1 << n))
    #define BitInv(v, n)        ((v) ^= (1 << (n)))
    #define BitWrite(b, n, v)   (v ? bitSet(b, n) : bitClear(b, n))
    #define Not(n)              (255 - (n))

    /// MATH

    #define min(a,b) ((a)<(b)?(a):(b))
    #define max(a,b) ((a)>(b)?(a):(b))
    //RB 03-10-2014 : moved to math.c to avoid conflict with stdlib.h
    //#define abs(x) ((x)>0?(x):-(x))
    #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
    #define round(x)        ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
    #define radians(deg)    ((deg)*DEG_TO_RAD)
    #define degrees(rad)    ((rad)*RAD_TO_DEG)
    #define sq(x)           ((x)*(x))
    #define swap(i, j)      {int t = i; i = j; j = t;}

#endif	/* __MACRO_H */
