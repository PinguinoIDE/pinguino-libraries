/*	----------------------------------------------------------------------------
    macro.h
    Régis Blanchot
    --------------------------------------------------------------------------*/

#ifndef __MACRO_H
#define __MACRO_H

    /// ASM

    //Already defined
    //#define interrupts()	asm volatile("ei")
    //#define noInterrupts()	asm volatile("di")
    
    //Already defined ???
    #define nop()               asm volatile("nop")

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
    #define BitRead(b, n)       (((b) >> (n)) & 1 )
    #define BitSet(b, n)        ((b) |= (1 << (n)))
    #define BitClear(b, n)      ((b) &= ~(1 << (n)))	//(b &= !(1 << n))
    #define BitTest(b, n)       (((b) & (1 << (n)))!=0)
    #define BitWrite(b, n, v)   (v ? bitSet(b, n) : bitClear(b, n))

    /// MATH

    #define min(a,b) ((a)<(b)?(a):(b))
    #define max(a,b) ((a)>(b)?(a):(b))
    //RB 03-10-2014 : moved to math.c to avoid conflict with stdlib.h
    //#define abs(x) ((x)>0?(x):-(x))
    #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
    #define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
    #define radians(deg) ((deg)*DEG_TO_RAD)
    #define degrees(rad) ((rad)*RAD_TO_DEG)
    #define sq(x) ((x)*(x))
    #define swap(i, j) {int t = i; i = j; j = t;}

#endif	/* __MACRO_H */

