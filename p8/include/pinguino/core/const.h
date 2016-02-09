/*  ----------------------------------------------------------------------------
    FILE:			const.h
    PROJECT:		pinguino
    PURPOSE:		Pinguino main constant definitions
    PROGRAMER:		Régis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:	20 Jun. 2009
    LAST RELEASE:	29 Nov. 2015
    ----------------------------------------------------------------------------
    CHANGELOG:
    2015-11-29 - Régis Blanchot - Commented #define BYTE (conflict with typedef.h)
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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    --------------------------------------------------------------------------*/
    
#ifndef __CONST_H
    #define __CONST_H

    #define PINGUINO_MAJOR_VERSION 12
    #define PINGUINO_MINOR_VERSION 1

    // already defined in math.h
    //#define PI        3.1415926535897932384626433832795
    //#define HALF_PI   1.5707963267948966192313216916398
    //#define TWO_PI    6.2831853071795864769252867665590
    //#define SQR_PI    9.8696044010893586188344909998761
    #define DEG_TO_RAD  0.0174532925199432957692369076848  // PI/180
    #define RAD_TO_DEG  57.295779513082320876798154814105  // 180/PI
    #define rad(x)      (x * DEG_TO_RAD)
    #define deg(x)      (x * RAD_TO_DEG)
     
    // already defined in common_types.h
    #ifndef FALSE
        #define FALSE		0
    #endif
    #define false			0 
    #define False			0 
    
    #ifndef TRUE
        #define TRUE		!FALSE
    #endif
    #define true			!false
    #define True			!false

    #ifndef NULL
      #define NULL          (void *)0
    #endif

    #define INPUT			1
    #define OUTPUT			0

    #define HIGH			1
    #define LOW				0
/*
    #ifndef ON
    #define ON				1
    #endif
    #ifndef OFF
    #define OFF				0
    #endif
*/
    //#define BYTE			1           // conflict with typedef.h
    #define BIN				2
    #define OCT				8 
    #define DEC				10
    #define HEX				16
    #define FLOAT			32

    #define MHZ             1000000
#endif

