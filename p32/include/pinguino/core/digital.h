/*    ----------------------------------------------------------------------------
    FILE:           digital.h
    PROJECT:        pinguino
    PURPOSE:
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    ----------------------------------------------------------------------------
    CHANGELOG:
    [15-12-2017][rblanchot@gmail.com][moved from digitalw.c and analog.c]
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

#ifndef __DIGITAL_H
#define __DIGITAL_H

#define pA      0
#define pB      1
#define pC      2
#define pD      3
#define pE      4
#define pF      5
#define pG      6

#define _0      (1<<0)                  // 0x0001
#define _1      (1<<1)                  // 0x0002
#define _2      (1<<2)                  // 0x0004
#define _3      (1<<3)                  // 0x0008
#define _4      (1<<4)                  // 0x0010
#define _5      (1<<5)                  // 0x0020
#define _6      (1<<6)                  // 0x0040
#define _7      (1<<7)                  // 0x0080
#define _8      (1<<8)                  // 0x0100
#define _9      (1<<9)                  // 0x0200
#define _10     (1<<10)                 // 0x0400
#define _11     (1<<11)                 // 0x0800
#define _12     (1<<12)                 // 0x1000
#define _13     (1<<13)                 // 0x2000
#define _14     (1<<14)                 // 0x4000
#define _15     (1<<15)                 // 0x8000
#define nil     (1<<16)

#endif    /* __DIGITAL_H */
