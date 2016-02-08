/*****************************************************************
    ███████╗ █████╗ ███████╗██╗  ██╗██╗  ██╗██╗      ██████╗██████╗  *
    ██╔════╝██╔══██╗██╔════╝██║  ██║██║  ██║██║     ██╔════╝██╔══██╗ *
    █████╗  ╚█████╔╝███████╗███████║███████║██║     ██║     ██║  ██║ *
    ██╔══╝  ██╔══██╗╚════██║╚════██║╚════██║██║     ██║     ██║  ██║ *
    ██║     ╚█████╔╝███████║     ██║     ██║███████╗╚██████╗██████╔╝ *
    ╚═╝      ╚════╝ ╚══════╝     ╚═╝     ╚═╝╚══════╝ ╚═════╝╚═════   *
-------------------------------------------------------Alpha version *
 ******************************************************************
 PCD8544[module].c :
 * C library for Monochrome Nokia LCD
 * with PCD8544 controler
 * for pinguino boards

    ->File        : PCD8544[module].c
    ->Revision    : 0.01 Alpha
    ->Last update : March 2014
    ->Description : f8544lcd core code.
    ->Author      : Thomas Missonier (sourcezax@users.sourceforge.net). 
------------------------------------------------------------------------
    CHANGELOG:
    04 Feb. 2016 - Régis Blanchot - added SPI library support 
------------------------------------------------------------------------
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

#ifndef __PCD8544_C
#define __PCD8544_C

#include <PCD8544.h>
#include <stdarg.h>
#include <const.h>              // false, true, ...
#include <macro.h>              // BitSet, BitClear, ...
#include <typedef.h>            // u8, u16, ...
#include <delayms.c>            // Delayms, Delayus
#include <spi.c>                // SPI harware and software functions
#include <digitalw.c>           // digitalwrite
#include <digitalp.c>           // pinmode

// Printf
#if defined(PCD8544PRINTF)
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(PCD8544PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics
#ifdef PCD8544GRAPHICS
#include <graphics.c>           // graphic routines
#endif

#include <logo/pinguino84x48.h> // Pinguino Logo


static volatile u8 * PCD8544_buffer = logo;  // screen buffer points on logo[]

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

// DC = LOW => CMD
void PCD8544_command(u8 module, u8 c)
{
    digitalwrite(PCD8544[module].pin.dc, LOW);
    SPI_write(module, c);
}
 
// DC = HIGH => DATA
#if 0
void PCD8544_data(u8 module, u8 c)
{
    digitalwrite(PCD8544[module].pin.dc, HIGH);
    PCD8544_select(module);
    SPI_write(module, c);
    PCD8544_deselect(module);
}
#endif

///	--------------------------------------------------------------------
/// Display functions
///	--------------------------------------------------------------------

#ifdef enablePartialUpdate
static u8 xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;

static void PCD8544_updateBoundingBox(u8 module, u8 xmin, u8 ymin, u8 xmax, u8 ymax)
{
  if (xmin < xUpdateMin) xUpdateMin = xmin;
  if (xmax > xUpdateMax) xUpdateMax = xmax;
  if (ymin < yUpdateMin) yUpdateMin = ymin;
  if (ymax > yUpdateMax) yUpdateMax = ymax;
}
#endif

void PCD8544_init(u8 module, ...)
{
    u8 sda, sck, cs;
    va_list args;
    
    PCD8544_SPI = module;
    
    va_start(args, module); // args points on the argument after module

    PCD8544[module].pin.dc = va_arg(args, u8); // get the first arg
    pinmode(PCD8544[module].pin.dc, OUTPUT);
    
    PCD8544[module].pin.rst = va_arg(args, u8); // get the first arg
    pinmode(PCD8544[module].pin.rst, OUTPUT);
    // toggle RST low to reset
    digitalwrite(PCD8544[module].pin.rst, LOW);
    Delayms(30);
    digitalwrite(PCD8544[module].pin.rst, HIGH);
    
    // init SPI communication

    if (module == SPISW)
    {
        sda = va_arg(args, u8);         // get the next arg
        sck = va_arg(args, u8);         // get the next arg
        cs  = va_arg(args, u8);         // get the last arg
        SPI_setBitOrder(module, SPI_MSBFIRST);
        SPI_begin(module, sda, sck, cs);
    }
    else
    {
        SPI_setMode(PCD8544_SPI, SPI_MASTER);
        SPI_setDataMode(PCD8544_SPI, SPI_MODE0);
        //maximum baud rate possible = FPB = FOSC/4
        SPI_setClockDivider(PCD8544_SPI, SPI_CLOCK_DIV16); // DIV4
        SPI_begin(PCD8544_SPI);
    }
    va_end(args);           // cleans up the list
    
    // GFX properties
    PCD8544[module].screen.width    = DISPLAY_WIDTH;
    PCD8544[module].screen.height   = DISPLAY_HEIGHT;
    PCD8544[module].orientation     = PORTRAIT;
    PCD8544[module].rows            = ((DISPLAY_HEIGHT / 8) - 1);
    PCD8544[module].cursor.y        = 0;
    PCD8544[module].cursor.x        = 0;
    PCD8544[module].textsize        = 1;
    PCD8544[module].color.c         = BLACK;
    PCD8544[module].bcolor.c        = WHITE;
    //PCD8544[module].wrap = true;

    PCD8544_select(module);
    // get into the EXTENDED mode!
    PCD8544_command(module, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
    // LCD bias select (4 is optimal?)
    PCD8544_command(module, PCD8544_SETBIAS | 0x4);
    // set VOP
    PCD8544_command(module, PCD8544_SETVOP | 0x7F); // contrast set to max by default
    // back to normal mode
    PCD8544_command(module, PCD8544_FUNCTIONSET);
    // Set display to Normal
    PCD8544_command(module, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
    PCD8544_deselect(module);

    // set up a bounding box for screen updates
    #ifdef enablePartialUpdate
    PCD8544_updateBoundingBox(0, 0, PCD8544[module].screen.width-1, PCD8544[module].screen.height-1);
    #endif
    
    // Push out PCD8544_buffer to the Display (will show the AFI logo)
    PCD8544_refresh(module);
}

void PCD8544_refresh(u8 module)
{
    u8 col, p;
     
    PCD8544_select(module);

    for(p = 0; p <= PCD8544[module].rows; p++) 
    {
        col = 0;
        PCD8544_command(module, PCD8544_SETYADDR | p);
        PCD8544_command(module, PCD8544_SETXADDR | col);
        // send data in DDRAM
        digitalwrite(PCD8544[module].pin.dc, HIGH);
        //PCD8544_select(module);
        for(; col < PCD8544[module].screen.width; col++) 
            SPI_write(module, PCD8544_buffer[(PCD8544[module].screen.width*p)+col]);
        //PCD8544_deselect(module);
    }

    // no idea why this is necessary ?
    // maybe to finish the last byte ?
    PCD8544_command(module, PCD8544_SETYADDR );

    #ifdef enablePartialUpdate
      xUpdateMin = PCD8544[module].screen.width - 1;
      xUpdateMax = 0;
      yUpdateMin = PCD8544[module].screen.height-1;
      yUpdateMax = 0;
    #endif

    PCD8544_deselect(module);
}

void PCD8544_clearScreen(u8 module)
{   
    //memset(PCD8544_buffer, 0, DISPLAY_SIZE);

    u16 i;

    for (i = 0; i < DISPLAY_SIZE; i++)
        PCD8544_buffer[i] = 0;

    #ifdef enablePartialUpdate
    PCD8544_updateBoundingBox(0, 0, PCD8544[module].screen.width-1, PCD8544[module].screen.height-1);
    #endif

    PCD8544[module].cursor.y = 0;
    PCD8544[module].cursor.x = 0;
}

#ifdef _PCD8544_USE_DISPLAYONOFF
void PCD8544_displayOff(u8 module)
{
    // First, fill RAM with zeroes to ensure minimum specified current consumption
    PCD8544_clearScreen(module);
    PCD8544_refresh(module);
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET|PCD8544_POWERDOWN);
    PCD8544_deselect(module);
}

void PCD8544_displayOn(u8 module)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET);
    PCD8544_deselect(module);
}
#endif

void PCD8544_setContrast(u8 module, u8 val)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
    PCD8544_command(module, PCD8544_SETVOP | (val & 0x7F)); 
    PCD8544_command(module, PCD8544_FUNCTIONSET);
    PCD8544_deselect(module);
}

///	--------------------------------------------------------------------
/// Scroll functions
///	--------------------------------------------------------------------

#if defined(PCD8544PRINTCHAR)   || defined(PCD8544PRINT)      || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT) || \
    defined(PCD8544PRINTLN)     || defined(PCD8544PRINTF)

void PCD8544_scrollUp(u8 module)
{
    u8 x, y;

    // copy line y to line y-1
    for (y = 1; y <= PCD8544[module].rows; y++)
    {
        for (x = 0; x < PCD8544[module].screen.width; x++)
        {
            PCD8544_buffer[x + PCD8544[module].screen.width * (y - 1)] = 
            PCD8544_buffer[x + PCD8544[module].screen.width * y];
        }
    }

    // clear last line
    for (x = 0; x < PCD8544[module].screen.width; x++)
    {
        PCD8544_buffer[x + PCD8544[module].screen.width * PCD8544[module].rows] = 0;
    }    

    PCD8544[module].cursor.y--;
}

#endif

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

#ifdef PCD8544SETFONT
void PCD8544_setFont(u8 module, const u8 *font)
{
    PCD8544[module].font.address = font;
    PCD8544[module].font.width   = font[0];
    PCD8544[module].font.height  = font[1];
}
#endif

#if defined(PCD8544PRINTCHAR)   || defined(PCD8544PRINT)      || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT) || \
    defined(PCD8544PRINTLN)     || defined(PCD8544PRINTF)
    
void printChar(u8 c)
{
    PCD8544_printChar(PCD8544_SPI, c);
}

void PCD8544_printChar(u8 module, u8 c)
{
    u8  b;

    while (PCD8544[module].cursor.x >= (PCD8544[module].screen.width / PCD8544[module].font.width))
    {
        PCD8544[module].cursor.x -= (PCD8544[module].screen.width / PCD8544[module].font.width);
        PCD8544[module].cursor.y++;            
    }

    while (PCD8544[module].cursor.y > PCD8544[module].rows)
    {
        PCD8544_scrollUp(module);            
    }

    switch (c)
    {
        case '\n':
            PCD8544[module].cursor.y++;
            break;
            
        case '\r':
            PCD8544[module].cursor.x = 0;
            break;
            
        case '\t':
            PCD8544[module].cursor.x = (PCD8544[module].cursor.x + 4) % 4;
            break;
            
        default:
            for (b = 0; b < PCD8544[module].font.width; b++)
            {
                PCD8544_buffer[PCD8544[module].cursor.x * PCD8544[module].font.width + PCD8544[module].cursor.y * PCD8544[module].screen.width + b] = 
                    PCD8544[module].font.address[2 + (c - 32) * PCD8544[module].font.width + b];
            }
        
        PCD8544[module].cursor.x++;
    }            
}
#endif

#if defined(PCD8544PRINT)       || defined(PCD8544PRINTLN)    || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
void PCD8544_print(u8 module, const u8 *string)
{
    while (*string != 0)
        PCD8544_printChar(module, *string++);
}
#endif

#if defined(PCD8544PRINTLN)
void PCD8544_println(u8 module, const u8 *string)
{
    PCD8544_print(module, string);
    PCD8544_print(module, (const u8*)"\n\r");
}
#endif

#if defined(PCD8544PRINTCENTER)
void PCD8544_printCenter(u8 module, const u8 *string)
{
    u8 strlen, nbspace;
    const u8 *p;

    for (p = string; *p; ++p);
    strlen = p - string;

    nbspace = (PCD8544[module].screen.width / PCD8544[module].font.width - strlen) / 2;
    
    // write spaces before
    while(nbspace--)
        PCD8544_printChar(module, 32);

    // write string
    PCD8544_print(module, string);
    PCD8544_print(module, (const u8*)"\n\r");
}
#endif

#if defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
void PCD8544_printNumber(u8 module, long value, u8 base)
{  
    PCD8544_SPI = module;
    printNumber(value, base);
}
#endif

#if defined(PCD8544PRINTFLOAT)
void PCD8544_printFloat(u8 module, float number, u8 digits)
{ 
    PCD8544_SPI = module;
    printFloat(number, digits);
}
#endif

#if defined(PCD8544PRINTF)
void PCD8544_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c != 0)
        PCD8544_printChar(module, *c++);
}
#endif

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

#ifdef PCD8544GRAPHICS

void setColor(u8 r, u8 g, u8 b)
{
    /*
    u16 c;
    r &= 0x1F;
    g &= 0x3F;
    b &= 0x1F;
    c = (r<<11) + (g<<5) + b;
    */
    //PCD8544_setColor(PCD8544_SPI, ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

//draw Pixel on the buffer
void PCD8544_drawPixel(u8 module, u8 x, u8 y)//, u8 color)
{
    if (x >= PCD8544[module].screen.width)  return;
    if (y >= PCD8544[module].screen.height) return;

    PCD8544_buffer[x + (y >> 3) * PCD8544[module].screen.width] |=  1 << (y % 8);
} 

// defined as extern void drawPixel(u16 x, u16 y); in graphics.c
void drawPixel(u16 x, u16 y)
{
    PCD8544_drawPixel(PCD8544_SPI, (u8)x, (u8)y);
}

void PCD8544_drawLine(u8 module, u8 x0, u8 y0, u8 x1, u8 y1)
{
    PCD8544_SPI = module;
    drawLine(x0, y0, x1, y1);
}

void drawVLine(u16 x, u16 y, u16 h)
{
    // To optimized
    drawLine(x, y, x, y+h-1);
}

void drawHLine(u16 x, u16 y, u16 w)
{
    // To optimized
    drawLine(x, y, x+w-1, y);
}

//clear Pixel on the buffer
void PCD8544_clearPixel(u8 module, u8 x, u8 y)
{
    if (x >= PCD8544[module].screen.width)  return;
    if (y >= PCD8544[module].screen.height) return;

    PCD8544_buffer[x + (y >> 3) * PCD8544[module].screen.width] &= ~(1 << (y % 8)); 
} 

u8 PCD8544_getPixel(u8 module, u8 x, u8 y)
{
    if (x >= PCD8544[module].screen.width)  return;
    if (y >= PCD8544[module].screen.height) return;

    return (PCD8544_buffer[x+ (y>>3) * PCD8544[module].screen.width] >> (y%8)) & 0x1;  
}

void PCD8544_drawCircle(u8 module, u8 x, u8 y, u8 radius)
{
    PCD8544_SPI = module;
    drawCircle(x, y, radius);
}

void PCD8544_fillCircle(u8 module, u8 x, u8 y, u8 radius)
{
    PCD8544_SPI = module;
    fillCircle(x, y, radius);
}

#endif

#ifdef _PCD8544_USE_SETCURSOR
void PCD8544_setCursor(u8 module, u8 x, u8 y)
{
     PCD8544[module].cursor.x = x;
     PCD8544[module].cursor.y = y;
}
#endif

#ifdef _PCD8544_USE_SETTEXTSIZE
void PCD8544_setTextSize(u8 module, u8 s)
{
    PCD8544[module].textsize = (s > 0) ? s : 1;
}
#endif

#ifdef _PCD8544_USE_SETTEXTCOLOR
void PCD8544_setTextColor(u8 module, u8 c)
{
    PCD8544[module].color.c = c;
    PCD8544[module].bcolor.c = c;
}
#endif

#ifdef _PCD8544_USE_SETTEXTCOLOR2
void PCD8544_setTextColor2(u8 module, u8 c, u8 bg)
{
    PCD8544[module].color.c   = c;
    PCD8544[module].bcolor.c = bg; 
}
#endif

#ifdef _PCD8544_USE_ORIENTATION
void PCD8544_setOrientation(u8 module, u8 x)
{
    PCD8544[module].orientation = (x & 3);
    switch(PCD8544[module].orientation) 
    {
        case 0:
        case 2:
            PCD8544[module].screen.width  = DISPLAY_WIDTH;
            PCD8544[module].screen.height = DISPLAY_HEIGHT;
            PCD8544[module].rows          = ((PCD8544[module].screen.height / 8) - 1)
            break;
        case 1:
        case 3:
            PCD8544[module].screen.width  = DISPLAY_HEIGHT;
            PCD8544[module].screen.height = DISPLAY_WIDTH;
            PCD8544[module].rows          = ((PCD8544[module].screen.height / 8) - 1)
            break;
    }
}
#endif

#ifdef  _PCD8544_USE_INVERT
void PCD8544_invertDisplay(u8 module, bool i)
{
  // Do nothing, must be subclassed if supported
}
#endif

#endif /* __PCD8544_C */
