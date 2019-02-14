/*  --------------------------------------------------------------------
    FILE:			trigo.c
    PROJECT:		pinguino
    PURPOSE:		optimized trigonometric calculation
                    http://www.dattalo.com/technical/theory/sinewave.html
    PROGRAMER:		Regis Blanchot
    FIRST RELEASE:	07 Apr. 2012
    LAST RELEASE:	18 Mar. 2014
    --------------------------------------------------------------------
    CHANGELOG : 
    Apr 07 2012 - initial release, sin and cos
    Feb 08 2013 - added some comments for better understanding
    Mar 18 2014 - added fast and accurate float sine/cosine
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

#ifndef __TRIGO_C
#define __TRIGO_C

#include <typedef.h>
//#include <macro.h>
//#include <const.h>
//#include <math.h>

/*  --------------------------------------------------------------------
    First 90 values (1/4's of the circle) of precomputed sinus table * 256
    To obtain the other 3/4's of the circle, we use the symmetry of sine wave.
    Each quadrant is obtained as follows:
      0 <= x < 90      y =  sine[ x ];
     90 <= x < 180     y =  sine[ 180 - x ];
    180 <= x < 270     y = -sine[ x - 180 ];
    270 <= x < 360     y = -sine[ 360 - x ];
    ------------------------------------------------------------------*/

#define _abs(x)     ((x)>0?(x):-(x));

// http://devmaster.net/posts/9648/fast-and-accurate-sine-cosine
#if defined(SINR) || defined(COSR)
float sine(int i)
{
    float x =  0.01745329 * (float)i;   // degree to rad
    float B =  1.27323954;              // 4/pi;
    float C = -0.40528473;              //-4/(pi*pi);
    float y = B * x + C * x * _abs(x);
    #ifdef TRIGO_EXTRA_PRECISION
    float P = 0.225;
    y = P * (y * _abs(y) - y) + y;
    #endif
    return y;
}
#endif

/*  --------------------------------------------------------------------
    sine function
    alpha:  angle in degree
    return: sine value in float
    ------------------------------------------------------------------*/

#if defined(SINR) || defined(COSR)
float sinr(int alpha)
{
    u8 sign = 0; // positive

    // normalize the angle
    if (alpha < 0)
        alpha = 360 - ((-alpha) % 360);
    else
        alpha %= 360;

    // sin(a+180) = - sin(a)
    if (alpha >= 180)
    {
        alpha -= 180;
        sign = 1; // negative 
    }

    // now a < 180
    // sin(180-a) == sin(a);
    if (alpha > 90)
        alpha = 180 - alpha;

    if (sign)
        return -sine(alpha);
    else
        return  sine(alpha);
}
#endif

/*  --------------------------------------------------------------------
    cosine function
    based on cos(a) = sin (a + 90)
    alpha:  angle in degree
    return: cosine value in float
    ------------------------------------------------------------------*/

#ifdef COSR
float cosr(int alpha)
{
    return sinr(alpha + 90);
}
#endif

/*  --------------------------------------------------------------------
    Return approximation of cos(i) where i is angle in integer degrees 0-359.
    Returned value is in range -100 to 100 corresponding to -1.0 to 1.0.
    ------------------------------------------------------------------*/

#if defined(COS100) || defined(SIN100)
s8 cos100(u16 alpha)
{
    s8 sign = 1;

    // input is 0-359 but curve is fit to 0-90
    // so perform quadrant conversion

    if (alpha > 270)
    {
        alpha = 360 - alpha;
    }
    else if (alpha > 180)
    {
        alpha = alpha - 180;
        sign = -1;
    } 
    else if (alpha > 90)
    {
        alpha = 180 - alpha;
        sign = -1;
    }

    // apply quadratic fit approximation
    return (((10188 - alpha * (alpha + 23)) / 101) * sign);
}
#endif

/*  --------------------------------------------------------------------
    Return approximation of sin(i) where i is angle in integer degrees 0-359.
    Returned value is in range -100 to 100 corresponding to -1.0 to 1.0.
    ------------------------------------------------------------------*/

#if defined(SIN100)
s8 sin100(u16 alpha)
{
    // sin is cos shifted -90 degrees
    return cos100((alpha + 270) % 360);
}
#endif

#endif /* __TRIGO_C */
