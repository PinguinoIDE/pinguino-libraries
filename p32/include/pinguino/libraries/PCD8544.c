/*****************************************************************
    ███████╗ █████╗ ███████╗██╗  ██╗██╗  ██╗██╗      ██████╗██████╗  *
    ██╔════╝██╔══██╗██╔════╝██║  ██║██║  ██║██║     ██╔════╝██╔══██╗ *
    █████╗  ╚█████╔╝███████╗███████║███████║██║     ██║     ██║  ██║ *
    ██╔══╝  ██╔══██╗╚════██║╚════██║╚════██║██║     ██║     ██║  ██║ *
    ██║     ╚█████╔╝███████║     ██║     ██║███████╗╚██████╗██████╔╝ *
    ╚═╝      ╚════╝ ╚══════╝     ╚═╝     ╚═╝╚══════╝ ╚═════╝╚═════   *
-------------------------------------------------------Alpha version *
 ******************************************************************
 PCD8544.c :
 * C library for Monochrome Nokia LCD
 * with PCD8544 controler
 * for pinguino boards

    ->File        : PCD8544.c
    ->Revision    : 0.01 Alpha
    ->Last update : March 2014
    ->Description : f8544lcd core code.
    ->Author      : Thomas Missonier (sourcezax@users.sourceforge.net). 

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

---------------------------------------------------------------------------
*********************************************************************/

#ifndef __PCD8544C
#define __PCD8544C

#include <PCD8544.h>
//Includes Pinguino
//#include <pic18fregs.h>
#include <const.h>          // false, true, ...
//#include <stdint.h>
#include <digitalw.c>
#include <stdlib.h>
#include <string.h>

#ifdef _PCD8544_USE_TEXT
#include <fonts/font6x8.h>
#include <string.h>
#endif

#include <macro.h>
#include <typedef.h>
#include <delay.c>

//Alias for _BV 
#define 	_BV(bit)   (1 << (bit))

//To check :Spi hardware on pinguino? 
//Fast Spiwrite is avr code and Pinguino provides hardware spi.
//Need to be implemented, using standard slow Shiftout Method
#define PCD8544_fastSPIwrite PCD8544_slowSPIwrite 
#define PCD8544_abs(x) (((x) < 0) ? -(x) : (x))
#define PCD8544_height() (pcd8544._height)
#define PCD8544_width() (pcd8544._width)

#ifdef _PCD8544_USE_TEXT
#define PCD8544_setTextWrap(w) pcd8544.wrap = (w)
#endif 

#ifdef _PCD8544_USE_ROTATION
#define PCD8544_getRotation() pcd8544.rotation
#endif

#ifdef  _PCD8544_USE_RECT
#define PCD8544_fillScreen(color) PCD8544_fillRect(0, 0,pcd8544._width,pcd8544._height, (color))
#endif

#ifdef _PCD8544_USE_ROUND_RECT
#define _PCD8544_USE_CIRCLE
#define _PCD8544_USE_RECT
#endif

//Structure creation
PCD8544 pcd8544;

//Video buffer 
u8 pcd8544_buffer[LCDBUFFERSIZE] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFE, 0xFF, 0xFC, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80, 0xC0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x7F,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x87, 0x8F, 0x9F, 0x9F, 0xFF, 0xFF, 0xFF,
0xC1, 0xC0, 0xE0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFE, 0xFE, 0xFE,
0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xE0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x80, 0xC0, 0xE0, 0xF1, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x0F, 0x0F, 0x87,
0xE7, 0xFF, 0xFF, 0xFF, 0x1F, 0x1F, 0x3F, 0xF9, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFD, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xF0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
0x7E, 0x3F, 0x3F, 0x0F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0xE0, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFC, 0xF0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01,
0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x1F, 0x3F, 0x7F, 0x7F,
0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/*
void PCD8544_shiftOut(u8 dataPin, u8 clockPin, u8 bitOrder, u8 val)
{
    u8 i;

    for (i = 0; i < 8; i++)  {
        if (bitOrder == LSBFIRST)
            digitalwrite(dataPin, !!(val & (1 << i)));
        else	
            digitalwrite(dataPin, !!(val & (1 << (7 - i))));
            
        digitalwrite(clockPin, HIGH);
        digitalwrite(clockPin, LOW);		
    }
}
*/

// regis : remove LSBFIRST test (always MSBFIRST here)
void PCD8544_shiftOut(u8 dataPin, u8 clockPin, u8 val)
{
    u8 i;

    for (i = 8; i > 0; i--)
    {
        digitalwrite(dataPin, (val & 0x80) ? HIGH : LOW);
        digitalwrite(clockPin, HIGH);
        digitalwrite(clockPin, LOW);
        val <<= 1;
    }
}

void PCD8544_command(u8 c)
{
    // DC = LOW => COMMAND
    digitalwrite(pcd8544._dc, LOW);
    if (pcd8544._cs > 0)
        digitalwrite(pcd8544._cs, LOW);
        
    PCD8544_shiftOut(pcd8544._din, pcd8544._sclk, c);
    
    if (pcd8544._cs > 0)
        digitalwrite(pcd8544._cs, HIGH); 
}
 
void PCD8544_data(u8 c)
{
    // DC = HIGH => DATA
    digitalwrite(pcd8544._dc, HIGH);
    if (pcd8544._cs > 0)
        digitalwrite(pcd8544._cs, LOW);
        
    PCD8544_shiftOut(pcd8544._din, pcd8544._sclk, c);
    
    if (pcd8544._cs > 0)
        digitalwrite(pcd8544._cs, HIGH);
}

#ifdef enablePartialUpdate
static u8 xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;

static void PCD8544_updateBoundingBox(u8 xmin, u8 ymin, u8 xmax, u8 ymax)
{
  if (xmin < xUpdateMin) xUpdateMin = xmin;
  if (xmax > xUpdateMax) xUpdateMax = xmax;
  if (ymin < yUpdateMin) yUpdateMin = ymin;
  if (ymax > yUpdateMax) yUpdateMax = ymax;
}
#endif

void PCD8544_init(s8 SCLK, s8 DIN, s8 DC, s8 CS, s8 RST)
{
    //pins
    pcd8544._din = DIN;
    pcd8544._sclk = SCLK;
    pcd8544._dc = DC;
    pcd8544._rst = RST;
    pcd8544._cs = CS;

    //GFX
    pcd8544._width    = LCDWIDTH;
    pcd8544. _height  = LCDHEIGHT;
    pcd8544.rotation  = 0;
    pcd8544.cursor_y  = 0;
    pcd8544.cursor_x  = 0;
    pcd8544.textsize  = 1;
    pcd8544.textcolor = pcd8544.textbgcolor = 0xFFFF;
    pcd8544.wrap = true;

    digitalwrite(pcd8544._dc,   HIGH);
    digitalwrite(pcd8544._cs,   HIGH);
    digitalwrite(pcd8544._rst,  HIGH);
    digitalwrite(pcd8544._din,  HIGH);
    digitalwrite(pcd8544._sclk, HIGH);

    // set pin directions
    pinmode(pcd8544._din,  OUTPUT);
    pinmode(pcd8544._sclk, OUTPUT);
    pinmode(pcd8544._dc,   OUTPUT);
    if (pcd8544._rst > 0)
    pinmode(pcd8544._rst,  OUTPUT);
    if (pcd8544._cs > 0)
    pinmode(pcd8544._cs,   OUTPUT);

    // toggle RST low to reset
    if (pcd8544._rst > 0)
    {
        digitalwrite(pcd8544._rst, LOW);
        Delayms(30);
        digitalwrite(pcd8544._rst, HIGH);
    }

    // get into the EXTENDED mode!
    PCD8544_command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );

    // LCD bias select (4 is optimal?)
    PCD8544_command(PCD8544_SETBIAS | 0x4);

    // set VOP
    PCD8544_command(PCD8544_SETVOP | 40); // contrast set to 40 by default

    // normal mode
    PCD8544_command(PCD8544_FUNCTIONSET);

    // Set display to Normal
    PCD8544_command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

    // initial display line
    // set page address
    // set column address
    // write display data

    // set up a bounding box for screen updates

    #ifdef enablePartialUpdate
    PCD8544_updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
    #endif
    
    // Push out pcd8544_buffer to the Display (will show the AFI logo)
    PCD8544_refresh();
}

//draw Pixel on the buffer
void PCD8544_drawPixel(s16 x, s16 y, u16 color)
{
    if (x < 0) return;
    if (x >= LCDWIDTH) return;
    if (y < 0) return;
    if (y >= LCDHEIGHT) return;

    // x is which column
    if (color) 
        pcd8544_buffer[x + (y>>3) * LCDWIDTH] |= _BV(y%8);  
    else
        pcd8544_buffer[x + (y>>3) * LCDWIDTH] &= ~_BV(y%8); 
    
    #ifdef enablePartialUpdate
    PCD8544_updateBoundingBox(x,y,x,y);
    #endif
} 

u8 PCD8544_getPixel(s8 x, s8 y)
{
    if (x < 0) return;
    if (x >= LCDWIDTH) return;
    if (y < 0) return;
    if (y >= LCDHEIGHT) return;

    return (pcd8544_buffer[x+ (y>>3)*LCDWIDTH] >> (y%8)) & 0x1;  
}

void PCD8544_setContrast(u8 val)
{
    //if (val > 0x7f) val = 0x7f;
    PCD8544_command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
    PCD8544_command(PCD8544_SETVOP | (val & 0x7F)); 
    PCD8544_command(PCD8544_FUNCTIONSET);
}

void PCD8544_refresh()
{
    u8 col, maxcol, p;
      
    for(p = 0; p < 6; p++) 
    {
        PCD8544_command(PCD8544_SETYADDR | p);
        col = 0;
        maxcol = LCDWIDTH-1;
        PCD8544_command(PCD8544_SETXADDR | col);
        // send data in DDRAM
        digitalwrite(pcd8544._dc, HIGH);

        if (pcd8544._cs > 0)
            digitalwrite(pcd8544._cs, LOW);

        for(; col <= maxcol; col++) 
              PCD8544_shiftOut(pcd8544._din, pcd8544._sclk, pcd8544_buffer[(LCDWIDTH*p)+col]);

        if (pcd8544._cs > 0)
              digitalwrite(pcd8544._cs, HIGH);

    }

    // no idea why this is necessary but it is to finish the last byte?
    PCD8544_command(PCD8544_SETYADDR );

    #ifdef enablePartialUpdate
      xUpdateMin = LCDWIDTH - 1;
      xUpdateMax = 0;
      yUpdateMin = LCDHEIGHT-1;
      yUpdateMax = 0;
    #endif
}

void PCD8544_clearScreen()
{
    memset(pcd8544_buffer, 0, LCDBUFFERSIZE);
    #ifdef enablePartialUpdate
    PCD8544_updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
    #endif
    pcd8544.cursor_y =0;
    pcd8544.cursor_x = 0;
}
 
//Adafruit GFX fusion functions
//Rewritted PCD8544_drawCircle for pic optimization 
#ifdef  _PCD8544_USE_CIRCLE
void PCD8544_drawCircle(s16 x0, s16 y0, s16 r, u16 color)
{
 s16 x,y,radiusError;
 x=r;
 y=0;
 radiusError =1-x;
 while(x >= y)
  {
    PCD8544_drawPixel(x + x0, y + y0, color);
    PCD8544_drawPixel(y + x0, x + y0, color);
    PCD8544_drawPixel(-x + x0, y + y0, color);
    PCD8544_drawPixel(-y + x0, x + y0, color);
    PCD8544_drawPixel(-x + x0, -y + y0, color);
    PCD8544_drawPixel(-y + x0, -x + y0, color);
    PCD8544_drawPixel(x + x0, -y + y0, color);
    PCD8544_drawPixel(y + x0, -x + y0, color);
    y++;
    if (radiusError<0)
    {
      radiusError += (y<<1) + 1;
    } else {
      x--;
      radiusError+= ((y - x + 1)<<1);
    }
  }
 }
 
 /*void PCD8544_drawCircle(s16 x0, s16 y0, s16 r, u16 color)
 {
     s16 f = 1 - r;
     s16 ddF_x = 1;
     s16 ddF_y = -2 * r;
     s16 x = 0;
     s16 y = r;

     PCD8544_drawPixel(x0  , y0+r, color);
     PCD8544_drawPixel(x0  , y0-r, color);
     PCD8544_drawPixel(x0+r, y0  , color);
     PCD8544_drawPixel(x0-r, y0  , color);

     while (x<y) 
     {
        if (f >= 0) 
        {
             y--;
             ddF_y += 2;
             f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        PCD8544_drawPixel(x0 + x, y0 + y, color);
        PCD8544_drawPixel(x0 - x, y0 + y, color);
        PCD8544_drawPixel(x0 + x, y0 - y, color);
        PCD8544_drawPixel(x0 - x, y0 - y, color);
        PCD8544_drawPixel(x0 + y, y0 + x, color);
        PCD8544_drawPixel(x0 - y, y0 + x, color);
        PCD8544_drawPixel(x0 + y, y0 - x, color);
        PCD8544_drawPixel(x0 - y, y0 - x, color);
     }
 }
 */
 
  void PCD8544_drawCircleHelper(s16 x0, s16 y0, s16 r, u8 cornername,u16 color)
  {
      s16 f     = 1 - r;
      s16 ddF_x = 1;
      s16 ddF_y = -2 * r;
      s16 x     = 0;
      s16 y     = r;

      while (x<y) 
      {
          if (f >= 0) 
          {
               y--;
               ddF_y += 2;
               f+= ddF_y;
          }
          x++;
          ddF_x += 2;
          f+= ddF_x;
          if (cornername & 0x4) {
              PCD8544_drawPixel(x0 + x, y0 + y, color);
              PCD8544_drawPixel(x0 + y, y0 + x, color);
            } 
          if (cornername & 0x2) {
              PCD8544_drawPixel(x0 + x, y0 - y, color);
              PCD8544_drawPixel(x0 + y, y0 - x, color);
            }
          if (cornername & 0x8) {
              PCD8544_drawPixel(x0 - y, y0 + x, color);
              PCD8544_drawPixel(x0 - x, y0 + y, color);
            }
          if (cornername & 0x1) {
             PCD8544_drawPixel(x0 - y, y0 - x, color);
              PCD8544_drawPixel(x0 - x, y0 - y, color);
            }
      }
  }

  void PCD8544_fillCircle(s16 x0, s16 y0, s16 r, u16 color)
  {
      PCD8544_drawFastVLine(x0, y0-r, (r<<1)+1, color);
      PCD8544_fillCircleHelper(x0, y0, r, 3, 0, color);
  }

void PCD8544_fillCircleHelper(s16 x0, s16 y0, s16 r, u8 cornername,s16 delta, u16 color)
{
    s16 f     = 1 - r;
    s16 ddF_x = 1;
    s16 ddF_y = -2 * r;
    s16 x     = 0;
    s16 y     = r;
     while (x<y) 
    {
        if (f >= 0) 
          {
              y--;
              ddF_y += 2;
              f += ddF_y;
          }
         x++;
         ddF_x += 2;
         f+= ddF_x;
         if (cornername & 0x1)
         {
              PCD8544_drawFastVLine(x0+x, y0-y, (y<<1)+1+delta, color);
              PCD8544_drawFastVLine(x0+y, y0-x, (x<<1)+1+delta, color);
         }
        if (cornername & 0x2) 
        {
            PCD8544_drawFastVLine(x0-x, y0-y, (y<<1)+1+delta, color);
            PCD8544_drawFastVLine(x0-y, y0-x, (x<<1)+1+delta, color);
        }
         
    }
}  
#endif
   void PCD8544_drawLine(s16 x0, s16 y0, s16 x1, s16 y1, u16 color)
   {
      s16 dx, dy,steep,err,ystep;
      steep = PCD8544_abs(y1 - y0) > PCD8544_abs(x1 - x0);
      if (steep) 
      {
        swap(x0, y0);
        swap(x1, y1);
      }

      if (x0 > x1) 
      {
        swap(x0, x1);
        swap(y0, y1);
      }

      
      dx = x1 - x0;
      dy = PCD8544_abs(y1 - y0);

      err = (dx>>1);
     

      if (y0 < y1) 
      {
        ystep = 1;
      } 
      else 
      {
        ystep = -1;
      }

      for (; x0<=x1; x0++) 
      {
            if (steep) 
            {
              PCD8544_drawPixel(y0, x0, color);
            } 
            else 
            {
              PCD8544_drawPixel(x0, y0, color);
            }
          err -= dy;
          if (err < 0) 
          {
          y0 += ystep;
           err += dx;
          }
      }
   }
   #ifdef _PCD8544_USE_RECT
void PCD8544_drawRect(s16 x, s16 y, s16 w, s16 h, u16 color)
{
    PCD8544_drawFastHLine(x, y, w, color);
    PCD8544_drawFastHLine(x, y+h-1, w, color);
    PCD8544_drawFastVLine(x, y, h, color);
    PCD8544_drawFastVLine(x+w-1, y, h, color);
}
void PCD8544_fillRect(s16 x, s16 y, s16 w, s16 h, u16 color)
 {
    s16 i;
    for (i=x; i<x+w; i++) 
    {
        PCD8544_drawFastVLine(i, y, h, color);
    }
 }
 #endif

void PCD8544_drawFastVLine(s16 x, s16 y, s16 h, u16 color)
{
    PCD8544_drawLine(x, y, x, y+h-1, color);
}

void PCD8544_drawFastHLine(s16 x, s16 y, s16 w, u16 color)
 {
    PCD8544_drawLine(x, y, x+w-1, y, color);
 }
 

/*void PCD8544_fillScreen(u16 color)
 {
    PCD8544_fillRect(0, 0,pcd8544._width,pcd8544._height, color);
 }*/
#ifdef _PCD8544_USE_ROUND_RECT
 void PCD8544_drawRoundRect(s16 x, s16 y, s16 w,s16 h, s16 r, u16 color)
 {
     PCD8544_drawFastHLine(x+r  , y    , w-(r<<1), color); // Top
     PCD8544_drawFastHLine(x+r  , y+h-1, w-(r<<1), color); // Bottom
     PCD8544_drawFastVLine(x    , y+r  , h-(r<<1), color); // Left
     PCD8544_drawFastVLine(x+w-1, y+r  , h-(r<<1), color); // Right
      // draw four corners
     PCD8544_drawCircleHelper(x+r    , y+r    , r, 1, color);
     PCD8544_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
     PCD8544_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
     PCD8544_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
 }
 
 void PCD8544_fillRoundRect(s16 x, s16 y, s16 w,s16 h, s16 r, u16 color)
 {
     // smarter version
     PCD8544_fillRect(x+r, y, w-2*r, h, color);
     // draw four corners
     PCD8544_fillCircleHelper(x+w-r-1, y+r, r, 1, h-(r<<1)-1, color);
     PCD8544_fillCircleHelper(x+r    , y+r, r, 2, h-(r<<1)-1, color);
}
#endif

#ifdef  _PCD8544_USE_TRIANGLE
void PCD8544_drawTriangle(s16 x0, s16 y0, s16 x1, s16 y1,s16 x2, s16 y2, u16 color)
{
    PCD8544_drawLine(x0, y0, x1, y1, color);
    PCD8544_drawLine(x1, y1, x2, y2, color);
    PCD8544_drawLine(x2, y2, x0, y0, color);
}
void PCD8544_fillTriangle(s16 x0, s16 y0, s16 x1, s16 y1,s16 x2, s16 y2, u16 color)
{
 s16 a, b, y, last;
 s16 dx01,dy01,dx02,dy02,dx12,sa,sb,dy12; 
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    PCD8544_drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  
    dx01 = x1 - x0;
    dy01 = y1 - y0;
    dx02 = x2 - x0;
    dy02 = y2 - y0;
    dx12 = x2 - x1;
    dy12 = y2 - y1;
    sa   = 0;
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    PCD8544_drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    PCD8544_drawFastHLine(a, y, b-a+1, color);
  }
}
#endif
#ifdef  _PCD8544_USE_BITMAP
void PCD8544_drawBitmap(s16 x, s16 y, const u8 *bitmap,s16 w, s16 h, u16 color)
{
    s16 i, j, byteWidth;
    byteWidth= (w + 7) / 8;

    for(j=0; j<h; j++) 
      {
          for(i=0; i<w; i++ ) 
          {
                  if((bitmap[(j * byteWidth + i / 8)]) & (128 >> (i & 7))) 
                  {	
                    PCD8544_drawPixel(x+i, y+j, color);
                  }
          }
      }
}
#endif

#ifdef _PCD8544_USE_TEXT
void PCD8544_write(u8 c)
{
     if (c == '\n') 
     {
         pcd8544.cursor_y += ((pcd8544.textsize)<<3);
         pcd8544.cursor_x  = 0;
     } 
     else if (c == '\r') 
     {
        // skip em
     } 
     else 
     {
        PCD8544_drawChar(pcd8544.cursor_x,pcd8544.cursor_y, c, pcd8544.textcolor, pcd8544.textbgcolor,pcd8544.textsize);
        pcd8544.cursor_x += (pcd8544.textsize)*6;
        if (pcd8544.wrap && (pcd8544.cursor_x > (pcd8544._width - pcd8544.textsize*6))) 
        {
          pcd8544.cursor_y += ((pcd8544.textsize)<<3);
          pcd8544.cursor_x = 0;
        }
      }
}

void PCD8544_drawChar(s16 x, s16 y, u8 c, u16 color,u16 bg, u8 size)
{
    s8 i,j,k;
    u8 line;

    // test also done in drawPixel
    /*
    if((x >= pcd8544._width)       || // Clip right
        (y >= pcd8544._height)     || // Clip bottom
        ((x + 6 * size - 1) < 0)   || // Clip left
        ((y + (size>>3) - 1) < 0))    // Clip top
        return;
    */
    
    if (c<0x20)return;
    if (c>0x7f)return;
  
    for (i=0; i<6; i++ )
    {
        //if (i == 5) 
        //    line = 0x0;
        //else //line = pgm_read_byte(font+(c*5)+i);
            line = font6x8[ 2 + (c - 32) * 6 + i ];

        for (j = 0; j<8; j++)
        {
            if (line & 0x01)
            {
                if (size == 1) // default size
                {
                    PCD8544_drawPixel(x+i, y+j, color);
                }
                else
                {
                    // big size
                    //PCD8544_fillRect(x+(i*size), y+(j*size), size, size, color);
                    for (k=(x+(i*size)); k<x+(i*size)+size; k++) 
                        PCD8544_drawFastVLine(k, y+(j*size), size, color);
                } 
            }
            
            else if (bg != color)
            {
                if (size == 1) // default size
                {
                    PCD8544_drawPixel(x+i, y+j, bg);
                }
                else
                {
                    // big size
                    //PCD8544_fillRect(x+(i*size), y+(j*size), size, size, bg);
                    for (k=(x+(i*size)); k<x+(i*size)+size; k++) 
                        PCD8544_drawFastVLine(k, y+(j*size), size, bg);
                }
            }

            line >>= 1;
        }
    }
}

void PCD8544_setCursor(s16 x, s16 y)
{
     pcd8544.cursor_x = x;
     pcd8544.cursor_y = y;
}

void PCD8544_setTextSize(u8 s)
{
    pcd8544.textsize = (s > 0) ? s : 1;
}

void PCD8544_setTextColor(u16 c)
{
    pcd8544.textcolor = c;
    pcd8544.textbgcolor = c;
}

void PCD8544_setTextColor2(u16 c, u16 bg)
{
    pcd8544.textcolor   = c;
    pcd8544.textbgcolor = bg; 
}
#endif

#ifdef _PCD8544_USE_ROTATION
void PCD8544_setRotation(u8 x)
{
    pcd8544.rotation = (x & 3);
    switch(pcd8544.rotation) 
      {
           case 0:
           case 2:
               pcd8544._width  = pcd8544.WIDTH;
               pcd8544._height = pcd8544.HEIGHT;
            break;
           case 1:
           case 3:
               pcd8544._width  = pcd8544.HEIGHT;
               pcd8544._height = pcd8544.WIDTH;
            break;
      }
}
#endif
/* Replaced by define to optimize code 
void PCD8544_setTextWrap(boolean w)
{
    pcd8544.wrap = w;
}
u8 PCD8544_getRotation()
{
    return pcd8544.rotation;
}
s16 PCD8544_width()
{
    return pcd8544._width;
}
s16 PCD8544_height()
{
    return pcd8544._height;
} */
#ifdef  _PCD8544_USE_INVERT
void PCD8544_invertDisplay(bool i) {
  // Do nothing, must be subclassed if supported
}
#endif

//Print functions 
#ifdef _PCD8544_USE_TEXT
void PCD8544_print(char *chaine)
{
    u8 counter;
    for( counter=0; counter<strlen(chaine); counter++)
        //PCD8544_write((chaine[counter - 1] < 10 ? '0' + chaine[counter - 1] : 'A' + chaine[counter - 1] - 10));
        PCD8544_write(chaine[counter]);
}

/* taken from lcdlib.h Print a number on LCD */
void PCD8544_printNumber(u16 n, u8 base)
{  
    u8 buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
    u16 i = 0;

    if (n == 0)
    {
        PCD8544_write('0');
        return;
    } 

    while (n > 0)
    {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
        PCD8544_write((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}
#endif
//Abs function
// Goal : Not using Math lib for one function
/*
s16 PCD8544_abs(s16 nb)
{
if (nb<0) nb=-nb;
return nb;
} */

#endif
