/*  --------------------------------------------------------------------
    FILE:           ks0108.h
    PROJECT:        Pinguino
    PURPOSE:        Pinguino's Graphic LCD functions
    PROGRAMER:      Régis Blanchot
    FIRST RELEASE:  2016-10-17
    --------------------------------------------------------------------
    CHANGELOG:
    * 2016-10-17    Régis Blanchot - Added use of Print libraries
    * 2016-11-24    Régis Blanchot - Complete re-write
    * 2016-12-05    Régis Blanchot - moved font indices to const.h
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

#ifndef	KS0108_H
#define KS0108_H

#include <const.h>

//#define KS0108_DEBUG            // Serial Output
//#define KS0108_FAST             // Use of PORTx as Data Port

#ifdef KS0108_FAST
    #define KS0108_TRIS         TRISB
    #define KS0108_LAT          LATB
    #define KS0108_PORT         PORTB
#endif

#define KS0108_DISPLAY_WIDTH    128
#define KS0108_DISPLAY_HEIGHT   64
#define KS0108_TABSIZE          4

//Panel controller chips
#define KS0108_CHIP_WIDTH       64  // pixels per chip 

// Chips
#define KS0108_CHIP1            0x00
#define KS0108_CHIP2            0x01

// Commands
#define KS0108_ON               0x3F
#define KS0108_OFF              0x3E
#define KS0108_SET_ADD          0x40
#define KS0108_SET_Y            0x40
#define KS0108_SET_PAGE         0xB8
#define KS0108_SET_X            0xB8
#define KS0108_DISP_START       0xC0
#define KS0108_START_LINE       0xC0
#define KS0108_BUSY_FLAG        0x80

// Orientations
#define PORTRAIT                1
#define LANDSCAPE               0

// Colors
#define KS0108_BLACK            0x00
#define KS0108_WHITE            0xFF

// useful user contants
#define KS0108_NON_INVERTED     0
#define KS0108_INVERTED         1

// Font Indices : 05-12-2016 : RB - moved to const.h
/*
#define FONT_LENGTH             0
#define FONT_FIXED_WIDTH        2
#define FONT_WIDTH              2
#define FONT_HEIGHT             3
#define FONT_FIRST_CHAR         4
#define FONT_CHAR_COUNT         5
#define FONT_WIDTH_TABLE        6
#define FONT_OFFSET             6
*/

/*
#if (KS0108_DISPLAY_WIDTH / KS0108_CHIP_WIDTH  == 2) 
   u8 chipSelect[] = {1,2};        // this is for 128 pixel displays
#elif (KS0108_DISPLAY_WIDTH / KS0108_CHIP_WIDTH  == 3)
   //byte chipSelect[] = {0, 1, 2};  // this is for 192 pixel displays
   u8 chipSelect[] = {0, 2, 1};  // this is for 192 pixel displays on sanguino only
#endif
*/

/**	--------------------------------------------------------------------
    Typedef.
    ------------------------------------------------------------------*/

    typedef struct
    {
        u8 x;
        u8 y;
    } coord_t;

    typedef struct
    {
        u8 cs1;
        u8 cs2;
        u8 rs;
        u8 rw;
        u8 en;
        u8 d[8];
        u8 rst;
    } pin_t;

    typedef struct
    {
        const u8 *address;
        //u16 size;
        u8 width;
        u8 height;
        u8 firstChar;
        u8 charCount;
    } font_t;

    typedef struct
    {
        u8 startx;
        u8 starty;
        u8 endx;
        u8 endy;
        u8 width;
        u8 height;
        u8 invert;
        u8 bcolor;
        u8 color;
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
        pin_t pin;
        rect_t screen;
        coord_t pixel;
        font_t font;
    } lcd_t;

/**	--------------------------------------------------------------------
    Globals
    ------------------------------------------------------------------*/

lcd_t KS0108;

/**	--------------------------------------------------------------------
    Prototypes
    ------------------------------------------------------------------*/

void GLCD_enable();
void GLCD_send(u8);
u8   GLCD_get();
void GLCD_selectChip(u8);
void GLCD_writeCommand(u8);
void GLCD_writeData(u8);
u8   GLCD_readData(u8, u8);
void GLCD_setStartLine(u8);
void GLCD_goto(u8, u8);

void GLCD_init(u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8);
//void GLCD_home();
void GLCD_displayOn();
void GLCD_displayOff();
void GLCD_setCursor(u8, u8);
void GLCD_clearScreen();
void GLCD_invertDisplay();
void GLCD_normalDisplay();

// Print Functions
void GLCD_setFont(const u8*);
u8   GLCD_charWidth(u8);
u16  GLCD_stringWidth(u8*);
void GLCD_scrollUp();
void GLCD_printChar(u8 c);
void GLCD_print(const u8*);
void GLCD_println(const u8*);
void GLCD_printCenter(const u8 *);
void GLCD_printNumber(long, u8);
void GLCD_printFloat(float , u8);
void GLCD_printf(const u8 *, ...);

// Graphic Functions
void GLCD_drawPixel(u8, u8);
void GLCD_drawLine(u8, u8, u8, u8);
void GLCD_drawRect(u8, u8, u8, u8);
void GLCD_drawRoundRect(u8, u8, u8, u8);
void GLCD_fillRect(u8, u8, u8, u8);
void GLCD_drawCircle(u8, u8, u8);
void GLCD_fillCircle(u8, u8, u8);
void GLCD_drawBitmap(const u8 *, u8, u8);

/**	--------------------------------------------------------------------
    Macros
    ------------------------------------------------------------------*/

//#define ClearScreenX() FillRect(0, 0, (DISPLAY_WIDTH-1), (DISPLAY_HEIGHT-1), WHITE)
//#define ClearSysTextLine(_line) FillRect(0, (line*8), (DISPLAY_WIDTH-1), ((line*8)+ 7), WHITE )

#define GLCD_getFontWidth()   (KS0108.font.width)
#define GLCD_getFontHeight()  (KS0108.font.height)

#endif /* KS0108_H */
