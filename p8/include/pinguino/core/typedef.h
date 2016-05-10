/*	--------------------------------------------------------------------
    FILE:			typedef.h
    PROJECT:		Pinguino
    PURPOSE:		Pinguino types
    PROGRAMER:		Régis Blanchot
    --------------------------------------------------------------------
    CHANGELOG :
    --------------------------------------------------------------------
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

#ifndef __TYPEDEF_H
    #define __TYPEDEF_H

    #include <stdint.h>                     // ISO C99 7.18 Integer types

/*	----------------------------------------------------------------------------
    pinguino types
    --------------------------------------------------------------------------*/

    typedef signed char         s8;
    typedef signed int          s16;
    typedef signed long         s32;
    #ifdef __XC8__
    typedef signed long long    s64;        // SDCC doesn't support 64-bit type
    #endif
    
    typedef unsigned char       u8;
    typedef unsigned int        u16;
    typedef unsigned long       u32;
    #ifdef __XC8__
    typedef unsigned long long  u64;        // SDCC doesn't support 64-bit type
    #endif
    
    typedef union
    {
        u16 w;
        struct
        {
            u8 l8;
            u8 h8;
        };
    } t16;

    typedef union
    {
        u32 w;
        struct
        {
            u8 l;
            u8 h;
            u8 u;
        };
    } t24;

    typedef void (*funcout) (u8);   // type of void funcout(u8)


/*	----------------------------------------------------------------------------
    avr-gcc types
    --------------------------------------------------------------------------*/

    typedef unsigned char       byte;
    typedef unsigned char       BOOL;       // bool is not compatible with c++
    typedef unsigned char       boolean;

    typedef unsigned int        word;

    typedef unsigned long       dword;

/*	----------------------------------------------------------------------------
    other types
    --------------------------------------------------------------------------*/

    typedef unsigned char       BYTE;
    typedef unsigned int        WORD;
    typedef unsigned long       DWORD;

/*	----------------------------------------------------------------------------
    output types
    --------------------------------------------------------------------------*/
/*
    typedef enum
    {
        SERIAL  = 1,
        SERIAL1 = 1,
        SERIAL2,
        SERIAL3,
        SERIAL4,
        SERIAL5,
        SERIAL6,
        SPI,
        I2C,
        CDC,
        LCD,
        TFT
    } Output;
*/
#endif
