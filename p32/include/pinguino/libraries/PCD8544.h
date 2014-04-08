/*****************************************************************
	███████╗ █████╗ ███████╗██╗  ██╗██╗  ██╗██╗      ██████╗██████╗  *
	██╔════╝██╔══██╗██╔════╝██║  ██║██║  ██║██║     ██╔════╝██╔══██╗ *
	█████╗  ╚█████╔╝███████╗███████║███████║██║     ██║     ██║  ██║ *
	██╔══╝  ██╔══██╗╚════██║╚════██║╚════██║██║     ██║     ██║  ██║ *
	██║     ╚█████╔╝███████║     ██║     ██║███████╗╚██████╗██████╔╝ *
	╚═╝      ╚════╝ ╚══════╝     ╚═╝     ╚═╝╚══════╝ ╚═════╝╚═════   *
-------------------------------------------------------Alpha version *
 ******************************************************************
         f8544lcd : C PCD8544 & GFX libs For Monochrome Nokia LCD (Model PCD8544) for pinguino boards

	->File :f8544lcd.h
    ->Revision : 0.01 Alpha
	->Last update : March 2014
	-> Description : f8544lcd library headers file
	
 This port is non affiliated, non licensed , non endorsed, and non supported in any way by Adafruit or Microchip.
 
This lib is a free and individual Pinguino/ PIC18F C port/rewriting of a mix of Two C++ libs From Adafruit :
 
 - Adafruit PCD8544 

  and 

- AdaFruit GFX                                

Known limitation : Can drive only 1 display at time (as original library).


Ported to Pinguino/Pic by Thomas Missonier (sourcezax@users.sourceforge.net). 


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions, the others licences below, and the following disclaimer.
- Redistributions in binary form must reproduce the above notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------
Adafruit PCD8544
*//*********************************************************************
This is a library for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************
AdaFruit GFX
---------------------------------------------------------------------------------
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!
 
Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*********************************************************************/
#ifndef __PCD8544H
#define __PCD8544H

#include <typedef.h>    // Pinguino's type : u8, u16, ..., and bool

#define BLACK 1
#define WHITE 0

#define LCDWIDTH  84
#define LCDHEIGHT 48
#define LCDBUFFERSIZE (LCDWIDTH * LCDHEIGHT / 8)

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

//Used by PCD8544_shiftOut
#define LSBFIRST 0
#define MSBFIRST 1

typedef struct
{
    s8 _din, _sclk, _dc, _rst, _cs;
    volatile u8 *mosiport, *clkport, *csport, *dcport;
    u8 mosipinmask, clkpinmask, cspinmask, dcpinmask;

    const s16   WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
    s16  _width, _height, // Display w/h as modified by current rotation
    cursor_x, cursor_y;
    u16 textcolor, textbgcolor;
    u8  textsize,  rotation;
    boolean  wrap;
  // the memory buffer for the LCD
} PCD8544;
   
void PCD8544_init(s8 SCLK, s8 DIN, s8 DC, s8 CS, s8 RST);

void PCD8544_command(u8 c);
void PCD8544_data(u8 c);

void PCD8544_setContrast(u8 val);
void PCD8544_clearScreen();
void PCD8544_refresh();

void PCD8544_drawPixel(s16 x, s16 y, u16 color);
u8 PCD8544_getPixel(s8 x, s8 y);

//Adafruit GFX Fusion
#ifdef _PCD8544_USE_CIRCLE
void PCD8544_drawCircle(s16 x0, s16 y0, s16 r, u16 color);
void PCD8544_drawCircleHelper(s16 x0, s16 y0, s16 r, u8 cornername,u16 color);
void PCD8544_fillCircle(s16 x0, s16 y0, s16 r, u16 color);
void PCD8544_fillCircleHelper(s16 x0, s16 y0, s16 r, u8 cornername,s16 delta, u16 color);
#endif
#ifdef _PCD8544_USE_TRIANGLE
void PCD8544_drawTriangle(s16 x0, s16 y0, s16 x1, s16 y1,s16 x2, s16 y2, u16 color);
void PCD8544_fillTriangle(s16 x0, s16 y0, s16 x1, s16 y1,s16 x2, s16 y2, u16 color);
#endif
#ifdef _PCD8544_USE_ROUND_RECT
void PCD8544_drawRoundRect(s16 x0, s16 y0, s16 w, s16 h,s16 radius, u16 color);
void PCD8544_fillRoundRect(s16 x, s16 y, s16 w,s16 h, s16 r, u16 color);
#endif
#ifdef _PCD8544_USE_BITMAP
void PCD8544_drawBitmap(s16 x, s16 y, const u8 *bitmap,s16 w, s16 h, u16 color);
#endif
#ifdef  _PCD8544_USE_TEXT
void PCD8544_drawChar(s16 x, s16 y, u8, u16 color,u16 bg, u8 size);
void PCD8544_setCursor(s16 x, s16 y);
void PCD8544_setTextColor(u16 c);
void PCD8544_setTextColor2(u16 c, u16 bg);
void PCD8544_setTextSize(u8 s);
#endif
//void PCD8544_setTextWrap(boolean w);
#ifdef _PCD8544_USE_ROTATION
void PCD8544_setRotation(u8 r);
#endif
/*s16 PCD8544_height();
s16 PCD8544_width();*/
void PCD8544_drawLine(s16 x0, s16 y0, s16 x1, s16 y1, u16 color);
void PCD8544_drawFastVLine(s16 x, s16 y, s16 h, u16 color);
void PCD8544_drawFastHLine(s16 x, s16 y, s16 w, u16 color);
#ifdef _PCD8544_USE_RECT
void PCD8544_drawRect(s16 x, s16 y, s16 w, s16 h, u16 color);
void PCD8544_fillRect(s16 x, s16 y, s16 w, s16 h, u16 color);
#endif
/*void PCD8544_fillScreen(u16 color);*/
#ifdef _PCD8544_USE_INVERT
//void PCD8544_invertDisplay(boolean i);
#endif
void PCD8544_write(u8 c);
//u8 PCD8544_getRotation();
#ifdef  _PCD8544_USE_INVERT
void PCD8544_invertDisplay(bool i);
#endif
static void PCD8544_updateBoundingBox(u8 xmin, u8 ymin, u8 xmax, u8 ymax);
//Shifout implementation
void PCD8544_shiftOut(u8 dataPin, u8 clockPin, u8 val);
//Print functions
#ifdef _PCD8544_USE_TEXT
void PCD8544_print(char *chaine);
void PCD8544_printNumber(u16 n, u8 base);
#endif
  //Function abs to reduce weight of lib
//s16 PCD8544_abs(s16 nb);

   #endif
