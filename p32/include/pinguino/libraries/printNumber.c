/*	--------------------------------------------------------------------
    FILE:			printNumber.c
    PROJECT:		pinguino - http://www.pinguino.cc/
    PURPOSE:		Display number in decimal or non-decimal base
    PROGRAMERS:		regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    TODO :
    --------------------------------------------------------------------
    CHANGELOG
    10 Nov 2010 - Régis Blanchot - first release
    05 Feb 2016 - Régis Blanchot - externalized the function to this file
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

#ifndef __PRINTNUMBER_C
#define __PRINTNUMBER_C

#include <typedef.h>

void printNumber(funcout printChar, s32 value, u8 base)
{  
    u8 tmp[12];
    u8 *tp = tmp;               // pointer on tmp
    u8 i, sign;
    u32 v;                      // absolute value

    //pn_printChar = func;

    if (value == 0)
    {
        printChar('0');
        return;
    }
    
    sign = ( (base == 10) && (value < 0) );

    if (sign)
    {
        printChar('-');
        v = -value;
    }
    else
        v = (u32)value;

    while (v)
    {
        i = v % base;
        v = v / base;
        
        if (i < 10)
            *tp++ = i + '0';
        else
            *tp++ = i + 'A' - 10;
    }

    // backwards writing 
    while (tp > tmp)
        printChar(*--tp);
}

#endif /* __PRINTNUMBER_C */
