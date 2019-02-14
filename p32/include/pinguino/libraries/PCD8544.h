/*  --------------------------------------------------------------------
    File          : PCD8544.h
    Project       : Pinguino
    Description   : Pinguino C library for Monochrome Nokia LCD
                    with PCD8544 controler
    Author        : Thomas Missonier (sourcezax@users.sourceforge.net)
                    Régis Blanchot (rblanchot@gmail.com)
    First release : March 2014
    --------------------------------------------------------------------
    CHANGELOG:
    04 Feb. 2016 - Régis Blanchot - added Pinguino SPI library support
    22 Oct. 2016 - Régis Blanchot - fixed graphics functions
    --------------------------------------------------------------------
    TODO:
    --------------------------------------------------------------------
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
    ------------------------------------------------------------------*/

#ifndef __PCD8544H
#define __PCD8544H

#include <typedef.h>            // Pinguino's type : u8, u8, ..., and bool
#include <macro.h>              // BitSet, BitClear
#include <spi.h>                // NUMOFSPI

//#include <logo/pinguino84x48.h> // Screen buffer pre-filled with Pinguino Logo

/** --------------------------------------------------------------------
    Display interfaces
    ------------------------------------------------------------------*/
/*
#define PCD8544_SPIHW           1<<0
#define PCD8544_SPISW           1<<1
#define PCD8544_PORTB           1<<2
#ifndef PCD8544_INTERFACE
    #define PCD8544_INTERFACE PCD8544_SPISW
#endif

#if (PCD8544_INTERFACE & PCD8544_PORTB)
    #define PCD8544_RST         0  // LCD RST 
    #define PCD8544_SCE         1  // LCD CS/CE  
    #define PCD8544_DC          2  // LCD Dat/Com
    #define PCD8544_SDIN        3  // LCD SPIDat/DIN/NOKIA_SDIN
    #define PCD8544_SCLK        4  // LCD SPIClk/CLK 
    #define PCD8544_VCC         5  // LCD NOKIA_VCC 3.3V 
    #define PCD8544_LIGHT       6  // LCD BACKNOKIA_LIGHT : GROUND or NOKIA_VCC 3.3V depends on models                                      
    #define PCD8544_GND         7  // LCD GROUND 
#endif
*/
/** --------------------------------------------------------------------
    Display sizes
    ------------------------------------------------------------------*/

#define PCD8544_DISPLAY_WIDTH  84
#define PCD8544_DISPLAY_HEIGHT 48
#define PCD8544_DISPLAY_ROWS   (PCD8544_DISPLAY_HEIGHT / 8)
#define PCD8544_DISPLAY_SIZE   (PCD8544_DISPLAY_WIDTH * PCD8544_DISPLAY_ROWS)
#define PCD8544_TABSIZE        4

/** --------------------------------------------------------------------
    Display commands
    ------------------------------------------------------------------*/

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
#define PCD8544_SETVOP  0x80

//Colors
#define PCD8544_BLACK 1
#define PCD8544_WHITE 0

//Orientations
#define PCD8544_PORTRAIT 1
#define PCD8544_LANDSCAPE 0

#ifdef _PCD8544_USE_ROUND_RECT
    #define _PCD8544_USE_CIRCLE
    #define _PCD8544_USE_RECT
#endif

/** --------------------------------------------------------------------
    Typedef.
    ------------------------------------------------------------------*/

/*
    typedef struct
    {
        u8 _din, _sclk, _dc, _rst, _cs;
        volatile u8 *mosiport, *clkport, *csport, *dcport;
        u8 mosipinmask, clkpinmask, cspinmask, dcpinmask;

        const u8   WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
        u8  _width, _height, // Display w/h as modified by current rotation
        cursor_x, cursor_y;
        u8 textcolor, textbgcolor;
        u8  textsize,  rotation;
        boolean  wrap;
      // the memory buffer for the LCD
    } PCD8544_t;
*/

    typedef struct
    {
        u16 x;
        u16 y;
        u16 page;
    } coord_t;

    typedef struct
    {
        u8 dc;
        u8 cs;
        u8 sdo;
        u8 sck;
        u8 rst;
    } pin_t;

    typedef struct
    {
        const u8 *address;
        u8 width;
        u8 height;
        u8 firstChar;
        u8 charCount;
    } font_t;

    typedef struct
    {
        u16 startx;
        u16 starty;
        u16 endx;
        u16 endy;
        u16 width;
        u16 height;
        u16 rows;
    } rect_t;

    typedef union
    {
        u16 w;
        struct
        {
            u8 l8;
            u8 h8;
        };
    } word_t;

    typedef struct
    {
        u8 orientation;
        pin_t pin;
        rect_t screen;
        coord_t pixel;
        font_t font;
    } lcd_t;

/** --------------------------------------------------------------------
    Globals
    ------------------------------------------------------------------*/

u8 PCD8544_SPI;
lcd_t PCD8544[NUMOFSPI];
//u8 * PCD8544_buffer = logo;  // screen buffer points on logo[]

/**	--------------------------------------------------------------------
    Prototypes
    ------------------------------------------------------------------*/

void PCD8544_init(int, ...);
void PCD8544_command(u8, u8);
void PCD8544_data(u8, u8);
void PCD8544_setContrast(u8, u8);
void PCD8544_clearScreen(u8);
void PCD8544_refresh(u8);
void PCD8544_drawPixel(u8, u8, u8);
void PCD8544_clearPixel(u8, u8, u8);
//u8 PCD8544_getPixel(u8, u8, u8);

#ifdef PCD8544GRAPHICS
//LINE
void PCD8544_drawLine(u8, u8, u8, u8, u8);
//TRIANGLE
void PCD8544_drawTriangle(u8, u8, u8, u8, u8, u8, u8);
void PCD8544_fillTriangle(u8, u8, u8, u8, u8, u8, u8);
//RECT
void PCD8544_drawRect(u8, u8, u8, u8, u8);
void PCD8544_fillRect(u8, u8, u8, u8, u8);
#define PCD8544_fillScreen(m) PCD8544_fillRect(m, 0, 0,PCD8544[m].screen.width,PCD8544[m].screen.height)
//ROUND_RECT
void PCD8544_drawRoundRect(u8, u8, u8, u8, u8);
void PCD8544_fillRoundRect(u8, u8, u8, u8, u8);
//CIRCLE
void PCD8544_drawCircle(u8, u8, u8, u8);
void PCD8544_fillCircle(u8, u8, u8, u8);
//BITMAP
void PCD8544_drawBitmap(u8, u8, u8, const u8 *,u8, u8);
//BASICS
void drawPixel(u16, u16);
extern void drawBitmap(u8, const u8 *, u16, u16);
#endif

void PCD8544_setCursor(u8, u8, u8);
void PCD8544_home(u8);

#ifdef _PCD8544_USE_ORIENTATION
void PCD8544_setOrientation(u8, u8);
#define PCD8544_getOrientation(m) (PCD8544[m].orientation)
#endif

#ifdef  _PCD8544_USE_INVERT
void PCD8544_invertDisplay(u8);
void PCD8544_normalDisplay(u8);
#endif

//Print functions
//#ifdef _PCD8544_USE_TEXT
void PCD8544_printChar(u8, u8 c);
void PCD8544_print(u8, const u8 *);
void PCD8544_println(u8, const u8 *);
void PCD8544_printf(u8, const u8 *, ...);
void PCD8544_printNumber(u8, long, u8);
void PCD8544_printFloat(u8, float, u8);
//#endif

/*
#ifdef __SDCC
#define PCD8544_drawPixel(m, x, y)  *(PCD8544_buffer[(y) >> 3] + (x)) |= (1 << ((y) % 8))
#else
#define PCD8544_drawPixel(m, x, y)  PCD8544_buffer[(y) >> 3][x] |= (1 << ((y) % 8))
#endif

#ifdef __SDCC
#define PCD8544_clearPixel(m, x, y) *(PCD8544_buffer[y >> 3] + (x)) &= ~(1 << (y % 8))
#else
#define PCD8544_clearPixel(m, x, y) PCD8544_buffer[y >> 3][x] &= ~(1 << (y % 8))
#endif
*/
#define PCD8544_getFontWidth(m)  (PCD8544[m].font.width)
#define PCD8544_getFontHeight(m) (PCD8544[m].font.height)
#define PCD8544_select(m)         SPI_select(m)
#define PCD8544_deselect(m)       SPI_deselect(m)

#endif // __PCD8544H
