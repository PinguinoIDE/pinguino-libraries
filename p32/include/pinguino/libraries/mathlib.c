/*	----------------------------------------------------------------------------
    FILE:			mathlib.c
    PROJECT:		pinguino
    PURPOSE:		math. functions
    PROGRAMER:		regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:	2009 ?
    LAST RELEASE:	2013 Mai 22
    ----------------------------------------------------------------------------
    CHANGELOG:
    * 2012 Jul. 10  regis blanchot  fixed bug (functions already defined in macro.h)
    * 2013 Mai  22  regis blanchot  added random function
    ----------------------------------------------------------------------------
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
    --------------------------------------------------------------------------*/

#ifndef __MATHLIB_C
#define __MATHLIB_C

//#include <typedef.h>
#include <macro.h>
#include <stdlib.h>

/** --------------------------------------------------------------------
    ---------- abs
    --------------------------------------------------------------------
    Description : 
    Parameters  :
    Returns     : 
    Nota        : 
    ------------------------------------------------------------------*/

int abs(int v)
{
    return (v < 0) ? -(unsigned)v : v;
}

/** --------------------------------------------------------------------
    ---------- random
    --------------------------------------------------------------------
    Description : returns pseudo random number between mini and maxi
    Parameters  :
    Returns     : integer between mini and maxi
    Nota        : random number generator must be initialize with randomSeed function
    ------------------------------------------------------------------*/

int random(int mini, int maxi)
{
    return ( (rand() % maxi ) - mini );
}

/** --------------------------------------------------------------------
    ---------- bounds
    --------------------------------------------------------------------
    Description : 
    Parameters  :
    Returns     : 
    Nota        : 
    ------------------------------------------------------------------*/

long bounds(long x, long _min, long _max)
{
	long temp;
	
	if (_max < _min)
	{
		temp = _max;
		_max = _min;
		_min = temp;
	}
	
	if (x > _max) return _max;
	if (x < _min) return _min;
	return x;
}

/** --------------------------------------------------------------------
    ---------- Map
    --------------------------------------------------------------------
    Description : map(value, fromLow, fromHigh, toLow, toHigh)
    Parameters  :
    Returns     :
    ------------------------------------------------------------------*/

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/** --------------------------------------------------------------------
    ---------- umul16
    --------------------------------------------------------------------
    Description : 
    Parameters  :
    Returns     : 
    Nota        : 
    ------------------------------------------------------------------*/

// about 20 cycles
u16 umul16(u16 multiplier, u16 multiplicand)
{
    u16 product;

    #define uLB16(x) (*(u8 *)(&x))
    #define uHB16(x) (*(((u8 *)(&x))+1))

    product =  (uLB16(multiplier) * uLB16(multiplicand));
    product += (uLB16(multiplier) * uHB16(multiplicand)) << 8;
    product += (uHB16(multiplier) * uLB16(multiplicand)) << 8;

    return product;
}

/** --------------------------------------------------------------------
    ---------- udiv32
    --------------------------------------------------------------------
    Description : 
    Parameters  :
    Returns     : 
    Nota        : 
    ------------------------------------------------------------------*/

// about 100 cycles
u32 udiv32(u32 dividend, u32 divisor)
{
    u32 quotient;
    u8  counter;

    quotient = 0;
    if(divisor != 0)
    {
        counter = 1;
        while((divisor & 0x8000000UL) == 0)
        {
            divisor <<= 1;
            counter++;
        }
        do {
            quotient <<= 1;
            if(divisor <= dividend)
            {
                dividend -= divisor;
                quotient |= 1;
            }
            divisor >>= 1;
        } while(--counter != 0);
    }
    return quotient;
}

#endif
