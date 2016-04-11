/*	--------------------------------------------------------------------
    FILE:			printFloat.c
    PROJECT:		pinguino - http://www.pinguino.cc/
    PURPOSE:		Display float number
    PROGRAMERS:		regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    TODO : thousands separator
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

#ifndef __PRINTFLOAT_C
#define __PRINTFLOAT_C

#include <typedef.h>
#include <printNumber.c>

void printFloat(funcout printChar, float number, u8 digits)
{ 
    u8 i, toPrint;
    u16 int_part;
    float rounding, remainder;
    
    // Handle negative numbers
    if (number < 0.0)
    {
        printChar('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"  
    rounding = 0.5;
    for (i=0; i<digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it  
    int_part = (u16)number;
    remainder = number - (float)int_part;
    printNumber(printChar, int_part, 10);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
        printChar('.'); 

    // Extract digits from the remainder one at a time
    while (digits-- > 0)
    {
        remainder *= 10.0;
        toPrint = (unsigned int)remainder;
        printNumber(printChar, toPrint, 10);
        remainder -= toPrint; 
    }
}

#endif /* __PRINTFLOAT_C */
